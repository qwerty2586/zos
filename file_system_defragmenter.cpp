#include <algorithm>
#include <unistd.h>
#include "file_system_defragmenter.h"

int FileSystemDefragmenter::mount(const std::string &fat_file) {

    int r = FileSystem::mount(fat_file);
    if (r != EXIT_OK) return r;

    create_back_table();
    populate_jobs_and_back_table(0, -1);
    return EXIT_OK;

}

void FileSystemDefragmenter::create_back_table() {
    back_table = new int32_t[br->usable_cluster_count];
    reservation_table = new bool[br->usable_cluster_count];
    for (int i = 0; i < br->usable_cluster_count; ++i) {
        back_table[i] = -1;
        reservation_table[i] = false;
    }
}

int FileSystemDefragmenter::defragment(int threads_count) {
    std::unique_lock<std::mutex> lock(main_thread_mutex);
    phase = 0;
    all_jobs_finished = false;
    for (int i = 0; i < threads_count; ++i) {
        workers.push_back(std::thread([&]() {
            this->worker_loop();
        }));
    }

    // pockame nez workeri najedou
    usleep(10000);

    auto start_time = std::chrono::high_resolution_clock::now();
    std::cout << "defragmenting using " << threads_count << " threads" << std::endl;
    long last_score = INT64_MAX;

    while (1) {

        analyze_and_sort_jobs();
        print_stats();
        if (total_score >= last_score || prepared.get_vector().size() == 0) {
            auto end_time = std::chrono::high_resolution_clock::now();
            std::cout << "defragmenting using " << threads_count << " threads finished" << std::endl;
            std::cout << "time of operation " <<
                      std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                      << " ms"<< std::endl;
            break;
        }
        last_score = total_score;


        phase++;


        paused_threads = 0;
        workers_cv.notify_all(); // start_threads

        while (paused_threads < threads_count) {
            main_thread_cv.wait(lock);
        }

        fat_mutex.lock();
        flush_fat();
        fat_mutex.unlock();
    }


    //zabijeme vlakna
    all_jobs_finished = true;
    workers_cv.notify_all();
    for (int i = 0; i < threads_count; ++i) {
        workers[i].join();
    }


    return 0;
}


void FileSystemDefragmenter::worker_loop() {
    std::unique_lock<std::mutex> lock(workers_mutex);
    workers_cv.wait(lock); // ceka na prvni odstartovani

    while (1) {

        Job *job = prepared.pop();
        if (job == NULL) { //dokonceni prace ve fazi nejsou joby
            paused_threads++;
            main_thread_cv.notify_all();
            workers_cv.wait(lock);
            if (all_jobs_finished) break; // posledni bariera
            else continue;
        }


        int32_t old_first_cluster = job->clusters[0];
        int32_t size = (int32_t) job->clusters.size();
        int32_t first_cluster = reserve_chunk(size);
        if (first_cluster < 0) {
            // nemame tak velky blok a tak zkusime se souborem pockat do dalsiho kola
            to_next_round.push(job);
            continue;
        }

        //presuneme soubor
        for (int i = 0; i < size; ++i) {
            move_cluster(job->clusters[i], first_cluster + i);
        }
        job->score = 0; // pozadame o pouze castecnou analyzu - soubor je defragmentovan
        fat_mutex.lock();
        for (int i = 0; i < size; ++i) {
            fat[job->clusters[i]] = FAT_UNUSED;
            fat[first_cluster + i] = first_cluster + i + 1;
            back_table[first_cluster + i] = first_cluster + i - 1;
        }
        fat[first_cluster + size - 1] = FAT_FILE_END;
        back_table[first_cluster] = back_table[job->clusters[0]];
        fat_mutex.unlock();
        rw_mutex.lock();
        std::vector<directory> *dir = get_dir(back_table[first_cluster]);
        for (auto item : *dir) {
            if (item.start_cluster == old_first_cluster) {
                item.start_cluster = first_cluster;
                break;
            }
        }
        write_dir(back_table[first_cluster], dir);
        delete dir;
        rw_mutex.unlock();
        for (int i = 0; i < size; ++i) {
            job->clusters[i] = first_cluster + i;
        }
        dereserve_chunk(first_cluster, size);

        to_next_round.push(job);

    }


}

void FileSystemDefragmenter::populate_jobs_and_back_table(int32_t cluster, int32_t return_cluster) {

    back_table[cluster] = return_cluster;

    switch (fat[cluster]) {
        case FAT_UNUSED:
        case FAT_BAD_CLUSTER: {
            std::cout << "error processing cluster " << cluster << " skipping item" << std::endl;
            return;
        }
        case FAT_DIRECTORY: {
            auto dir = get_dir(cluster);
            for (auto item : *dir) {
                populate_jobs_and_back_table(item.start_cluster, cluster);
            }
            //Job *job = new Job;
            //job->is_dir = true;
            //job->score = -1;
            //job->clusters.push_back(cluster);
            //to_next_round.push(job);
            // budeme pridavat jenom soubory zatim
            return;
        }

        case FAT_FILE_END: {

            Job *job = new Job;
            job->is_dir = false;
            job->score = -1;
            int32_t back = cluster;
            while (fat[back] != FAT_DIRECTORY) {
                job->clusters.insert(job->clusters.begin(), back); // push_front()
                back = back_table[back];
            }
            to_next_round.push(job);
            return;
        }
        default:;;
            populate_jobs_and_back_table(fat[cluster], cluster);
    }


}

int32_t FileSystemDefragmenter::reserve_chunk(int32_t size) {
    int32_t r = last_reserver; // default 0 meli bychm resetovat mezi fazemi
    fat_mutex.lock();
    reservation_mutex.lock();
    for (int64_t i = last_reserver; i < last_reserver + br->usable_cluster_count - 1; ++i) {
        if (fat[i % br->usable_cluster_count] == FAT_UNUSED &&
            reservation_table[i % br->usable_cluster_count] == false) {
            if (r < 0) {
                r = (int32_t) (i % br->usable_cluster_count);
            } else {
                if ((i % br->usable_cluster_count) - r >= size) {
                    for (int32_t j = r; j < r + size; ++j) {
                        reservation_table[j] = true;
                    }
                    last_reserver = r + size;
                    reservation_mutex.unlock();
                    fat_mutex.unlock();
                    return r;
                }
            }
        } else {
            r = -1; // tohle nam vyresetuje pocitani
        }
    }
    reservation_mutex.unlock();
    fat_mutex.unlock();
    return -1; //pokud nenajdem chunk pak se musi pockat se souborem do dalsi faze
}

void FileSystemDefragmenter::dereserve_chunk(int32_t addr, int32_t size) {
    reservation_mutex.lock();
    for (int32_t i = 0; i < size; ++i) {
        reservation_table[i + addr] = false;
    }
    reservation_mutex.unlock();
}

void FileSystemDefragmenter::analyze_and_sort_jobs() {
    fat_mutex.lock();
    auto joblist = to_next_round.get_vector();
    total_score = 0;
    total_fragments = 0;
    for (int i = 0; i < joblist.size(); ++i) {
        if (joblist[i]->score < 0) { // -1 znamena ze mame analyzovat dukladne zda mame clustery fragmentovany
            joblist[i]->score = 0;
            for (int j = 0; j < joblist[i]->clusters.size() - 1; ++j) {
                if (joblist[i]->clusters[j] + 1 != joblist[i]->clusters[j + 1]) {
                    joblist[i]->score++;
                    total_fragments++;
                }
            }
        } else {
            joblist[i]->score = 0; // stejne musime vynulovat
        }
        if (fat[joblist[i]->clusters.front() - 1] == FAT_UNUSED) joblist[i]->score++;
        if (joblist[i]->clusters.back() <
            br->usable_cluster_count - 1) { //test zda to neni nahodou posledni cluster v fatce
            if (fat[joblist[i]->clusters.back() + 1] == FAT_UNUSED) joblist[i]->score++;
        }
        total_score += joblist[i]->score;
    }
    Job *job = to_next_round.pop();
    while (job != NULL) {
        if (job->score == 0) {
            finished.push(job);
        } else {
            prepared.push(job);
        }
        job = to_next_round.pop();
    }
    total_files = prepared.get_vector().size();
    fat_mutex.unlock();
}

void FileSystemDefragmenter::print_stats() {
    std::cout << "============================" << std::endl;
    std::cout << " PHASE     : " << phase << std::endl;
    std::cout << " JOBS      : " << total_files << std::endl;
    std::cout << " FRAGMENTS : " << total_fragments << std::endl;
    std::cout << " SCORE     : " << total_score << std::endl;
}

void FileSystemDefragmenter::move_cluster(int32_t from, int32_t to) {
    rw_mutex.lock();
    fseek(fs, cluster_to_addr(from), SEEK_SET);
    fread(buffer_cluster, br->cluster_size, 1, fs);
    fseek(fs, cluster_to_addr(to), SEEK_SET);
    fwrite(buffer_cluster, br->cluster_size, 1, fs);
    rw_mutex.unlock();
};





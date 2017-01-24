#ifndef ZOS_FILE_SYSTEM_DEFRAGMENTER_H
#define ZOS_FILE_SYSTEM_DEFRAGMENTER_H

#include <thread>
#include <condition_variable>
#include "file_system.h"
#include "job_list.h"

class FileSystemDefragmenter : public FileSystem {
public:
    int mount(const std::string &fat_file) override;

    int umount() override;

    int defragment(int threads_count);

    void print_blocks();

protected:

    FILE *fw, *fr; // pouze na kopirovani clusteru

    ///seznamy praci pro pracovni vlakna
    JobList prepared, to_next_round, finished;
    ///tabulka zpetnych clusteru
    int32_t *back_table;


    ///kondicni promene
    std::mutex workers_mutex, main_thread_mutex;
    std::condition_variable workers_cv, main_thread_cv;

    ///mutexy
    std::mutex reservation_mutex, fat_mutex, fs_mutex, fw_mutex, fr_mutex;

    ///pro oznameni vlaknum ze jsme hotovy
    bool all_jobs_finished = false;

    ///pracovni vlakna
    std::vector<std::thread> workers;

    ///blokujici tabulka
    bool *reservation_table;

    int phase = 0;

    /// pro oznameni hlavnimu vlaknu
    int paused_threads = 0;

    //statistiky
    int total_score;
    int total_fragments;
    unsigned long total_files;


    void print_stats();

    void analyze_and_sort_jobs();

    void dereserve_chunk(int32_t addr, int32_t size);

    void create_back_table();

    int32_t reserve_chunk(int32_t size);

    void worker_loop();

    void populate_jobs_and_back_table(int32_t cluster, int32_t return_cluster);

    void move_cluster(char *buffer_cluster, int32_t from, int32_t to);
};


#endif //ZOS_FILE_SYSTEM_DEFRAGMENTER_H

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

    FILE *fw,*fr; // pouze na kopirovani clusteru

    JobList prepared,to_next_round,finished;
    int32_t *back_table;
    int32_t last_reserver = 0;

    std::mutex workers_mutex,main_thread_mutex;
    std::condition_variable workers_cv,main_thread_cv;
    std::mutex reservation_mutex, fat_mutex, fs_mutex, fw_mutex, fr_mutex;

    bool all_jobs_finished = false;
    std::vector<std::thread> workers;
    bool *reservation_table;
    void create_back_table();
    int32_t reserve_chunk(int32_t size);
    void worker_loop();
    void populate_jobs_and_back_table(int32_t cluster, int32_t return_cluster);

    int phase = 0;

    void dereserve_chunk(int32_t addr, int32_t size);

    int paused_threads = 0;

    void analyze_and_sort_jobs();

    int total_score;
    int total_fragments;
    unsigned long total_files;

    void print_stats();

    void move_cluster(char *buffer_cluster, int32_t from, int32_t to);
};


#endif //ZOS_FILE_SYSTEM_DEFRAGMENTER_H

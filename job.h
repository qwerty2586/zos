#ifndef ZOS_JOB_H
#define ZOS_JOB_H

#include <cstdint>
#include <vector>

class Job {
public:
    int32_t start_cluster;
    bool is_dir;
    std::vector<int32_t> clusters;
    int32_t score;
private:
};

#endif //ZOS_JOB_H

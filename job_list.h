#ifndef ZOS_JOB_LIST_H
#define ZOS_JOB_LIST_H

#include <vector>
#include <mutex>
#include "job.h"

class JobList {
public:
    Job *pop();
    void push(Job *job);
    std::vector<Job *> get_vector() { return jobs; };
private:
    std::vector<Job *> jobs;
    std::mutex mutex;
};

#endif //ZOS_JOB_LIST_H

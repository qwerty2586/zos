#include "job_list.h"

Job *JobList::pop() {
    Job *r;
    mutex.lock();
    if (jobs.empty()) {
        mutex.unlock();
        return NULL;
    };
    r = jobs.front();
    jobs.erase(jobs.begin());
    mutex.unlock();
    return r;
}

void JobList::push(Job *job) {

    mutex.lock();
    jobs.push_back(job);
    mutex.unlock();
}

#ifndef _WORKER_MANAGER_H_
#define _WORKER_MANAGER_H_

#include <thread>
#include <vector>
#include <limits.h>

#include "ServiceThread.h"
#include "ServiceWorker.h"

using WorkersT = std::vector<std::shared_ptr<ServiceThread>>;

class WorkerManager{
    private:
        int                 m_threadCount;
        uint32_t            m_index;
        int                 m_distType;
        WorkersT            m_workers;
        void loadbalanceFd(int fd);
        uint32_t getWorkerIndex();
    public:
        enum LoadDistributionType{
            ROUND_ROBIN,
            DIST_LESS_LOADED_WORKER
        };
        WorkerManager(int threadCount, LoadDistributionType distType);
        WorkerManager(const WorkerManager&) = delete;
        WorkerManager& operator=(const WorkerManager&) = delete;
        ~WorkerManager();

        void initWorkers();
        void dispatchFd(int);
        void stopWorkers();

};
#endif

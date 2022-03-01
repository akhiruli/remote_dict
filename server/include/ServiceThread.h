#ifndef _SERVICE_THREAD_H_
#define _SERVICE_THREAD_H_

#include "EventImpl.h"
#include "Helper.h"

#include <deque>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <pthread.h>

struct ThreadContextT {
    int                   m_sockets[2];
    EventImpl             *m_libEvent;
    struct event          m_socketPairEvent;
    std::deque<int>       m_queue;
    std::mutex            m_threadCtxMtx;
    ThreadContextT() : m_libEvent(nullptr){
        if(m_libEvent)
            delete m_libEvent;
    }
};

class ServiceThread{
    public:
        std::atomic<bool>              m_exited;
        std::atomic<bool>              m_processExiting;
        static std::mutex              m_threadStopMtx;
        static std::condition_variable m_threadStopCondVar;
        ServiceThread();
        virtual ~ServiceThread();
        static void onEvent(int fd, short ev, void *arg);
        void start(bool spawnThread=true);
        void run();
        bool stop();
        void stopThread();
        bool addFdToEvLoop(int fd);
        uint32_t getCurrentFdQueueSize();
        long getThreadId(){
            return m_threadId;
        }
        std::shared_ptr<ThreadContextT> getThreadCtx(){
            return m_threadCtx;
        }

    protected:
        virtual void cleanState(){}
        virtual void processEvent(int fd) {}
        virtual void initApp(){}
        void addEvent(struct event *event, int fd, event_callback_fn cbk, void *arg);//It should be called from same the thread
        void deleteEvent(struct event *l_event);
        std::shared_ptr<ThreadContextT> m_threadCtx;
    private:
        long     m_threadId;
        bool     m_spawnThread;
};
#endif

#ifndef _STATS_H_
#define _STATS_H_

#include <memory>
#include <atomic>
#include <mutex>

class Stats{
    private:
        Stats();
        Stats(const Stats&) = delete;
        Stats& operator=(const Stats&) = delete;
        static std::unique_ptr<Stats> m_instance;
        static std::once_flag         m_initInstanceFlag;
        static std::mutex             m_mtx;
        uint64_t     m_numGetReq;
        uint64_t     m_numGetReqSucc;
        uint64_t     m_numGetReqFail;
    public:
        static Stats* getInstance();
        void incrGetReq();
        void incrGetReqSucc();
        void incrGetReqFail();

        uint64_t getNumGetReq(){
            return m_numGetReq;
        }

        uint64_t getNumGetReqSucc(){
            return m_numGetReqSucc;
        }

        uint64_t getNumGetReqFail(){
            return m_numGetReqFail;
        }
};
#endif

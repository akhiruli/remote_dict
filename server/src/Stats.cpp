#include "Stats.h"
std::unique_ptr<Stats> Stats::m_instance;
std::once_flag Stats::m_initInstanceFlag;
std::mutex Stats::m_mtx;

Stats::Stats() : m_numGetReq(0), m_numGetReqSucc(0), m_numGetReqFail(0){
}

Stats* Stats::getInstance(){
    std::call_once(m_initInstanceFlag, [&](){ m_instance.reset(new Stats());});
    return m_instance.get();
}

void Stats::incrGetReq(){
    std::lock_guard<std::mutex> lock(m_mtx);
    ++m_numGetReq;
}

void Stats::incrGetReqSucc(){
    std::lock_guard<std::mutex> lock(m_mtx);
    ++m_numGetReqSucc;
}

void Stats::incrGetReqFail(){
    std::lock_guard<std::mutex> lock(m_mtx);
    ++m_numGetReqFail;
}

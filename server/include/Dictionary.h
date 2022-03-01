#ifndef _DICT_H_
#define _DICT_H_

#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>

#include "bloom_filter.hpp"

class Dictionary{
    private:
        Dictionary();
        Dictionary(const Dictionary&) = delete;
        Dictionary& operator=(const Dictionary&) = delete;
        static std::unique_ptr<Dictionary> m_instance;
        static std::once_flag              m_initInstanceFlag;
        static std::mutex                  m_mtx;
        bool                               m_bloomFilterEnabled;
        std::unordered_map<std::string, std::string> m_dict;
        std::unique_ptr<bloom_filter>      m_bloomFilter;
        void initBloomFilters();
    public:
        static Dictionary* getInstance(); 
        bool set(std::string key, std::string val);
        bool get(std::string key, std::string& val);
        void enableBloomFilter(bool enable);

};

#endif

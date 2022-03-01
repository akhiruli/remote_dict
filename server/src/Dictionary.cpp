#include "Dictionary.h"

#define MAX_DICT_SIZE 1000000

std::unique_ptr<Dictionary> Dictionary::m_instance;
std::once_flag              Dictionary::m_initInstanceFlag;
std::mutex                  Dictionary::m_mtx;

Dictionary::Dictionary() : m_bloomFilterEnabled(false){

}

/*
 *@brief creates a single instance of Dictionary class
 *@return returns the singleton class of Dictionary
 * */
Dictionary* Dictionary::getInstance(){
    std::call_once(m_initInstanceFlag, [&](){ m_instance.reset(new Dictionary());});
    return m_instance.get();
}

/*
 *@brief initiate the bloom filter class if it is enabled
 *@param the bloom filter enab,le flag
 * */
void Dictionary::enableBloomFilter(bool enable){
    this->m_bloomFilterEnabled = enable;
    if(this->m_bloomFilterEnabled){
        initBloomFilters();
    }
}

/*
 *@brief initiate the bloom filter class
 * */
void Dictionary::initBloomFilters(){
    printf("Initializing bloom filter library\n");
    bloom_parameters parameters;
    parameters.projected_element_count = MAX_DICT_SIZE;
    parameters.false_positive_probability = 0.0001; // 1 in 10000
    parameters.compute_optimal_parameters();

    m_bloomFilter.reset(new bloom_filter(parameters));
}

/*
 *@brief handls SET request
 *@param key
 *@param value
 *@return true for success, false for failure
 * */
bool Dictionary::set(std::string key, std::string val){
    if(m_dict.size() >= MAX_DICT_SIZE){
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mtx);
    if(m_bloomFilterEnabled)
        m_bloomFilter->insert(key);

    m_dict[key] = val;

    return true;
}

/*
 *@brief handls GET request
 *@param key
 *@param value a out param
 *@return true for success, false for failure
 * */
bool Dictionary::get(std::string key, std::string& val){
    std::lock_guard<std::mutex> lock(m_mtx);
    bool ret = true;
    if(m_bloomFilterEnabled){
        if(m_bloomFilter->contains(key)){
            if(m_dict.find(key) != m_dict.end()){
                val = m_dict[key];
            }
            ret = false;//false positive
        } else{
            val = "NOT_FOUND";
            ret = false;
        }
    }else{
        if(m_dict.find(key) != m_dict.end()){
            val = m_dict[key];
        } else{
            val = "NOT_FOUND";
            ret = false;
        }
    }

    return ret;
}

#ifndef _TEST_CLIENT_H_
#define _TEST_CLIENT_H_

#include <Client.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <thread>
#include <memory>
#include <cctype>
#include <fstream>
#include <time.h>

#include "Helper.h"

#define MAX_LINE_WRITE 1024

using ThreadPool = std::vector<std::pair<std::shared_ptr<std::thread>, std::shared_ptr<Client>>>;

class TestClient{
    public:
        TestClient(std::string input_file, int num_thread, std::string host, int port);
        TestClient(const TestClient&) = delete;
        TestClient& operator = (const TestClient&) = delete;
        ~TestClient();
        void start();
        static bool compareByTimestamp(const ResultData &a, const ResultData &b);
    private:
        std::string m_inputFile;
        int         m_numThread;
        std::string m_host;
        int         m_port;
        void readAndParseInputFile(std::vector<MessageT>& input_data);
        bool caseInSensStringCompare(std::string str1, std::string str2);
        std::string getReqType(Payload_RequestType type);
        void saveResults(const ThreadPool pool);

};
#endif

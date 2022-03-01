#ifndef _HELPER_H_
#define _HELPER_H_

#include <unistd.h>
#include <fcntl.h>
#include <iostream>

namespace Helper{
    const std::string WHITESPACE = " \n\r\t\f\v";
    /*
     *@brief left trimming of string
     *@param string to be trimmed
     * */
    static std::string ltrim(const std::string& s){
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }

    /*
     *@brief right trimming of string
     *@param string to be trimmed
     * */
    static std::string rtrim(const std::string& s){
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }

    /*
     *@brief trimming of string
     *@param string to be trimmed
     * */
    static std::string trim(const std::string& s){
        return rtrim(ltrim(s));
    }

    /*
     *@brief setting a FD to non blocking
     *@param fd, file descriptor
     *@return 0, for success and -1 for failure
     * */
    static int setnonblock(int fd){
        int flags;

        flags = fcntl(fd, F_GETFL);
        if (flags < 0)
            return flags;
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0)
            return -1;

        return 0;
    }
}

#endif

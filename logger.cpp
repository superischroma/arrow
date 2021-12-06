#include <iostream>

#include "logger.h"

namespace arrow
{
    void log(std::string str, std::string&& level)
    {
        std::cout << "[arrow | " << level << "] " << str << std::endl;
    }

    void info(std::string str)
    {
        log(str, "info");
    }

    void warn(std::string str)
    {
        log(str, "warning");
    }

    void err(std::string str)
    {
        log(str, "error");
    }

    void err(std::string str, int line)
    {
        log(str, "error at line " + std::to_string(line));
    }
}
#ifndef ARROW_LOGGER_H
#define ARROW_LOGGER_H

#include <string>

namespace arrow
{
    void info(std::string str);
    void warn(std::string str);
    void err(std::string str);
    void err(std::string str, int line);
}

#endif
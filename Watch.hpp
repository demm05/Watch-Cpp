#pragma once

#include <cstdint>
#include <map>
#include <string>

class Watch {
public:
    Watch();
    ~Watch();

    bool add(char const *path, uint32_t mask);
    void operator()();

private:
    int inotifyFd_;
    std::map<int, std::string> watchedPaths_;
};

#include "Watch.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/inotify.h>
#include <unistd.h>

// Define buffer size for reading events
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

Watch::Watch() : inotifyFd_(inotify_init()) {
    if (inotifyFd_ < 0) {
        throw std::runtime_error("inotify_init failed");
    }
}

Watch::~Watch() {
    // Use .first to get the key (the watch descriptor) from the map iterator
    for (auto const &[wd, path] : watchedPaths_) {
        inotify_rm_watch(inotifyFd_, wd);
    }
    close(inotifyFd_);
}

bool Watch::add(char const *path, uint32_t mask) {
    // The 'mask' parameter from the function is now correctly used
    int wd = inotify_add_watch(inotifyFd_, path, mask);
    if (wd < 0) {
        perror("inotify_add_watch");
        return false;
    }
    std::cout << "Registered watch for: " << path << std::endl;
    // Store the watch descriptor and path in the map
    watchedPaths_[wd] = path;
    return true;
}

void Watch::operator()() {
    char buffer[BUF_LEN];

    while (true) {
        int length = read(inotifyFd_, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        int i = 0;
        while (i < length) {
            // Point to the current event in the buffer
            struct inotify_event *event = (struct inotify_event *)&buffer[i];

            // The main logic fix: We no longer check if event->len > 0.
            // We process every event we receive.
            std::string event_name;
            if (event->mask & IN_CREATE)
                event_name = "CREATED";
            else if (event->mask & IN_MODIFY)
                event_name = "MODIFIED";
            else if (event->mask & IN_DELETE)
                event_name = "DELETED";
            else if (event->mask & IN_CLOSE_WRITE)
                event_name = "CLOSED_AFTER_WRITE";
            else
                event_name = "OTHER_EVENT";

            // Use the watch descriptor to find the path
            std::string path = watchedPaths_[event->wd];

            // If a filename is provided, it's an event in a directory
            if (event->len > 0) {
                path += "/" + std::string(event->name);
            }

            std::cout << "Event: " << event_name << " on path: " << path << std::endl;

            // YOUR CUSTOM CODE HERE:
            // e.g., reload_configuration(path);

            // Move to the next event in the buffer
            i += EVENT_SIZE + event->len;
        }
    }
}

#include "Watch.hpp"
#include <sys/inotify.h>

int main(int argc, char **argv) {
    Watch w;
    for (int i = 1; i < argc; i++) {
        w.add(argv[i], IN_MODIFY | IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);
    }
    w();
}

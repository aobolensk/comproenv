#include <iostream>
#include "shell.h"

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    Shell shell(argc < 2 ? "config/windows_sample.yaml" : argv[1]);
    #else
    Shell shell(argc < 2 ? "config/linux_sample.yaml" : argv[1]);
    #endif  // _WIN32
    shell.run();
    return 0;
}
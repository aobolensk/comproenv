#include <iostream>
#include "shell.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Too few arguments";
        return 1;
    }
    Shell shell(argv[1]);
    shell.run();
    return 0;
}
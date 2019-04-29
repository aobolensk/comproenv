#include <iostream>
#include "shell.h"

int main(int argc, char *argv[]) {
    Shell shell(argc < 2 ? "config/config.yaml" : argv[1]);
    shell.run();
    return 0;
}
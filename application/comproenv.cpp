#include "shell.h"

int main(int argc, char *argv[]) {
    comproenv::Shell shell(argc < 2 ? "" : argv[1], argc < 3 ? "" : argv[2]);
    shell.run();
    return 0;
}

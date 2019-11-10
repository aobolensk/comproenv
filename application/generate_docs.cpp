#include <iostream>
#include <fstream>
#include "fs.h"
#include "shell.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: generate_docs <docs directory>" << std::endl;
        return 1;
    }
    if (!fs::exists(argv[1])) {
        std::cerr << "Path " << argv[1] << " does not exist" << std::endl;
    }
    fs::path commands_path = fs::path(argv[1]) / "Commands.md";
    std::ofstream commands(commands_path);
    if (!commands.is_open()) {
        std::cerr << "File " << commands_path << " can not be opened" << std::endl;
    }
    commands << "#### Warning! This document is generated automatically by using 'generate_docs' executable\n\n";
    commands << "# Commands list";
    comproenv::Shell shell;
    for (int i = 0; i < comproenv::Shell::State::INVALID; ++i) {
        commands << "\n\n"
                    "### Scope: " << shell.state_names[i] << "\n\n"
                    "| Command | Description |\n"
                    "|---------|-------------|\n";
        commands << shell.get_help(comproenv::Shell::State(i));
    }
    commands.close();
    return 0;
}

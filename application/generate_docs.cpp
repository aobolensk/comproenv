#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include "shell.h"

namespace fs = std::experimental::filesystem;

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
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
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

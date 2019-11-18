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

    comproenv::Shell shell;

    fs::path commands_path = fs::path(argv[1]) / "Commands.md";
    std::ofstream commands(commands_path);
    if (!commands.is_open()) {
        std::cerr << "File " << commands_path << " can not be opened" << std::endl;
    }
    commands << "#### Warning! This document is generated automatically by using 'generate_docs' executable\n\n";
    commands << "# Commands list";
    for (int i = 0; i < comproenv::Shell::State::INVALID; ++i) {
        commands << "\n\n"
                    "### Scope: " << shell.state_names[i] << "\n\n"
                    "| Command | Description |\n"
                    "|---------|-------------|\n";
        commands << shell.get_help(comproenv::Shell::State(i));
    }
    commands.close();

    fs::path examples_path = fs::path(argv[1]) / "Examples.md";
    std::ofstream examples(examples_path);
    if (!examples.is_open()) {
        std::cerr << "File " << examples_path << " can not be opened" << std::endl;
    }
    examples << "#### Warning! This document is generated automatically by using 'generate_docs' executable\n\n";
    examples << "# Examples of commands usage";
    for (int i = 0; i < comproenv::Shell::State::INVALID; ++i) {
        examples << "\n\n"
                    "### Scope: " << shell.state_names[i] << "\n\n";
        auto command_examples = shell.get_examples(comproenv::Shell::State(i));
        for (const auto &example : command_examples) {
            examples << "#### " << example.first << "\n```\n" << example.second << "```\n";
        }
    }
    examples.close();
    return 0;
}

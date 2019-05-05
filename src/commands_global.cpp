#include <fstream>
#include <experimental/filesystem>
#include "shell.h"

namespace fs = std::experimental::filesystem;

void Shell::configure_commands_global() {
    // Set environment
    add_command(State::GLOBAL, "se", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i) {
            if (envs[i].get_name() == arg[1]) {
                current_env = i;
                current_state = State::ENVIRONMENT;
                return 0;
            }
        }
        throw std::runtime_error("Incorrect environment name");
    });

    // Create environment
    add_command(State::GLOBAL, "ce", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i)
            if (envs[i].get_name() == arg[1])
                throw std::runtime_error("Environment named " + arg[1] + " already exists");
        envs.push_back(arg[1]);
        fs::path path = fs::current_path() / ("env_" + arg[1]);
        if (!fs::exists(path)) {
            fs::create_directory(path);
        }
        return 0;
    });

    // Delete environment
    add_command(State::GLOBAL, "de", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i) {
            if (envs[i].get_name() == arg[1]) {
                envs.erase(envs.begin() + i);
                fs::path path = fs::current_path() / ("env_" + arg[1]);
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        throw std::runtime_error("Incorrect environment name");
    });

    // Save settings
    add_command(State::GLOBAL, "s", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        int indent = 0;
        std::ofstream f(config_file, std::ios::out);
        auto serialize_settings = [&](std::unordered_map <std::string, std::string> &settings) {
            std::vector <std::pair <std::string, std::string>> compilers;
            for (auto &setting : settings) {
                if (setting.first.compare(0, std::size("compiler_") - 1, "compiler_") == 0) {
                    compilers.emplace_back(setting.first.substr(std::size("compiler_") - 1), setting.second);
                } else {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    f << setting.first << ": " << setting.second << std::endl;
                }
            }
            if (compilers.size()) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << "compilers:" << std::endl;
                indent += 2;
                for (auto &compiler : compilers) {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    f << compiler.first << ": " << compiler.second << std::endl;
                }
                indent -= 2;
            }
        };
        f << "environments:" << std::endl;
        indent += 2;
        for (auto &env : envs) {
            for (int i = 0; i < indent; ++i)
                f << " ";
            f << "- name: " << env.get_name() << std::endl;
            indent += 2;
            serialize_settings(env.get_settings());
            if (env.get_tasks().size()) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << "tasks:" << std::endl;
                indent += 2;
                for (auto &task : env.get_tasks()) {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    f << "- name: " << task.get_name() << std::endl;
                    indent += 2;
                    serialize_settings(task.get_settings());
                    indent -= 2;
                }
                indent -= 2;
            }
            indent -= 2;
        }
        indent -= 2;
        f << std::endl;
        f << "global:" << std::endl;
        indent += 2;
        serialize_settings(global_settings);
        f.close();
        return 0;
    });

    // Exit from program
    add_command(State::GLOBAL, "q", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::cout << "Exiting..." << std::endl;
        exit(0);
        return 0;
    });
    add_alias(State::GLOBAL, "q", State::GLOBAL, "exit");
}

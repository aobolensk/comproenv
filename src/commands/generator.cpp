#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <experimental/filesystem>
#ifdef _WIN32
#include <direct.h>
#define chdir _chdir
#else
#include <unistd.h>
#endif  // _WIN32
#include "shell.h"
#include "utils.h"

namespace comproenv {

namespace fs = std::experimental::filesystem;

void Shell::configure_commands_generator() {
    add_command(State::GENERATOR, "cg", "Compile generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string current_compiler = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        std::string command = get_setting_by_name("compiler_" + current_compiler);
        replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                            "tests" / "generator").string());
        replace_all(command, "@lang@", current_compiler);
        std::cout << "\033[35m" << "-- Compile generator for " <<
            envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
        DEBUG_LOG(command);
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\033[35m" << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m\n";
        return ret_code;
    });

    add_command(State::GENERATOR, "rg", "Run generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string current_runner = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        std::string command;
        #ifdef _WIN32
        std::string directory = "\"" + env_prefix + envs[current_env].get_name() + "\\" +
                task_prefix + envs[current_env].get_tasks()[current_task].get_name() + "\\"
                "tests";
        #else
        std::string directory = std::string("\"./") + env_prefix + envs[current_env].get_name() + "/" +
                task_prefix + envs[current_env].get_tasks()[current_task].get_name() + "/"
                "tests";
        #endif  // _WIN32
        chdir(directory.c_str());
        try {
            command = get_setting_by_name("runner_" + current_runner);
        } catch (std::runtime_error &) {
            #ifdef _WIN32
            command = "generator.exe";
            #else
            command = "./generator";
            #endif  // _WIN32
        }
        DEBUG_LOG(command);
        replace_all(command, "@name@", (fs::path(env_prefix + envs[current_env].get_name()) / 
                            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
                            "tests" / "generator").string());
        replace_all(command, "@lang@", current_runner);
        std::cout << "\033[35m" << "-- Run generator for " <<
            envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
        DEBUG_LOG(command);
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        chdir("../../..");
        std::cout << "\033[35m\n" << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m\n";
        return ret_code;
    });

    add_command(State::GENERATOR, "eg", "Edit generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::path(env_prefix + envs[current_env].get_name()) /
            (task_prefix + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / "generator";
        std::string lang = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        std::string command = get_setting_by_name("editor");
        replace_all(command, "@name@", file_path.string());
        replace_all(command, "@lang@", lang);
        DEBUG_LOG(command);
        auto ampersand_pos = command.find("&");
        #ifdef _WIN32
        if (ampersand_pos != std::string::npos) {
            command.erase(ampersand_pos);
            command += " > NUL";
            std::thread thr([&](const std::string command) -> void {
                DEBUG_LOG(command);
                int res = system(command.c_str());
                (void)res;
            }, command);
            thr.detach();
            return 0;
        } else {
            DEBUG_LOG(command);
            return system(command.c_str());
        }
        #else
        if (ampersand_pos != std::string::npos) {
            command.insert(std::max(0ul, ampersand_pos - 1), " &> /dev/null ");
        }
        DEBUG_LOG(command);
        return system(command.c_str());
        #endif  // _WIN32
    });
    add_alias(State::GENERATOR, "eg", State::GENERATOR, "edit");

    add_command(State::GENERATOR, "q", "Exit from generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_state = State::TASK;
        return 0;
    });
    add_alias(State::GENERATOR, "q", State::GENERATOR, "exit");

    add_alias(State::GLOBAL, "help", State::GENERATOR, "help");
    add_alias(State::GLOBAL, "help", State::GENERATOR, "?");
}

}  // namespace comproenv

#include <string>
#include <fstream>
#include <thread>
#include <experimental/filesystem>
#include "shell.h"
#include "task.h"

namespace fs = std::experimental::filesystem;

void Shell::configure_commands_generator() {
    add_command(State::GENERATOR, "cg", "Compile generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_compiler = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        if (envs[current_env].get_tasks()[current_task].get_settings().find("compiler_" + current_compiler) !=
            envs[current_env].get_tasks()[current_task].get_settings().end())
            command = envs[current_env].get_tasks()[current_task].get_settings()["compiler_" + current_compiler];
        else if (envs[current_env].get_settings().find("compiler_" + current_compiler) !=
                envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["compiler_" + current_compiler];
        else if (global_settings.find("compiler_" + current_compiler) != global_settings.end())
            command = global_settings["compiler_" + current_compiler];
        else {
            std::cout << "There is no compiler rule for language " <<
                envs[current_env].get_tasks()[current_task].get_settings()["language"] << '\n';
            return -1;
        }
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            "tests" / "generator").string());
        }
        std::cout << "\033[35m" << "-- Compile generator for " <<
            envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
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
        std::string command;
        std::string current_runner = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        if (envs[current_env].get_tasks()[current_task].get_settings().find("runner_" + current_runner) !=
            envs[current_env].get_tasks()[current_task].get_settings().end())
            command = envs[current_env].get_tasks()[current_task].get_settings()["runner_" + current_runner];
        else if (envs[current_env].get_settings().find("runner_" + current_runner) !=
                envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["runner_" + current_runner];
        else if (global_settings.find("runner_" + current_runner) != global_settings.end())
            command = global_settings["runner_" + current_runner];
        else {
            #ifdef _WIN32
            command = "env_" + envs[current_env].get_name() + "\\" +
                "task_" + envs[current_env].get_tasks()[current_task].get_name() + "\\" +
                "tests" + "\\" + "generator" + ".exe";
            #else
            command = std::string("./") + "env_" + envs[current_env].get_name() + "/" +
                "task_" + envs[current_env].get_tasks()[current_task].get_name() + "/" +
                "tests" + "/" + "generator";
            #endif  // _WIN32
        }
        std::cout << "cmd: " << command << '\n';
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            "tests" / "generator").string());
        }
        std::cout << "\033[35m" << "-- Run generator for " <<
            envs[current_env].get_tasks()[current_task].get_name() << ":" <<
            "\033[0m\n";
        auto time_start = std::chrono::high_resolution_clock::now();
        int ret_code = system(command.c_str());
        auto time_finish = std::chrono::high_resolution_clock::now();
        std::cout << "\033[35m\n" << "-- Time elapsed:" <<
            std::chrono::duration_cast<std::chrono::duration<double>>(time_finish - time_start).count() <<
            "\033[0m\n";
        return ret_code;
    });

    add_command(State::GENERATOR, "eg", "Edit generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        fs::path file_path = fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
            "tests" / "generator";
        std::string lang = envs[current_env].get_tasks()[current_task].get_settings()["generator"];
        std::string command = "";
        std::cout << "gce: " << global_settings["editor"] << '\n';
        if (global_settings.find("editor") != global_settings.end())
            command = global_settings["editor"];
        if (envs[current_env].get_settings().find("editor") != envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["editor"];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1, file_path.string());
        }
        pos = std::string::npos;
        while ((pos = command.find("@lang@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@lang@") - 1, lang);
        }
        std::cout << "cmd: " << command << '\n';
        auto ampersand_pos = command.find("&");
        #ifdef _WIN32
        if (ampersand_pos != std::string::npos) {
            command.erase(ampersand_pos);
            command += " > NUL";
            std::thread thr([&](const std::string command) -> void {
                int res = system(command.c_str());
                (void)res;
            }, command);
            thr.detach();
            return 0;
        } else {
            return system(command.c_str());
        }
        #else
        if (ampersand_pos != std::string::npos) {
            command.insert(std::max(0ul, ampersand_pos - 1), " &> /dev/null ");
        }
        return system(command.c_str());
        #endif  // _WIN32
    });

    add_command(State::GENERATOR, "q", "Exit from generator",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_state = State::TASK;
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        } else {
            return 0;
        }
    });
    add_alias(State::TASK, "q", State::TASK, "exit");

    add_alias(State::GLOBAL, "help", State::GENERATOR, "help");
    add_alias(State::GLOBAL, "help", State::GENERATOR, "?");
}

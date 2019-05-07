#include <string>
#include <fstream>
#include <experimental/filesystem>
#include "shell.h"
#include "task.h"

namespace fs = std::experimental::filesystem;

Task::Task(const std::string &task_name) : name(task_name) {

}

std::string Task::get_name() const {
    return name;
}

void Task::add_setting(const std::string &key, const std::string &value) {
    settings.emplace(key, value);
}

std::unordered_map <std::string, std::string> &Task::get_settings() {
    return settings;
}

void Shell::configure_commands_task() {
    // Compile task
    add_command(State::TASK, "c", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string current_compiler = envs[current_env].get_tasks()[current_task].get_settings()["language"];
        if (envs[current_env].get_tasks()[current_task].get_settings().find("compiler_" + current_compiler) !=
            envs[current_env].get_tasks()[current_task].get_settings().end())
            command = envs[current_env].get_tasks()[current_task].get_settings()["compiler_" + current_compiler];
        else if (envs[current_env].get_settings().find("compiler_" + current_compiler) !=
                envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["compiler_" + current_compiler];
        else
            command = global_settings["compiler_" + current_compiler];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        }
        return system(command.c_str());
    });

    // Run task
    add_command(State::TASK, "r", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        #ifdef _WIN32
        command = "env_" + envs[current_env].get_name() + "\\" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name() + "\\" +
            envs[current_env].get_tasks()[current_task].get_name() + ".exe";
        #else
        command = std::string("./") + "env_" + envs[current_env].get_name() + "/" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name() + "/" +
            envs[current_env].get_tasks()[current_task].get_name();
        #endif  // _WIN32
        return system(command.c_str());
    });

    // Test task
    add_command(State::TASK, "t", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command;
        std::string path;
        std::vector <fs::path> in_files;
        fs::recursive_directory_iterator it_begin(fs::current_path() / ("env_" + envs[current_env].get_name()) /
            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) / "tests"), it_end;
        std::copy_if(it_begin, it_end, std::back_inserter(in_files), [](const fs::path &path) {
            return fs::is_regular_file(path) && path.extension() == ".in";
        });
        for (auto &it : in_files)
            std::cout << it << std::endl;
        #ifdef _WIN32
        path = std::string("env_") + envs[current_env].get_name() + "\\" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name();
        #else
        path = "env_" + envs[current_env].get_name() + "/" +
            "task_" + envs[current_env].get_tasks()[current_task].get_name();
        #endif  // _WIN32
        int errors = 0, error_code = 0;
        for (auto &in_file : in_files) {
            std::cout << "\x1B[33m-- Test " << in_file << "\033[0m" << std::endl;
            std::string buf;
            std::ifstream f(in_file);
            while (std::getline(f, buf))
                std::cout << buf << std::endl;
            f.close();
            std::cout << "\x1B[35m-- Result:" << "\033[0m" << std::endl;
            #ifdef _WIN32
            command = path + "\\" +
                envs[current_env].get_tasks()[current_task].get_name() + " < " + std::string(in_file);
            #else
            command = std::string("./") + path + "/" +
                envs[current_env].get_tasks()[current_task].get_name() + " < " + std::string(in_file);
            #endif  // _WIN32
            error_code = system(command.c_str());
            if (error_code) {
                std::cout << "\x1B[31m-- Runtime error!\033[0m" << std::endl;
                ++errors;
            }
            std::string out_file = in_file;
            for (int i = 0; i < 2; ++i)
                out_file.pop_back();
            out_file.append("out");
            f.open(out_file);
            if (f.is_open()) {
                std::cout << "\x1B[35m-- Expected:" << "\033[0m" << std::endl;
                while (std::getline(f, buf))
                    std::cout << buf << std::endl;
                f.close();
            }
            std::cout << "\x1B[33m-- End of test " << in_file << "\033[0m" << std::endl;
        }
        return errors;
    });

    // Configure settings
    add_command(State::TASK, "set", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
            return 0;
        }
        if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_tasks()[current_task].get_settings().erase(arg[1]);
            envs[current_env].get_tasks()[current_task].get_settings().emplace(arg[1], second_arg);
            return 0;
        }
        throw std::runtime_error("Incorrect arguments for command " + arg[0]);
    });

    // Edit
    add_command(State::TASK, "edit", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command = "";
        std::cout << "gce: " << global_settings["editor"] << std::endl;
        if (global_settings.find("editor") != global_settings.end())
            command = global_settings["editor"];
        if (envs[current_env].get_settings().find("editor") != envs[current_env].get_settings().end())
            command = envs[current_env].get_settings()["editor"];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@name@") - 1,
                            (fs::current_path() / ("env_" + envs[current_env].get_name()) / 
                            ("task_" + envs[current_env].get_tasks()[current_task].get_name()) /
                            envs[current_env].get_tasks()[current_task].get_name()).string());
        }
        pos = std::string::npos;
        while ((pos = command.find("@lang@")) != std::string::npos) {
            command.replace(command.begin() + pos, command.begin() + pos + std::size("@lang@") - 1,
                            envs[current_env].get_tasks()[current_task].get_settings()["language"]);
        }
        std::cout << "cmd: " << command << std::endl;
        return system(command.c_str());
    });

    // Exit from task
    add_command(State::TASK, "q", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_task = -1;
        current_state = State::ENVIRONMENT;
        return 0;
    });
    add_alias(State::TASK, "q", State::TASK, "exit");
}

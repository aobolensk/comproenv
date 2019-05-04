#include <fstream>
#include <experimental/filesystem>
#include "shell.h"

namespace fs = std::experimental::filesystem;

void Shell::configure_commands_environment() {
    // Set task
    add_command(State::ENVIRONMENT, "st", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                current_task = i;
                current_state = State::TASK;
                return 0;
            }
        }
        throw std::runtime_error("Incorrect task name");
    });

    // Create task
    add_command(State::ENVIRONMENT, "ct", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() < 2 || arg.size() > 3)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i)
            if (envs[current_env].get_tasks()[i].get_name() == arg[1])
                throw std::runtime_error("Task named " + arg[1] + " already exists");
        envs[current_env].get_tasks().push_back(arg[1]);
        fs::path path = fs::current_path() / ("env_" + envs[current_env].get_name()) / ("task_" + arg[1]);
        if (arg.size() == 2) {
            std::string lang = "cpp";
            if (global_settings.find("language") != global_settings.end())
                lang = global_settings["language"];
            if (envs[current_env].get_settings().find("language") != envs[current_env].get_settings().end())
                lang = envs[current_env].get_settings()["language"];
            envs[current_env].get_tasks().back().get_settings().emplace("language", lang);
        } else if (arg.size() == 3) {
            envs[current_env].get_tasks().back().get_settings().emplace("language", arg[2]);
        }
        if (!fs::exists(path)) {
            fs::create_directory(path);
        }
        if (!fs::exists(path / ("main." + envs[current_env].get_tasks().back().get_settings()["language"]))) {
            std::ofstream f(path / ("main." + envs[current_env].get_tasks().back().get_settings()["language"]), std::ios::out);
            f.close();
        }
        return 0;
    });

    // Delete task
    add_command(State::ENVIRONMENT, "dt", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                envs[current_env].get_tasks().erase(envs[current_env].get_tasks().begin() + i);
                fs::path path = fs::current_path() / ("env_" + envs[current_env].get_name()) / ("task_" + arg[1]);
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        throw std::runtime_error("Incorrect task name");
    });

    // Configure settings
    add_command(State::ENVIRONMENT, "set", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_settings().erase(arg[1]);
            return 0;
        }
        if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_settings().erase(arg[1]);
            envs[current_env].get_settings().emplace(arg[1], second_arg);
            return 0;
        }
        throw std::runtime_error("Incorrect arguments for command " + arg[0]);
    });

    // Exit from environment
    add_command(State::ENVIRONMENT, "q", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_env = -1;
        current_state = State::GLOBAL;
        return 0;
    });
    add_alias(State::ENVIRONMENT, "q", State::ENVIRONMENT, "exit");
}

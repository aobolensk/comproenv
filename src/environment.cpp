#include <fstream>
#include <experimental/filesystem>
#include "shell.h"
#include "environment.h"

namespace comproenv {

namespace fs = std::experimental::filesystem;

Environment::Environment(const std::string &env_name) : name(env_name) {

}

void Environment::add_task(const Task &task) {
    tasks.emplace_back(task);
}

void Environment::add_setting(const std::string &key, const std::string &value) {
    settings.emplace(key, value);
}

std::string Environment::get_name() const {
    return name;
}

std::vector <Task> &Environment::get_tasks() {
    return tasks;
}


std::map <std::string, std::string> &Environment::get_settings() {
    return settings;
}

void Shell::configure_commands_environment() {
    add_command(State::ENVIRONMENT, "st", "Set task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                current_task = (int)i;
                current_state = State::TASK;
                return 0;
            }
        }
        throw std::runtime_error("Incorrect task name");
    });

    add_command(State::ENVIRONMENT, "ct", "Create task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() < 2 || arg.size() > 3)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i)
            if (envs[current_env].get_tasks()[i].get_name() == arg[1])
                throw std::runtime_error("Task named " + arg[1] + " already exists");
        envs[current_env].get_tasks().push_back(arg[1]);
        fs::path path = fs::path("env_" + envs[current_env].get_name()) / ("task_" + arg[1]);
        std::string lang;
        if (arg.size() == 2) {
            try {
                lang = get_setting_by_name("language");
            } catch (std::runtime_error &) {
                lang = "cpp";
            }
            envs[current_env].get_tasks().back().get_settings().emplace("language", lang);
        } else if (arg.size() == 3) {
            lang = arg[2];
            envs[current_env].get_tasks().back().get_settings().emplace("language", lang);
        }
        if (!fs::exists(path)) {
            fs::create_directory(path);
        }
        if (!fs::exists(path / (arg[1] + "." + lang))) {
            std::ofstream f(path / (arg[1] + "." + lang), std::ios::out);
            if (!f.is_open()) {
                return -1;
            }
            std::string file_name;
            try {
                file_name = get_setting_by_name("template_" + lang);
                DEBUG_LOG("Template file: " + file_name);
                std::ifstream t(file_name);
                if (t.is_open()) {
                    std::string buf;
                    while (std::getline(t, buf))
                        f << buf << '\n';
                    t.close();
                } else {
                    std::cout << "Unable to open template file\n";
                }
            } catch (std::runtime_error &) {
                file_name = fs::path("templates") / lang;
                DEBUG_LOG("Default template file: " + file_name);
                if (fs::is_regular_file(file_name)) {
                    std::ifstream t(file_name);
                    if (t.is_open()) {
                        std::string buf;
                        while (std::getline(t, buf))
                            f << buf << '\n';
                        t.close();
                    } else {
                        std::cout << "Unable to open default template file\n";
                    }
                } else {
                    std::cout << "Template for language " + lang + " is not found. "
                                    "Created empty file\n";
                }
            }
            f.close();
        }
        if (!fs::exists(path / "tests")) {
            fs::create_directory(path / "tests");
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "rt", "Remove task",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                envs[current_env].get_tasks().erase(envs[current_env].get_tasks().begin() + i);
                fs::path path = fs::path("env_" + envs[current_env].get_name()) / ("task_" + arg[1]);
                if (global_settings["autosave"] == "on") {
                    std::vector <std::string> save_args = {"s"};
                    commands[State::GLOBAL][save_args.front()](save_args);
                }
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        throw std::runtime_error("Incorrect task name");
    });

    add_command(State::ENVIRONMENT, "lt", "List of tasks",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::cout << "List of tasks in environment " << envs[current_env].get_name() << "\n";
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            std::cout << "    |-> " << envs[current_env].get_tasks()[i].get_name() << ": " <<
                envs[current_env].get_tasks()[i].get_settings()["language"] << "\n";
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "set", "Configure environment settings",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_settings().erase(arg[1]);
        }
        if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_settings().erase(arg[1]);
            envs[current_env].get_settings().emplace(arg[1], second_arg);
        } else {
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "q", "Exit from environment",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_env = -1;
        current_state = State::GLOBAL;
        return 0;
    });
    add_alias(State::ENVIRONMENT, "q", State::ENVIRONMENT, "exit");

    add_alias(State::GLOBAL, "help", State::ENVIRONMENT, "help");
    add_alias(State::GLOBAL, "help", State::ENVIRONMENT, "?");
}

}  // namespace comproenv

#include <fstream>
#include "fs.h"
#include "const.h"
#include "shell.h"
#include "utils.h"

namespace comproenv {

void Shell::configure_commands_environment() {
    add_command(State::ENVIRONMENT, "st", "Set task",
    "st t1 <- enter the task with name 't1'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                current_task = (int)i;
                current_state = State::TASK;
                store_cache();
                set_console_title();
                return 0;
            }
        }
        FAILURE("Incorrect task name");
    });

    add_command(State::ENVIRONMENT, "ct", "Create task",
    "ct t1 <- create a task with name 't1' in C++ (default language)\n"
    "ct t1 py <- create a task with name 't1' in Python\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() < 2 || arg.size() > 3)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i)
            if (envs[current_env].get_tasks()[i].get_name() == arg[1])
                FAILURE("Task named " + arg[1] + " already exists");
        envs[current_env].get_tasks().emplace_back(arg[1]);
        fs::path path = fs::path(env_prefix + envs[current_env].get_name()) / (task_prefix + arg[1]);
        std::string lang;
        if (arg.size() == 2) {
            lang = get_setting_by_name("language").value_or("cpp");
            envs[current_env].get_tasks().back().get_settings().emplace("language", lang);
        } else if (arg.size() == 3) {
            lang = arg[2];
            envs[current_env].get_tasks().back().get_settings().emplace("language", lang);
        }
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
        if (!fs::exists(path / (arg[1] + "." + lang))) {
            std::ofstream f(path / (arg[1] + "." + lang), std::ios::out);
            if (!f.is_open()) {
                return -1;
            }
            std::string file_name = get_setting_by_name("template_" + lang)
                .value_or((fs::path("templates") / lang).string());
            DEBUG_LOG("Template file: " + file_name);
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
        if (!fs::exists(path / "tests")) {
            fs::create_directories(path / "tests");
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "rt", "Remove task",
    "rt t1 <- remove the task with name 't1'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            if (envs[current_env].get_tasks()[i].get_name() == arg[1]) {
                envs[current_env].get_tasks().erase(envs[current_env].get_tasks().begin() + i);
                fs::path path = fs::path(env_prefix + envs[current_env].get_name()) / (task_prefix + arg[1]);
                if (global_settings["autosave"] == "on") {
                    std::vector <std::string> save_args = {"s"};
                    commands[State::GLOBAL][save_args.front()](save_args);
                }
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        FAILURE("Incorrect task name");
    });

    add_command(State::ENVIRONMENT, "lt", "List of tasks",
    "lt <- show list of all available tasks\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "List of tasks in environment " << envs[current_env].get_name() << "\n";
        for (size_t i = 0; i < envs[current_env].get_tasks().size(); ++i) {
            std::cout << "    |-> " << envs[current_env].get_tasks()[i].get_name() << ": " <<
                envs[current_env].get_tasks()[i].get_settings()["language"] << "\n";
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "ee", "Edit environment",
    "ee <- edit environment\n"
    "You will get a list of editable settings, where you can either edit option or "
    "leave it as is (by pressing Enter)\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string buf;
        std::error_code e;
        do {
            e.clear();
            std::cout << "Name [" << envs[current_env].get_name() << "]: ";
            std::getline(std::cin, buf);
            if (buf.size() > 0) {
                fs::rename(env_prefix + envs[current_env].get_name(), env_prefix + buf, e);
                if (e.value() != 0) {
                    std::cout << "Rename error: " << e.message() << std::endl;
                } else {
                    envs[current_env].set_name(buf);
                    std::cout << "Set environment name: " << buf << '\n';
                }
            }
        } while (e.value() != 0);
        return 0;
    });

    add_command(State::ENVIRONMENT, "set", "Configure environment settings",
    "set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17 -O3 <- set compiler command for C++\n"
    "set editor notepad @name@.@lang@ & <- set editor to notepad"
    "set max_history_size 32 <- set history size buffer to 32 entries\n"
    "set python_interpreter python <- set path to python interpreter\n"
    "set runner_py python @name@.@lang@ <- set runner for Python\n"
    "set template_cpp templates/cpp <- set path to template file for C++\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_settings().erase(arg[1]);
        } else if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            envs[current_env].get_settings().erase(arg[1]);
            envs[current_env].get_settings().emplace(arg[1], second_arg);
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "unset", "Delete environment setting",
    "unset runner_py <- delete runner for Python\n"
    "unset template_cpp <- delete template for C++\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            envs[current_env].get_settings().erase(arg[1]);
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::ENVIRONMENT, "q", "Exit from environment",
    "q <- exit from the environment\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        current_env = -1;
        current_state = State::GLOBAL;
        store_cache();
        set_console_title();
        return 0;
    });
    add_alias(State::ENVIRONMENT, "q", State::ENVIRONMENT, "exit");

    add_alias(State::GLOBAL, "help", State::ENVIRONMENT, "help");
    add_alias(State::GLOBAL, "help", State::ENVIRONMENT, "?");
}

}  // namespace comproenv

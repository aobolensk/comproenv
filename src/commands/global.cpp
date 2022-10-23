#include <fstream>
#include <vector>
#include "fs.h"
#include <csignal>
#include <ctime>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX
#include <Windows.h>
#pragma warning(disable: 4996)
#endif  // _WIN32
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <sys/utsname.h>
#endif
#include "const.h"
#include "environment.h"
#include "yaml_parser.h"
#include "shell.h"
#include "utils.h"

namespace comproenv {

void Shell::configure_commands_global() {
    add_command(State::GLOBAL, "se", "Set environment",
    "se e1 <- enter the environment with name 'e1'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i) {
            if (envs[i].get_name() == arg[1]) {
                current_env = (int)i;
                current_state = State::ENVIRONMENT;
                store_cache();
                set_console_title();
                return 0;
            }
        }
        FAILURE("Incorrect environment name");
    });

    add_command(State::GLOBAL, "ce", "Create environment",
    "ce e1 <- create an environment with name 'e1'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i)
            if (envs[i].get_name() == arg[1])
                FAILURE("Environment named " + arg[1] + " already exists");
        envs.emplace_back(arg[1]);
        fs::path path = fs::path(env_prefix + arg[1]);
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::GLOBAL, "re", "Remove environment",
    "re e1 <- remove the environment with name 'e1'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs.size(); ++i) {
            if (envs[i].get_name() == arg[1]) {
                std::string buf;
                std::cout << "Are you sure? [y/n]: " << std::flush;
                std::getline(std::cin, buf);
                if (tolower(buf[0]) != 'y') {
                    std::cout << "Removing " << arg[1] << " environment is cancelled" << '\n';
                    return 1;
                }
                envs.erase(envs.begin() + i);
                fs::path path = fs::path(env_prefix + arg[1]);
                if (global_settings["autosave"] == "on") {
                    std::vector <std::string> save_args = {"s"};
                    commands[State::GLOBAL][save_args.front()](save_args);
                }
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        FAILURE("Incorrect environment name");
    });

    add_command(State::GLOBAL, "le", "List of environments",
    "le <- show list of all available environments\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "List of environments in global:\n";
        for (size_t i = 0; i < envs.size(); ++i) {
            std::cout << "|-> " << envs[i].get_name() << "\n";
            for (size_t j = 0; j < std::min(size_t(3), envs[i].get_tasks().size()); ++j) {
                std::cout << "    |-> " << envs[i].get_tasks()[j].get_name() << ": " <<
                    envs[i].get_tasks()[j].get_settings()["language"] << "\n";
            }
            if (envs[i].get_tasks().size() > 3) {
                std::cout << "    (and " << envs[i].get_tasks().size() - 3ul << " more...)" "\n";
            }
        }
        return 0;
    });

    add_command(State::GLOBAL, "les", "List of environments (short: only names)",
    "les <- show list of all available environments\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "List of environments in global:\n";
        for (size_t i = 0; i < envs.size(); ++i) {
            std::cout << "|-> " << envs[i].get_name() << "\n";
        }
        return 0;
    });

    add_command(State::GLOBAL, "lef", "List of environments (full)",
    "lef <- show list of all available environments\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "List of environments in global:\n";
        for (size_t i = 0; i < envs.size(); ++i) {
            std::cout << "|-> " << envs[i].get_name() << "\n";
            for (size_t j = 0; j < envs[i].get_tasks().size(); ++j) {
                std::cout << "    |-> " << envs[i].get_tasks()[j].get_name() << ": " <<
                    envs[i].get_tasks()[j].get_settings()["language"] << "\n";
            }
        }
        return 0;
    });

    add_command(State::GLOBAL, "s", "Save settings",
    "s <- save settings\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        int indent = 0;
        std::ofstream f(config_file, std::ios::out);

        auto serialize_mapping = [&](const std::vector <std::pair <std::string, std::string>> &instances,
                                    const std::string instances_name) -> void {
            if (instances.size()) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << instances_name << ":" << std::endl;
                indent += 2;
                for (auto &instance : instances) {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    f << instance.first << ": " << instance.second << std::endl;
                }
                indent -= 2;
            }
        };

        auto serialize_sequence = [&](const std::vector <std::string> &instances,
                                    const std::string instances_name) -> void {
            if (instances.size()) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << instances_name << ":" << std::endl;
                indent += 2;
                for (auto &instance : instances) {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    std::string str = instance;
                    replace_all(str, "\\", "\\\\");
                    f << "- " << '\"' << str << '\"' << std::endl;
                }
                indent -= 2;
            }
        };

        auto serialize_settings = [&](std::map <std::string, std::string> &settings) {
            std::vector <std::pair <std::string, std::string>> compilers, runners, templates, aliases;

            for (auto &setting : settings) {
                if (setting.first.compare(0, std::size("compiler_") - 1, "compiler_") == 0) {
                    compilers.emplace_back(setting.first.substr(std::size("compiler_") - 1), setting.second);
                } else if (setting.first.compare(0, std::size("runner_") - 1, "runner_") == 0) {
                    runners.emplace_back(setting.first.substr(std::size("runner_") - 1), setting.second);
                } else if (setting.first.compare(0, std::size("template_") - 1, "template_") == 0) {
                    templates.emplace_back(setting.first.substr(std::size("template_") - 1), setting.second);
                } else if (setting.first.compare(0, std::size("alias_") - 1, "alias_") == 0) {
                    aliases.emplace_back(setting.first.substr(std::size("alias_") - 1), setting.second);
                } else {
                    for (int i = 0; i < indent; ++i)
                        f << " ";
                    f << setting.first << ": " << setting.second << std::endl;
                }
            }
            serialize_mapping(compilers, "compilers");
            serialize_mapping(runners, "runners");
            serialize_mapping(templates, "templates");
            serialize_mapping(aliases, "aliases");
        };
        if (global_settings.size()) {
            f << "global:" << std::endl;
            indent += 2;
            serialize_settings(global_settings);
            serialize_sequence(commands_history.get_all(), "commands_history");
            indent -= 2;
        }
        f.close();
        if (envs.size()) {
            f.open(environments_file, std::ios::out);
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
            f.close();
        }
        return 0;
    });

    add_command(State::GLOBAL, "py-shell", "Launch Python shell",
    "py-shell <- launch Python shell\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (!get_setting_by_name("python_interpreter").has_value()) {
            FAILURE("Python interpreter is not found!");
        }
        DEBUG_LOG(get_setting_by_name("python_interpreter").value());
        return system(get_setting_by_name("python_interpreter").value().c_str());
    });
    add_alias(State::GLOBAL, "py-shell", State::ENVIRONMENT, "py-shell");
    add_alias(State::GLOBAL, "py-shell", State::TASK, "py-shell");
    add_alias(State::GLOBAL, "py-shell", State::GENERATOR, "py-shell");

    add_command(State::GLOBAL, "autosave", "Toggle autosave",
    "autosave <- toggle autosave (if it was 'on' it will be 'off' and vice versa)\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::string state = "on";
        auto it = global_settings.find("autosave");
        if (it != global_settings.end()) {
            state = it->second;
            global_settings.erase(it);
        }
        if (state == "on")
            state = "off";
        else if (state == "off")
            state = "on";
        else
            FAILURE("Unknown state for autosave");
        global_settings["autosave"] = state;
        std::cout << "Set autosave to " << state << std::endl;
        return 0;
    });
    add_alias(State::GLOBAL, "autosave", State::ENVIRONMENT, "autosave");
    add_alias(State::GLOBAL, "autosave", State::TASK, "autosave");
    add_alias(State::GLOBAL, "autosave", State::GENERATOR, "autosave");

    add_command(State::GLOBAL, "history", "Show commands history",
    "history <- show commands history\n"
    "Commands history length can be set using: set max_history_size <new_history_size>\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        auto history = commands_history.get_all();
        std::cout << "Commands history:\n";
        for (const auto &command : history) {
            std::cout << command << '\n';
        }
        return 0;
    });
    add_alias(State::GLOBAL, "history", State::ENVIRONMENT, "history");
    add_alias(State::GLOBAL, "history", State::TASK, "history");
    add_alias(State::GLOBAL, "history", State::GENERATOR, "history");

    add_command(State::GLOBAL, "reload-settings", "Hot reload settings from config file",
    "reload-settings <- reload settings from config.yaml file\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        YAMLParser config_parser(config_file);
        YAMLParser::Mapping config = config_parser.parse().get_mapping();
        YAMLParser environments_parser(config_file);
        YAMLParser::Mapping environments = environments_parser.parse().get_mapping();
        global_settings.clear();
        envs.clear();
        parse_settings(config, environments);
        configure_user_defined_aliases();
        create_paths();
        current_env = -1;
        current_task = -1;
        current_state = State::GLOBAL;
        store_cache();
        set_console_title();
        return 0;
    });
    add_alias(State::GLOBAL, "reload-settings", State::ENVIRONMENT, "reload-settings");
    add_alias(State::GLOBAL, "reload-settings", State::TASK, "reload-settings");
    add_alias(State::GLOBAL, "reload-settings", State::GENERATOR, "reload-settings");

    add_command(State::GLOBAL, "reload-envs", "Reload all environments and tasks\nfrom comproenv directory",
    "reload-envs <- reload environments from environments.yaml file\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        envs.clear();
        for (auto &p : fs::directory_iterator(".")) {
            std::string env_dir = p.path().filename().string();
            if (env_dir.find(env_prefix) == 0) {
                std::string env_name = env_dir.substr(env_prefix.size(), env_dir.size() - env_prefix.size());
                envs.push_back(Environment(env_name));
                for (auto &q : fs::directory_iterator(env_dir)) {
                    std::string task_dir = q.path().filename().string();
                    std::string task_name = task_dir.substr(task_prefix.size(), task_dir.size() - task_prefix.size());
                    if (task_dir.find(task_prefix) == 0) {
                        envs.back().get_tasks().push_back(Task(task_name));
                        for (auto &r : fs::directory_iterator(fs::path(env_dir) / fs::path(task_dir))) {
                            if (r.path().has_extension()) {
                                std::string task_lang = r.path().extension().string().substr(1);
                                if (task_lang != "in" && task_lang != "out" && task_lang != "exe") {
                                    envs.back().get_tasks().back().add_setting("language", task_lang);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
    });
    add_alias(State::GLOBAL, "reload-envs", State::ENVIRONMENT, "reload-envs");
    add_alias(State::GLOBAL, "reload-envs", State::TASK, "reload-envs");
    add_alias(State::GLOBAL, "reload-envs", State::GENERATOR, "reload-envs");

    add_command(State::GLOBAL, "set", "Configure global settings",
    "set compiler_cpp g++ @name@.@lang@ -o @name@ -std=c++17 -O3 <- set compiler command for C++\n"
    "set editor notepad @name@.@lang@ & <- set editor to notepad\n"
    "set max_history_size 32 <- set history size buffer to 32 entries\n"
    "set python_interpreter python <- set path to python interpreter\n"
    "set runner_py python @name@.@lang@ <- set runner for Python\n"
    "set template_cpp templates/cpp <- set path to template file for C++\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            global_settings.erase(arg[1]);
        } else if (arg.size() >= 3) {
            std::string second_arg = arg[2];
            for (unsigned i = 3; i < arg.size(); ++i) {
                second_arg.push_back(' ');
                second_arg += arg[i];
            }
            global_settings.erase(arg[1]);
            global_settings.emplace(arg[1], second_arg);
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::GLOBAL, "unset", "Delete global setting",
    "unset runner_py <- delete runner for Python\n"
    "unset template_cpp <- delete template for C++\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() == 2) {
            global_settings.erase(arg[1]);
        } else {
            FAILURE("Incorrect arguments for command " + arg[0]);
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });

    add_command(State::GLOBAL, "sets", "Print settings",
    "sets <- print all settings in key:value format\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() > 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (global_settings.size()) {
            std::cout << "Settings in " << state_names[State::GLOBAL] << ":\n";
            for (const auto &it : global_settings) {
                std::cout << "    \"" << it.first << "\" : \"" << it.second << "\"\n";
            }
        }
        if (current_env != -1 && envs[current_env].get_settings().size()) {
            std::cout << "Settings in " << state_names[State::ENVIRONMENT] << ":\n";
            for (const auto &it : envs[current_env].get_settings()) {
                std::cout << "    \"" << it.first << "\" : \"" << it.second << "\"\n";
            }
        }
        if (current_task != -1 && envs[current_env].get_tasks()[current_task].get_settings().size()) {
            std::cout << "Settings in " << state_names[State::TASK] << ":\n";
            for (const auto &it : envs[current_env].get_tasks()[current_task].get_settings()) {
                std::cout << "    \"" << it.first << "\" : \"" << it.second << "\"\n";
            }
        }
        return 0;
    });
    add_alias(State::GLOBAL, "sets", State::ENVIRONMENT, "sets");
    add_alias(State::GLOBAL, "sets", State::TASK, "sets");

    add_command(State::GLOBAL, "q", "Exit from program",
    "q <- exit\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "Exiting..." << std::endl;
        if (remove((fs::current_path() / cache_file_name).string().c_str())) {
            std::cout << "Unable to remove cache file" << std::endl;
        }
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            int res = commands[State::GLOBAL][save_args.front()](save_args);
            exit(res);
        } else {
            exit(0);
        }
    });
    add_alias(State::GLOBAL, "q", State::GLOBAL, "exit");

    add_command(State::GLOBAL, "alias", "Define aliases for commands",
    "alias se cd <- create alias 'cd' for command 'se'\n"
    "alias q quit <- create alias 'quit' for command 'q'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 3)
            FAILURE("Incorrect arguments for command " + arg[0]);
        add_alias(current_state, arg[1], current_state, arg[2]);
        auto it = global_settings.find("alias_" + state_names[current_state]);
        if (it == global_settings.end()) {
            global_settings.emplace("alias_" + state_names[current_state], "");
            it = global_settings.find("alias_" + state_names[current_state]);
        }
        if (std::size(it->second) && it->second.back() != ' ')
            it->second.push_back(' ');
        it->second += arg[1] + ' ' + arg[2] + ' ';
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });
    add_alias(State::GLOBAL, "alias", State::ENVIRONMENT, "alias");
    add_alias(State::GLOBAL, "alias", State::TASK, "alias");
    add_alias(State::GLOBAL, "alias", State::GENERATOR, "alias");

    add_command(State::GLOBAL, "delete-alias", "Delete aliases for commands",
    "delete-alias se <- delete all aliases for command 'se'\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        auto it = global_settings.find("alias_" + state_names[current_state]);
        if (it == global_settings.end())
            FAILURE("Aliases in state " + state_names[current_state] + " are not found");
        std::vector <std::string> aliases;
        split(aliases, it->second);
        std::function <void(const std::string_view)> delete_aliases =
        [&](const std::string_view str) {
            for (size_t i = 0; i < aliases.size(); ) {
                if (aliases[i] == str) {
                    delete_aliases(aliases[i + 1]);
                    aliases.erase(aliases.begin() + i, aliases.begin() + i + 2);
                } else {
                    i += 2;
                }
            }
        };
        delete_aliases(arg[1]);
        it->second = join(" ", aliases);
        if (global_settings["autosave"] == "on") {
            std::vector <std::string> save_args = {"s"};
            return commands[State::GLOBAL][save_args.front()](save_args);
        }
        return 0;
    });
    add_alias(State::GLOBAL, "delete-alias", State::ENVIRONMENT, "delete-alias");
    add_alias(State::GLOBAL, "delete-alias", State::TASK, "delete-alias");
    add_alias(State::GLOBAL, "delete-alias", State::GENERATOR, "delete-alias");

    add_command(State::GLOBAL, "help", "Help",
    "help <- get list of commands\n"
    "help <command> <- get examples of using this command\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() > 2)
            FAILURE("Incorrect arguments for command " + arg[0]);
        if (arg.size() == 1) {
            std::cout << "Help:" << "\n";
            size_t max_name_length = 0, max_desc_length = 0;
            for (auto &help_info : help[current_state]) {
                for (const std::string_view command : help_info.second)
                    max_name_length = std::max(max_name_length, command.size());
                size_t current_line_length = 0;
                for (size_t i = 0; i < help_info.first.size(); ++i) {
                    if (help_info.first[i] == '\n') {
                        max_desc_length = std::max(max_desc_length, current_line_length);
                        current_line_length = 0;
                    } else {
                        ++current_line_length;
                    }
                }
                max_desc_length = std::max(max_desc_length, current_line_length);
            }
            for (size_t i = 0; i < max_name_length + 1; ++i)
                std::cout << '-';
            std::cout << '|';
            for (size_t i = 0; i < max_desc_length + 1; ++i)
                std::cout << '-';
            std::cout << '\n';
            for (auto &help_info : help[current_state]) {
                // Help info
                auto command = help_info.second.begin();
                size_t description_index = 0;
                while (description_index < help_info.first.size()) {
                    if (command != help_info.second.end()) {
                        for (size_t i = 0; i < max_name_length - command->size(); ++i)
                            std::cout << ' ';
                        std::cout << *command << " | ";
                        ++command;
                    } else {
                        for (size_t i = 0; i < max_name_length; ++i)
                            std::cout << ' ';
                        std::cout << *command << " | ";
                    }
                    bool endline_trigger = false;
                    while (description_index < help_info.first.size()) {
                        if (help_info.first[description_index] == '\n') {
                            endline_trigger = true;
                        }
                        std::cout << help_info.first[description_index];
                        ++description_index;
                        if (endline_trigger) {
                            break;
                        }
                    }
                }
                std::cout << '\n';
                while (command != help_info.second.end()) {
                    for (size_t i = 0; i < max_name_length - command->size(); ++i)
                        std::cout << ' ';
                    std::cout << *command << " |\n";
                    ++command;
                }
                // Separator
                for (size_t i = 0; i < max_name_length + 1; ++i)
                    std::cout << '-';
                std::cout << '|';
                for (size_t i = 0; i < max_desc_length + 1; ++i)
                    std::cout << '-';
                std::cout << '\n';
            }
        } else if (arg.size() == 2) {
            std::cout << examples[current_state][arg[1]];
        }
        return 0;
    });
    add_alias(State::GLOBAL, "help", State::GLOBAL, "?");

    add_command(State::GLOBAL, "docs", "Get link to online documentation",
    "docs <- get link to online documentation\n",
    [](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "Online docs: https://github.com/aobolensk/comproenv/tree/"
            TOSTRING(COMPROENV_HASH) "/docs\n";
        return 0;
    });
    add_alias(State::GLOBAL, "docs", State::ENVIRONMENT, "docs");
    add_alias(State::GLOBAL, "docs", State::TASK, "docs");
    add_alias(State::GLOBAL, "docs", State::GENERATOR, "docs");

    add_command(State::GLOBAL, "clear", "Clear the console screen",
    "clear <- clear the console screen\n",
    [](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        #ifndef _WIN32
        std::cout << "\033[2J\033[1;1H";
        #else
        system("cls");
        #endif  // _WIN32
        return 0;
    });
    add_alias(State::GLOBAL, "clear", State::ENVIRONMENT, "clear");
    add_alias(State::GLOBAL, "clear", State::TASK, "clear");
    add_alias(State::GLOBAL, "clear", State::GENERATOR, "clear");

    add_command(State::GLOBAL, "about", "Get information about comproenv executable\nand environment",
    "about <- get information about comproenv executable and environment\n",
    [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            FAILURE("Incorrect arguments for command " + arg[0]);
        std::cout << "Repo: https://github.com/aobolensk/comproenv.git\n";
        std::cout << "Commit: " TOSTRING(COMPROENV_HASH) "\n";
        std::cout << "3rd party dependencies:\n";
        std::cout << "    libyaml: " TOSTRING(COMPROENV_LIBYAML_HASH) "\n";
        tm build_time;
        memset(&build_time, 0, sizeof(tm));
        sscanf(TOSTRING(COMPROENV_BUILDTIME), "%d-%d-%d %d:%d:%d",
                &build_time.tm_year, &build_time.tm_mon, &build_time.tm_mday,
                &build_time.tm_hour, &build_time.tm_min, &build_time.tm_sec);
        build_time.tm_year -= 1900;
        build_time.tm_mon -= 1;
        time_t bt = mktime(&build_time);
        #if defined(_WIN32)
        bt -= _timezone;
        #elif defined(__FreeBSD__)
        // Bug: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=24590
        int timezone_ = []() {
            struct tm tm;
            time_t t = 0;
            tzset();
            localtime_r(&t, &tm);
            return -(tm.tm_gmtoff);
        }();
        bt -= timezone_;
        #else
        bt -= timezone;
        #endif  // _WIN32
        std::cout << "Build   time: ";
        tm btm;
        localtime_r(&bt, &btm);
        printf("%d-%02d-%02d %02d:%02d:%02d\n",
                1900 + btm.tm_year, 1 + btm.tm_mon, btm.tm_mday,
                btm.tm_hour, btm.tm_min, btm.tm_sec);
        std::cout << "Current time: ";
        time_t now = time(0);
        tm ctm;
        localtime_r(&now, &ctm);
        printf("%d-%02d-%02d %02d:%02d:%02d\n",
                1900 + ctm.tm_year, 1 + ctm.tm_mon, ctm.tm_mday,
                ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
        std::cout << "OS: ";
        #ifdef _WIN32
        OSVERSIONINFOEX info;
        ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx((LPOSVERSIONINFO)& info);
        std::cout << "Microsoft Windows " << info.dwMajorVersion << "." << info.dwMinorVersion << "\n";
        #elif defined(__unix__) || defined(__linux__) || defined(__APPLE__)
        utsname buf;
        if (uname(&buf)) {
            std::cout << "undefined *nix\n";
        } else {
            std::cout << buf.sysname << ' ' << buf.nodename << ' ' << buf.release << ' ' << buf.version << ' ' << buf.machine << "\n";
        }
        #else
        std::cout << "unknown\n";
        #endif
        std::cout << "Compiler: ";
        #ifdef __clang__
        std::cout << "clang++ " TOSTRING(__clang_major__) "." TOSTRING(__clang_minor__) "." TOSTRING(__clang_patchlevel__) "\n";
        #elif _MSC_FULL_VER
        std::cout << "MSVC " TOSTRING(_MSC_FULL_VER) "\n";
        #elif __GNUC__
        std::cout << "g++ " TOSTRING(__GNUC__) "." TOSTRING(__GNUC_MINOR__) "." TOSTRING(__GNUC_PATCHLEVEL__) "\n";
        #else
        std::cout << "unknown\n";
        #endif
        std::cout << "Python: ";
        std::string command = global_settings["python_interpreter"] + " --version 2>&1";
        #ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> stream(_popen(command.c_str(), "r"), _pclose);
        #else
        std::unique_ptr<FILE, decltype(&pclose)> stream(popen(command.c_str(), "r"), pclose);
        #endif
        char buffer[256];
        std::string result;
        while (fgets(buffer, 256, stream.get())) {
            result += buffer;
        }
        std::vector <std::string> s_result;
        split(s_result, result);
        if ((s_result[0] == "Python")) {
            std::cout << s_result[1];
        } else {
            std::cout << "not found\n";
        }
        return 0;
    });
}

}  // namespace comproenv

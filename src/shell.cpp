#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include <stdexcept>
#include "environment.h"
#include "yaml_parser.h"
#include "shell.h"
#include "utils.h"

namespace fs = std::experimental::filesystem;

Shell::Shell(const std::string &file) : config_file(file) {
    configure_commands();
    if (config_file != "") {
        YAMLParser p1(config_file);
        YAMLParser::Mapping p = p1.parse().get_mapping();
        parse_settings(p);
        create_paths();
    }
    current_env = -1;
    current_task = -1;
    current_state = State::GLOBAL;
}

void Shell::add_command(int state, std::string name, std::function<int(std::vector <std::string> &)> func) {
    commands[state].emplace(name, func);
}

void Shell::add_alias(int old_state, std::string old_name, int new_state, std::string new_name) {
    auto it = commands[old_state].find(old_name);
    if (it == commands[old_state].end())
        throw std::runtime_error("Unable to add alias for " + old_name);
    commands[new_state].emplace(new_name, it->second);
}

void Shell::configure_commands() {
    configure_commands_global();
    configure_commands_environment();
    configure_commands_task();
}

void Shell::parse_settings(YAMLParser::Mapping &config) {
    auto deserialize_compilers = [&](std::unordered_map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        if (map.has_key("compilers")) {
            std::map <std::string, YAMLParser::Value> compilers = map.get_value("compilers").get_mapping().get_map();
            for (auto &compiler_data : compilers) {
                settings.emplace("compiler_" + compiler_data.first, compiler_data.second.get_string());
            }
        }

    };
    std::vector <YAMLParser::Value> environments = config.get_value("environments").get_sequence();
    for (auto &env_data : environments) {
        YAMLParser::Mapping map = env_data.get_mapping();
        Environment env(map.get_value("name").get_string());
        std::cout << "env: " << map.get_value("name").get_string() << std::endl;
        if (map.has_key("tasks")) {
            std::vector <YAMLParser::Value> tasks = map.get_value("tasks").get_sequence();
            for (auto &task_data : tasks) {
                YAMLParser::Mapping map = task_data.get_mapping();
                Task task(map.get_value("name").get_string());
                deserialize_compilers(task.get_settings(), map);
                for (auto &setting : map.get_map()) {
                    if (setting.first != "name" &&
                        setting.first != "tasks" &&
                        setting.first != "compilers") {
                        task.add_setting(setting.first, setting.second.get_string());
                    }
                }
                env.add_task(task);
            }
        }
        deserialize_compilers(env.get_settings(), map);
        for (auto &setting : map.get_map()) {
            if (setting.first != "name" &&
                setting.first != "tasks" &&
                setting.first != "compilers") {
                env.add_setting(setting.first, setting.second.get_string());
            }
        }
        envs.push_back(env);
    }
    YAMLParser::Mapping global_settings_map = config.get_value("global").get_mapping();
    auto compilers_settings = global_settings_map.get_value("compilers").get_mapping().get_map();
    for (auto &cs : compilers_settings) {
        global_settings.emplace("compiler_" + cs.first, cs.second.get_string());
    }
    for (auto &setting : global_settings_map.get_map()) {
        if (setting.first != "name" &&
            setting.first != "tasks" &&
            setting.first != "compilers") {
            global_settings.emplace(setting.first, setting.second.get_string());
        }
    }
}

void Shell::create_paths() {
    fs::path path = fs::current_path();
    for (auto &env : envs) {
        fs::path env_path = path / ("env_" + env.get_name());
        if (!fs::exists(env_path)) {
            fs::create_directory(env_path);
        }
        for (auto &task : env.get_tasks()) {
            fs::path task_path = env_path / ("task_" + task.get_name());
            if (!fs::exists(task_path)) {
                fs::create_directory(task_path);
            }
            if (!fs::exists(task_path / ("main." + task.get_settings()["language"]))) {
                std::ofstream f(task_path / ("main." + task.get_settings()["language"]), std::ios::out);
                f.close();
            }
        }
    }
}

void Shell::run() {
    std::string command;
    std::vector <std::string> args;
    while (true) {
        std::cout << ">";
        if (current_env != -1) {
            std::cout << "/" << envs[current_env].get_name();
        }
        if (current_task != -1) {
            std::cout << "/" << envs[current_env].get_tasks()[current_task].get_name();
        }
        std::cout << " ";
        std::flush(std::cout);
        std::getline(std::cin, command);
        split(args, command);
        std::cout << "DBG: ";
        for (auto &el : args)
            std::cout << el << " ";
        std::cout << std::endl;
        if (args.size() == 0)
            continue;
        if (commands[current_state].find(args[0]) == commands[current_state].end()) {
            std::cout << "Unknown command " << args[0] << std::endl;
        } else {
            try {
                int verdict = commands[current_state][args[0]](args);
                if (verdict) {
                    std::cout << "Command " << args[0] << " returned " << verdict << std::endl;
                }
            } catch(std::runtime_error &re) {
                std::cout << "Error: " << re.what() << std::endl;
            }
        }
    }
}

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
    add_command(State::GLOBAL, "q", [](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::cout << "Exiting..." << std::endl;
        exit(0);
        return 0;
    });
    add_alias(State::GLOBAL, "q", State::GLOBAL, "exit");
}

Shell::~Shell() {
    
}
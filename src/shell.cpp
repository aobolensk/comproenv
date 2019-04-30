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
        YAMLParser::Mapping p = p1.parse().getMapping();
        parse_settings(p);
        create_paths();
    }
    current_env = -1;
    current_task = -1;
    current_state = State::GLOBAL;
    current_compiler = "cpp";
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

    // Set environment
    add_command(State::GLOBAL, "se", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs_.size(); ++i) {
            if (envs_[i].get_name() == arg[1]) {
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
        for (size_t i = 0; i < envs_.size(); ++i)
            if (envs_[i].get_name() == arg[1])
                throw std::runtime_error("Environment named " + arg[1] + " already exists");
        envs_.push_back(arg[1]);
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
        for (size_t i = 0; i < envs_.size(); ++i) {
            if (envs_[i].get_name() == arg[1]) {
                envs_.erase(envs_.begin() + i);
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
        f << "environments:" << std::endl;
        indent += 2;
        for (auto &env : envs_) {
            for (int i = 0; i < indent; ++i)
                f << " ";
            f << "- name: " << env.get_name() << std::endl;
            indent += 2;
            for (int i = 0; i < indent; ++i)
                f << " ";
            f << "tasks:" << std::endl;
            indent += 2;
            for (auto &task : env.get_tasks()) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << "- name: " << task.get_name() << std::endl;
            }
            indent -= 2;
            indent -= 2;
        }
        indent -= 2;
        f << std::endl;
        f << "global:" << std::endl;
        indent += 2;
        for (int i = 0; i < indent; ++i)
            f << " ";
        f << "compilers:" << std::endl;
        indent += 2;
        for (auto &setting : global_settings) {
            if (setting.first.compare(0, std::size("compiler_") - 1, "compiler_") == 0) {
                for (int i = 0; i < indent; ++i)
                    f << " ";
                f << setting.first.substr(std::size("compiler_") - 1) << ": " << setting.second << std::endl;
            }
        }
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

    // Set task
    add_command(State::ENVIRONMENT, "st", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs_[current_env].get_tasks().size(); ++i) {
            if (envs_[current_env].get_tasks()[i].get_name() == arg[1]) {
                current_task = i;
                current_state = State::TASK;
                return 0;
            }
        }
        throw std::runtime_error("Incorrect task name");
    });

    // Create task
    add_command(State::ENVIRONMENT, "ct", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs_[current_env].get_tasks().size(); ++i)
            if (envs_[current_env].get_tasks()[i].get_name() == arg[1])
                throw std::runtime_error("Task named " + arg[1] + " already exists");
        envs_[current_env].get_tasks().push_back(arg[1]);
        fs::path path = fs::current_path() / ("env_" + envs_[current_env].get_name()) / ("task_" + arg[1]);
        if (!fs::exists(path)) {
            fs::create_directory(path);
        }
        return 0;
    });

    // Delete task
    add_command(State::ENVIRONMENT, "dt", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (size_t i = 0; i < envs_[current_env].get_tasks().size(); ++i) {
            if (envs_[current_env].get_tasks()[i].get_name() == arg[1]) {
                envs_[current_env].get_tasks().erase(envs_[current_env].get_tasks().begin() + i);
                fs::path path = fs::current_path() / ("env_" + envs_[current_env].get_name()) / ("task_" + arg[1]);
                if (fs::exists(path)) {
                    return !fs::remove_all(path);
                }
            }
        }
        throw std::runtime_error("Incorrect task name");
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

    // Set compiler
    add_command(State::TASK, "sc", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 2)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        for (auto &setting : global_settings) {
            if (setting.first == "compiler_" + arg[1]) {
                current_compiler = arg[1];
                return 0;
            }
        }
        throw std::runtime_error("Incorrect compiler name");
    });

    // Compile task
    add_command(State::TASK, "c", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::string command = global_settings["compiler_" + current_compiler];
        size_t pos = std::string::npos;
        while ((pos = command.find("@name@")) != std::string::npos) {
            command.replace(pos, std::size("@name@") - 1,
                            fs::current_path() / ("env_" + envs_[current_env].get_name()) / 
                            ("task_" + envs_[current_env].get_tasks()[current_task].get_name()) /
                            "main");
        }
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

void Shell::parse_settings(YAMLParser::Mapping &config) {
    std::vector <YAMLParser::Value> environments = config.getValue("environments").getSequence();
    for (auto &env_data : environments) {
        YAMLParser::Mapping map = env_data.getMapping();
        Environment env(map.getValue("name").getString());
        std::cout << "env: " << map.getValue("name").getString() << std::endl;
        std::vector <YAMLParser::Value> tasks = map.getValue("tasks").getSequence();
        for (auto &task_data : tasks) {
            YAMLParser::Mapping map = task_data.getMapping();
            Task task(map.getValue("name").getString());
            env.add_task(task);
        }
        envs_.push_back(env);
    }
    YAMLParser::Mapping global_settings_map = config.getValue("global").getMapping();
    auto compilers_settings = global_settings_map.getValue("compilers").getMapping().getMap();
    for (auto &cs : compilers_settings) {
        global_settings.emplace("compiler_" + cs.first, cs.second.getString());
    }
}

void Shell::create_paths() {
    fs::path path = fs::current_path();
    for (auto &env : envs_) {
        fs::path env_path = path / ("env_" + env.get_name());
        if (!fs::exists(env_path)) {
            fs::create_directory(env_path);
        }
        for (auto &task : env.get_tasks()) {
            fs::path task_path = env_path / ("task_" + task.get_name());
            if (!fs::exists(task_path)) {
                fs::create_directory(task_path);
            }
            if (!fs::exists(task_path / "main.cpp")) {
                std::ofstream f(task_path / "main.cpp", std::ios::out);
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
            std::cout << "/" << envs_[current_env].get_name();
        }
        if (current_task != -1) {
            std::cout << "/" << envs_[current_env].get_tasks()[current_task].get_name();
        }
        std::cout << " ";
        std::flush(std::cout);
        std::getline(std::cin, command);
        split(args, command);
        std::cout << "DBG: ";
        for (auto &el : args)
            std::cout << el << " ";
        std::cout << std::endl;
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

Shell::~Shell() {
    
}
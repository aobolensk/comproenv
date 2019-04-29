#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include <stdexcept>
#include "environment.h"
#include "yaml_parser.h"
#include "shell.h"
#include "utils.h"

namespace fs = std::experimental::filesystem;

Shell::Shell(const std::string &config_file) {
    configure_commands();
    if (config_file != "") {
        YAMLParser p1(config_file);
        YAMLParser::Mapping p = p1.parse().getMapping();
        parse_settings(p);
        create_paths();
    }
    current_env = -1;
    current_task = -1;
}

void Shell::configure_commands() {
    commands[State::GLOBAL].emplace("se", [this](std::vector <std::string> &arg) -> int {
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

    commands[State::ENVIRONMENT].emplace("q", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        current_env = -1;
        current_state = State::GLOBAL;
        return 0;
    });

    commands[State::GLOBAL].emplace("q", [this](std::vector <std::string> &arg) -> int {
        if (arg.size() != 1)
            throw std::runtime_error("Incorrect arguments for command " + arg[0]);
        std::cout << "Exiting..." << std::endl;
        exit(0);
        return 0;
    });
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
            std::cout << "/" << envs_[current_task].get_name();
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
                    std::cout << "Command " << args[0] << "returned " << verdict << std::endl;
                }
            } catch(std::runtime_error &re) {
                std::cout << "Error: " << re.what() << std::endl;
            }
        }
    }
}

Shell::~Shell() {
    
}
#include <fstream>
#include <vector>
#include <experimental/filesystem>
#include "environment.h"
#include "yaml_parser.h"
#include "shell.h"

namespace fs = std::experimental::filesystem;

Shell::Shell(const std::string &config_file) {
    if (config_file != "") {
        YAMLParser p1(config_file);
        YAMLParser::Mapping p = p1.parse().getMapping();
        parse_settings(p);
        create_paths();
    }
}

void Shell::parse_settings(YAMLParser::Mapping &config) {
    std::vector <YAMLParser::Value> environments = config.getValue("environments").getSequence();
    for (auto &env_data : environments) {
        YAMLParser::Mapping map = env_data.getMapping();
        Environment env(map.getValue("name").getString());
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
    
}

Shell::~Shell() {
    
}
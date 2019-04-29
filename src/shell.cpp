#include <vector>
#include "environment.h"
#include "yaml_parser.h"
#include "shell.h"

Shell::Shell(const std::string &config_file) {
    if (config_file != "") {
        YAMLParser p1(config_file);
        YAMLParser::Mapping p = p1.parse().getMapping();
        parse_settings(p);
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
    YAMLParser::Mapping glob_settings = config.getValue("global").getMapping();
}

void Shell::run() {
    
}

Shell::~Shell() {
    
}
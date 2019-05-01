#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include "environment.h"
#include "yaml_parser.h"

class Shell {
 private:
    enum State {
        GLOBAL, ENVIRONMENT, TASK, INVALID
    };
    std::array <std::unordered_map <std::string, std::function<int(std::vector <std::string> &)>>, (size_t)State::INVALID> commands;
    int current_env, current_task, current_state;
    std::string current_compiler;
    std::vector <Environment> envs;
    std::unordered_map <std::string, std::string> global_settings;
    std::string config_file;
    void parse_settings(YAMLParser::Mapping &val);
    void create_paths();
    void configure_commands();
    void add_command(int state, std::string name, std::function<int(std::vector <std::string> &)> func);
    void add_alias(int old_state, std::string new_name, int new_state, std::string old_name);
 public:
    Shell(const std::string &file = "");
    void run();
    ~Shell();
};

#endif  // INCLUDE_SHELL_H

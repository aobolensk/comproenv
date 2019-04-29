#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H
#include <vector>
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
    std::vector <Environment> envs_;
    std::unordered_map <std::string, std::string> global_settings;
    void parse_settings(YAMLParser::Mapping &val);
    void create_paths();
    void configure_commands();
 public:
    Shell(const std::string &config_file = "");
    void run();
    ~Shell();
};

#endif  // INCLUDE_SHELL_H

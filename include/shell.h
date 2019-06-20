#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H
#include <vector>
#include <set>
#include <array>
#include <unordered_map>
#include <functional>
#include "environment.h"
#include "yaml_parser.h"

class Shell {
 private:
    enum State {
        GLOBAL, ENVIRONMENT, TASK, GENERATOR, INVALID
    };
    std::array <std::unordered_map <std::string, std::function<int(std::vector <std::string> &)>>, (size_t)State::INVALID> commands;
    std::array <std::map <std::string, std::set<std::string>>, (size_t)State::INVALID> help;
    int current_env, current_task, current_state;
    std::vector <Environment> envs;
    std::unordered_map <std::string, std::string> global_settings;
    std::string config_file;
    std::string environments_file;
    void parse_settings(YAMLParser::Mapping &config, YAMLParser::Mapping &environments);
    void create_paths();
    void configure_commands();
    void configure_commands_global();
    void configure_commands_environment();
    void configure_commands_task();
    void configure_commands_generator();
    void add_command(int state, std::string name,
                    std::string help_info,
                    std::function<int(std::vector <std::string> &)> func);
    void add_alias(int old_state, std::string new_name, int new_state, std::string old_name);
    std::string get_setting_by_name(const std::string name);
 public:
    Shell(const std::string_view config_file_path = "", const std::string_view environments_file_path = "");
    void run();
    ~Shell();
};

#endif  // INCLUDE_SHELL_H

#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H
#include <vector>
#include <set>
#include <array>
#include <map>
#include <functional>
#include "utils.h"
#include "environment.h"
#include "task.h"
#include "yaml_parser.h"

namespace comproenv {

const static std::string env_prefix = "env_";
const static std::string task_prefix = "task_";
#define MAX_HISTORY_SIZE 32

class Shell {
 public:
    #define STATES /* List of states: */ \
        X(GLOBAL) X(ENVIRONMENT) X(TASK) X(GENERATOR)
    #define X(state) state,
    enum State {
        STATES
        INVALID // Impossible state, created in order to get enum size
    };
    #undef X
    #define X(state) TOSTRING(state),
    const std::array <std::string, (size_t)State::INVALID> state_names = { STATES };
    #undef X
 private:
    std::array <std::map <std::string, std::function<int(std::vector <std::string> &)>>, (size_t)State::INVALID> commands;
    std::array <std::map <std::string, std::set<std::string>>, (size_t)State::INVALID> help;
    int current_env, current_task, current_state;
    std::vector <Environment> envs;
    std::map <std::string, std::string> global_settings;
    std::string config_file;
    std::string environments_file;
    struct CommandsHistory {
    private:
        std::array <std::string, MAX_HISTORY_SIZE> buf;
        int start, end;
    public:
        CommandsHistory();
        ~CommandsHistory() = default;
        void push(const std::string &com);
        std::vector <std::string> get_all();
    } commands_history;
    void parse_settings(YAMLParser::Mapping &config, YAMLParser::Mapping &environments);
    void create_paths();
    void configure_commands();
    void configure_commands_global();
    void configure_commands_environment();
    void configure_commands_task();
    void configure_commands_generator();
    void configure_user_defined_aliases();
    void add_command(int state, std::string name,
                    std::string help_info,
                    std::function<int(std::vector <std::string> &)> func);
    void add_alias(int old_state, std::string new_name, int new_state, std::string old_name);
    std::string get_setting_by_name(const std::string name);
 public:
    Shell(const std::string_view config_file_path = "", const std::string_view environments_file_path = "");
    void run();
    std::string get_help(State state);
    ~Shell();
};

}  // namespace comproenv

#endif  // INCLUDE_SHELL_H

#include <fstream>
#ifndef EXP_FS
#include <filesystem>
#else
#include <experimental/filesystem>
#endif  // EXP_FS
#include <csignal>
#ifdef _WIN32
#include <Windows.h>
#endif  // _WIN32
#include "shell.h"

namespace comproenv {

#ifndef EXP_FS
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif  // EXP_FS

#ifndef _WIN32
static void sigint_handler(int sig_num) {}
#endif  // _WIN32

Shell::Shell(const std::string_view config_file_path,
             const std::string_view environments_file_path) :
             config_file(config_file_path),
             environments_file(environments_file_path) {
    #ifndef _WIN32
    signal(SIGINT, &sigint_handler);
    #else
    signal(SIGINT, SIG_IGN);
    #endif  // _WIN32
    #ifndef _WIN32
    signal(SIGTSTP, SIG_IGN);
    #endif  // _WIN32
    YAMLParser::Mapping config, environments;
    configure_commands();
    if (config_file == "")
        config_file = "config.yaml";
    if (fs::exists(config_file)) {
        YAMLParser p1(config_file);
        config = p1.parse().get_mapping();
    } else {
        std::cout << "Configuration file (" << config_file << ") does not exist. Creating new one." << std::endl;
    }
    if (environments_file == "")
        environments_file = "environments.yaml";
    if (fs::exists(environments_file)) {
        YAMLParser p1(environments_file);
        try {
            environments = p1.parse().get_mapping();
        } catch (std::runtime_error &) {
            std::cout << "No environments found" << std::endl;
        }
    } else {
        std::cout << "Environments file (" << environments_file << ") does not exist. Creating new one." << std::endl;
    }
    parse_settings(config, environments);
    configure_user_defined_aliases();
    auto it = global_settings.find("python_interpreter");
    if (it != global_settings.end()) {
        std::string command = it->second + " --version 2>&1";
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
        if ((s_result[0] == "Python") && (s_result[1][0] - '0' >= 3)) {
            std::cout << "Found: " + result << std::endl;
        } else if ((s_result[0] == "Python") && (s_result[1][0] - '0' < 3)) {
            std::cout << "Warning: Python with version that less than 3 is not supported, "
                "so some features like parsing websites may be unavailable" << std::endl;
        } else {
            std::cout << "Warning: Python interpreter is not found!" << std::endl;
        }
    }
    if (global_settings.find("autosave") == global_settings.end()) {
        global_settings.emplace("autosave", "on");
    }
    create_paths();
    current_env = -1;
    current_task = -1;
    current_state = State::GLOBAL;
}

void Shell::add_command(int state, std::string name,
                        std::string help_info,
                        std::function<int(std::vector <std::string> &)> func) {
    commands[state].emplace(name, func);
    help[state][help_info].insert(name);
}

void Shell::add_alias(int old_state, std::string old_name, int new_state, std::string new_name) {
    auto old_command = commands[old_state].find(old_name);
    if (old_command == commands[old_state].end()) {
        std::cout << "Unable to add alias for " + old_name << '\n';
        return;
    }
    commands[new_state].emplace(new_name, old_command->second);
    for (auto &help_info : help[old_state]) {
        if (help_info.second.find(old_name) != help_info.second.end()) {
            help[new_state][help_info.first].insert(new_name);
            break;
        }
    }
}

std::optional <std::string> Shell::get_setting_by_name(const std::string name) {
    if (current_task != -1) {
        auto it = envs[current_env].get_tasks()[current_task].get_settings().find(name);
        if (it != envs[current_env].get_tasks()[current_task].get_settings().end())
            return it->second;
    }
    if (current_env != -1) {
        auto it = envs[current_env].get_settings().find(name);
        if (it != envs[current_env].get_settings().end())
            return it->second;
    }
    auto it = global_settings.find(name);
    if (it != global_settings.end()) {
        return it->second;
    } else {
        return {};
    }
}

void Shell::configure_commands() {
    configure_commands_global();
    configure_commands_environment();
    configure_commands_task();
    configure_commands_generator();
}

void Shell::parse_settings(YAMLParser::Mapping &config, YAMLParser::Mapping &environments) {
    DEBUG_LOG("Settings parsing");
    auto deserialize_compilers = [&](std::map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        if (map.has_key("compilers")) {
            std::map <std::string, YAMLParser::Value> compilers = map.get_value("compilers").get_mapping().get_map();
            for (auto &compiler_data : compilers) {
                settings.emplace("compiler_" + compiler_data.first, compiler_data.second.get_string());
                DEBUG_LOG("compiler_" + compiler_data.first + ": " + compiler_data.second.get_string());
            }
        }
    };
    auto deserialize_runners = [&](std::map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        if (map.has_key("runners")) {
            std::map <std::string, YAMLParser::Value> runners = map.get_value("runners").get_mapping().get_map();
            for (auto &runner_data : runners) {
                settings.emplace("runner_" + runner_data.first, runner_data.second.get_string());
                DEBUG_LOG("runner_" + runner_data.first + ": " + runner_data.second.get_string());
            }
        }
    };
    auto deserialize_templates = [&](std::map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        if (map.has_key("templates")) {
            std::map <std::string, YAMLParser::Value> templates = map.get_value("templates").get_mapping().get_map();
            for (auto &template_data : templates) {
                settings.emplace("template_" + template_data.first, template_data.second.get_string());
                DEBUG_LOG("template_" + template_data.first + ": " + template_data.second.get_string());
            }
        }
    };
    auto deserialize_aliases = [&](std::map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        if (map.has_key("aliases")) {
            std::map <std::string, YAMLParser::Value> aliases = map.get_value("aliases").get_mapping().get_map();
            for (auto &alias_data : aliases) {
                settings.emplace("alias_" + alias_data.first, alias_data.second.get_string());
                DEBUG_LOG("alias_" + alias_data.first + ": " + alias_data.second.get_string());
            }
        }
    };
    auto deserialize_commands_history = [&](YAMLParser::Mapping &map) {
        if (map.has_key("commands_history")) {
            std::vector <YAMLParser::Value> history = map.get_value("commands_history").get_sequence();
            for (const auto &command : history) {
                commands_history.push(command.get_string());
            }
        }
    };
    auto deserialize_rest_settings = [&](std::map <std::string, std::string> &settings, YAMLParser::Mapping &map) {
        for (auto &setting : map.get_map()) {
            if (setting.first != "name" &&
                setting.first != "tasks" &&
                setting.first != "compilers" &&
                setting.first != "runners" &&
                setting.first != "templates" &&
                setting.first != "aliases" &&
                setting.first != "commands_history") {
                settings.emplace(setting.first, setting.second.get_string());
                DEBUG_LOG(setting.first + ": " + setting.second.get_string());
            }
        }
    };

    if (environments.has_key("environments")) {
        std::vector <YAMLParser::Value> environments_content = environments.get_value("environments").get_sequence();
        for (auto &env_data : environments_content) {
            YAMLParser::Mapping map = env_data.get_mapping();
            Environment env(map.get_value("name").get_string());
            DEBUG_LOG("env: " + map.get_value("name").get_string());
            if (map.has_key("tasks")) {
                std::vector <YAMLParser::Value> tasks = map.get_value("tasks").get_sequence();
                for (auto &task_data : tasks) {
                    YAMLParser::Mapping task_map = task_data.get_mapping();
                    Task task(task_map.get_value("name").get_string());
                    deserialize_compilers(task.get_settings(), task_map);
                    deserialize_runners(task.get_settings(), task_map);
                    deserialize_templates(task.get_settings(), task_map);
                    deserialize_rest_settings(task.get_settings(), task_map);
                    env.add_task(task);
                }
            }
            deserialize_compilers(env.get_settings(), map);
            deserialize_runners(env.get_settings(), map);
            deserialize_templates(env.get_settings(), map);
            deserialize_rest_settings(env.get_settings(), map);
            envs.push_back(env);
        }
    }

    if (config.has_key("global")) {
        YAMLParser::Mapping global_settings_map = config.get_value("global").get_mapping();
        deserialize_compilers(global_settings, global_settings_map);
        deserialize_runners(global_settings, global_settings_map);
        deserialize_templates(global_settings, global_settings_map);
        deserialize_aliases(global_settings, global_settings_map);
        deserialize_rest_settings(global_settings, global_settings_map);
    }

    if (global_settings.find("python_interpreter") == global_settings.end()) {
        global_settings.emplace("python_interpreter", "python");
    }
    if (global_settings.find("max_history_size") == global_settings.end()) {
        global_settings.emplace("max_history_size", "32");
    }
    if (global_settings.find("max_lines_count") == global_settings.end()) {
        global_settings.emplace("max_lines_count", "100");
    }

    int max_history_size = std::stoi(global_settings.find("max_history_size")->second);
    if (max_history_size == 0) {
        max_history_size = 32;
    }
    commands_history = CommandsHistory(max_history_size);

    if (config.has_key("global")) {
        YAMLParser::Mapping global_settings_map = config.get_value("global").get_mapping();
        deserialize_commands_history(global_settings_map);
    }
}

void Shell::create_paths() {
    for (auto &env : envs) {
        fs::path env_path = fs::path(env_prefix + env.get_name());
        if (!fs::exists(env_path)) {
            fs::create_directory(env_path);
        }
        for (auto &task : env.get_tasks()) {
            fs::path task_path = env_path / (task_prefix + task.get_name());
            if (!fs::exists(task_path)) {
                fs::create_directory(task_path);
            }
            if (!fs::exists(task_path / (task.get_name() + "." + task.get_settings()["language"]))) {
                std::ofstream f(task_path / (task.get_name() + "." + task.get_settings()["language"]), std::ios::out);
                f.close();
            }
            if (!fs::exists(task_path / "tests")) {
                fs::create_directory(task_path / "tests");
            }
        }
    }
}

int Shell::store_cache() {
    if (!fs::is_regular_file(std::string(cache_file))) {
        std::cout << "Can not find cache file" << std::endl;
        return -1;
    }
    std::ofstream f(std::string(cache_file), std::ios::trunc);
    if (!f.is_open()) {
        std::cout << "Can not open cache file" << std::endl;
        return -2;
    }
    f << current_env << " " << current_task << " " << current_state << std::endl;
    f.close();
    return 0;
}

int Shell::read_cache() {
    if (!fs::is_regular_file(std::string(cache_file))) {
        std::cout << "Can not find cache file" << std::endl;
        return -1;
    }
    std::ifstream f(std::string(cache_file), std::ios::in);
    if (!f.is_open()) {
        std::cout << "Can not open cache file" << std::endl;
        return -2;
    }
    f >> current_env >> current_task >> current_state;
    if ((current_env < -1 || current_env > (int)envs.size()) ||                                 // Wrong environment index
        (current_task < -1 ||
        (current_env != -1 && current_task > (int)envs[current_env].get_tasks().size())) ||     // Wrong task index
        (current_state < 0 || current_state >= (int)State::INVALID)                             // Wrong state index
        ) {
        std::cout << "Unable to restore previous state from cache" << std::endl;
        current_env = current_task = -1;
        current_state = 0;
    } else {
        std::cout << "Successfully restored previous state from cache!" << std::endl;
    }
    f.close();
    return 0;
}

Shell::CommandsHistory::CommandsHistory(int buf_size) {
    start = end = 0;
    this->buf_size = buf_size;
    buf.resize(buf_size);
}

void Shell::CommandsHistory::push(const std::string &com) {
    buf[end] = com;
    if (start == (end + 1) % buf_size)
        start = (start + 1) % buf_size;
    end = (end + 1) % buf_size;
}

std::vector <std::string> Shell::CommandsHistory::get_all() {
    std::vector <std::string> result;
    for (int i = start; i != end; i = (i + 1) % buf_size)
        result.push_back(buf[i]);
    return result;
}

void Shell::set_console_title() {
    #ifdef _WIN32
    std::string title = application_name;
    if (current_env != -1) {
        title += " -> " + envs[current_env].get_name();
        if (current_task != -1) {
            title += "/" + envs[current_env].get_tasks()[current_task].get_name();
            if (current_state == State::GENERATOR) {
                title += "/generator";
            }
        }
    }
    SetConsoleTitle(title.c_str());
    #endif  // _WIN32
}

void Shell::run() {
    #ifndef EXP_FS
    DEBUG_LOG("Built with std::filesystem");
    #else
    DEBUG_LOG("Built with std::experimental::filesystem");
    #endif  // EXP_FS
    DEBUG_LOG("Launching shell: " << FUNC);
    std::string command;
    std::vector <std::string> args;
    DEBUG_LOG("Debug log is enabled");
    std::ofstream f;
    cache_file = (fs::current_path() / cache_file_name).string();
    if (fs::exists(cache_file)) {
        read_cache();
    } else {
        f.open(cache_file, std::ios::out | std::ios::app);
        if (!f.is_open()) {
            std::cout << cache_file << std::endl;
            std::cout << "Unable to open cache file" << std::endl;
        } else {
            f.close();
        }
    }
    store_cache();
    set_console_title();
    while (true) {
        std::cout << ">";
        if (current_env != -1) {
            std::cout << "/" << envs[current_env].get_name();
        }
        if (current_task != -1) {
            std::cout << "/" << envs[current_env].get_tasks()[current_task].get_name();
        }
        if (current_state == State::GENERATOR) {
            std::cout << "/gen";
        }
        std::cout << " ";
        std::flush(std::cout);
        while (std::cin.eof()) {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        std::getline(std::cin, command);
        DEBUG_LOG(command);
        split(args, command);
        if (args.size() == 0)
            continue;
        if (commands[current_state].find(args[0]) == commands[current_state].end()) {
            std::cout << "Unknown command " << args[0] << '\n';
        } else {
            try {
                int verdict = commands[current_state][args[0]](args);
                if (verdict) {
                    std::cout << "Command " << args[0] << " returned " << verdict << '\n';
                }
            } catch(std::runtime_error &re) {
                std::cout << "Error: " << re.what() << '\n';
            }
            commands_history.push(join(" ", args));
        }
        std::cout << std::flush;
    }
}

void Shell::configure_user_defined_aliases() {
    for (int state = 0; state < State::INVALID; ++state) {
        auto it = global_settings.find("alias_" + state_names[state]);
        if (it == global_settings.end())
            continue;
        std::vector <std::string> aliases;
        split(aliases, it->second);
        for (size_t i = 0; i < aliases.size(); i += 2) {
            add_alias(state, aliases[i], state, aliases[i + 1]);
        }
    }
}

std::string Shell::get_help(Shell::State state) {
    std::string result;
    for (auto &help_info : help[state]) {
        bool flag = false;
        result += "| ";
        for (const std::string &command : help_info.second) {
            if (flag)
               result += ", ";
            else
                flag = true;
           result += command;
        }
        result += " | ";
        for (size_t i = 0; i < help_info.first.size(); ++i) {
            if (help_info.first[i] != '\n')
                result.push_back(help_info.first[i]);
            else
                result.push_back(' ');
        }
        result += " |\n";
    }
    return result;
}

Shell::~Shell() {
    
}

}  // namespace comproenv

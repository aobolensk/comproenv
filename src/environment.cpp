#include "environment.h"

namespace comproenv {

Environment::Environment(const std::string &env_name) : name(env_name) {

}

void Environment::add_task(const Task &task) {
    tasks.emplace_back(task);
}

void Environment::add_setting(const std::string &key, const std::string &value) {
    settings.emplace(key, value);
}

std::string Environment::get_name() const {
    return name;
}

std::vector <Task> &Environment::get_tasks() {
    return tasks;
}


std::map <std::string, std::string> &Environment::get_settings() {
    return settings;
}

}  // namespace comproenv

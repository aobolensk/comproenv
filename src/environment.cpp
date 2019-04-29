#include "environment.h"

Environment::Environment(const std::string &env_name) : name(env_name) {

}

void Environment::add_task(const Task &task) {
    tasks_.emplace_back(task);
}

void Environment::add_setting(const std::string &key, const std::string &value) {
    settings_.emplace(key, value);
}
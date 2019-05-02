#include <string>
#include "task.h"

Task::Task(const std::string &task_name) : name(task_name) {

}

std::string Task::get_name() const {
    return name;
}

void Task::add_setting(const std::string &key, const std::string &value) {
    settings.emplace(key, value);
}

std::unordered_map <std::string, std::string> &Task::get_settings() {
    return settings;
}
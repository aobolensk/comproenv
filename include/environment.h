#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H
#include <vector>
#include <map>
#include "task.h"

namespace comproenv {

class Environment {
 private:
    std::vector <Task> tasks;
    std::map <std::string, std::string> settings;
    std::string name;
 public:
    Environment(const std::string &env_name);
    void add_task(const Task &task);
    std::string get_name() const;
    void set_name(std::string_view new_name);
    std::vector <Task> &get_tasks();
    std::map <std::string, std::string> &get_settings();
    void add_setting(const std::string &key, const std::string &value);
};

}  // namespace comproenv

#endif  // INCLUDE_ENVIRONMENT_H

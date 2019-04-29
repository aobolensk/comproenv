#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H
#include <vector>
#include <unordered_map>
#include "task.h"

class Environment {
 private:
    std::vector <Task> tasks_;
    std::unordered_map <std::string, std::string> settings_;
    std::string name;
 public:
    Environment(const std::string &env_name);
    void add_task(const Task &task);
    std::string get_name() const;
    const std::vector <Task> get_tasks() const;
    void add_setting(const std::string &key, const std::string &value);
};

#endif  // INCLUDE_ENVIRONMENT_H

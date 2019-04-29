#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H
#include <string>
#include <unordered_map>

class Task {
 private:
    std::unordered_map <std::string, std::string> settings_;
    std::string name;
 public:
    Task(const std::string &task_name);
};

#endif  // INCLUDE_TASK_H

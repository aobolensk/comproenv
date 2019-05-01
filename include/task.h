#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H
#include <string>
#include <unordered_map>

class Task {
 private:
    std::unordered_map <std::string, std::string> settings;
    std::string name;
 public:
    Task(const std::string &task_name);
    std::string get_name() const;
};

#endif  // INCLUDE_TASK_H

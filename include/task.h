#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H
#include <string>
#include <unordered_map>

namespace comproenv {

class Task {
 private:
    std::unordered_map <std::string, std::string> settings;
    std::string name;
 public:
    Task(const std::string &task_name);
    void add_setting(const std::string &key, const std::string &value);
    std::unordered_map <std::string, std::string> &get_settings();
    std::string get_name() const;
};

}  // namespace comproenv

#endif  // INCLUDE_TASK_H

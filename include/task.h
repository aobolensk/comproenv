#ifndef INCLUDE_TASK_H
#define INCLUDE_TASK_H
#include <string>
#include <map>

namespace comproenv {

class Task {
 private:
    std::map <std::string, std::string> settings;
    std::string name;
 public:
    Task(const std::string_view task_name);
    void add_setting(const std::string_view key, const std::string_view value);
    std::map <std::string, std::string> &get_settings();
    std::string get_name() const;
    void set_name(std::string_view new_name);
};

}  // namespace comproenv

#endif  // INCLUDE_TASK_H

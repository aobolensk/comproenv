#include <string>
#include "task.h"

Task::Task(const std::string &task_name) : name(task_name) {

}

std::string Task::get_name() const {
    return name;
}

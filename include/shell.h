#ifndef INCLUDE_SHELL_H
#define INCLUDE_SHELL_H
#include <vector>
#include "environment.h"
#include "yaml_parser.h"

class Shell {
 private:
    std::vector <Environment> envs_;
    void parse_settings(YAMLParser::Mapping &val);
    void create_paths();
 public:
    Shell(const std::string &config_file = "");
    void run();
    ~Shell();
};

#endif  // INCLUDE_SHELL_H

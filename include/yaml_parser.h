#ifndef INCLUDE_YAML_PARSER_H
#define INCLUDE_YAML_PARSER_H
#include <string>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <map>
#include <vector>
#include "libyaml/include/yaml.h"

class YAMLParser {
 private:
    std::string file_name_;
    std::unique_ptr<FILE, decltype(&fclose)> file_;
    yaml_parser_t parser_;
 public:
    YAMLParser(std::string file_name);
    YAMLParser(const YAMLParser &p);
    ~YAMLParser();
};

#endif  // INCLUDE_YAML_PARSER_H

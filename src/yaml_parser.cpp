#include "yaml_parser.h"
#include <iostream>

YAMLParser::YAMLParser(std::string file_name) :
    file_name_(file_name),
    file_(fopen(file_name.c_str(), "r"), &fclose) {
    if (!file_)
        throw std::runtime_error("YAMLParser: File is not found");
    yaml_parser_initialize(&parser_);
    yaml_parser_set_input_file(&parser_, file_.get());
}

YAMLParser::YAMLParser(const YAMLParser &p) :
    file_name_(p.file_name_),
    file_(fopen(file_name_.c_str(), "r"), &fclose) {
    if (!file_)
        throw std::runtime_error("YAMLParser: File is not found");
    yaml_parser_initialize(&parser_);
    yaml_parser_set_input_file(&parser_, file_.get());
}

YAMLParser::~YAMLParser() {
    yaml_parser_delete(&parser_);
}

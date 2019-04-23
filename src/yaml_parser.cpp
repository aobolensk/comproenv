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

bool YAMLParser::Mapping::hasKey(const std::string &name) const {
    return map_.find(name) != map_.end();
}

const std::string &YAMLParser::Value::getString() const {
    if (type_ != Type::String)
        throw std::runtime_error("Value is not String type");
    return string_;
}

const YAMLParser::Mapping &YAMLParser::Value::getMapping() const {
    if (type_ != Type::Mapping)
        throw std::runtime_error("Value is not Mapping type");
    return mapping_;
}

const YAMLParser::Sequence &YAMLParser::Value::getSequence() const {
    if (type_ != Type::Sequence)
        throw std::runtime_error("Value is not Sequence type");
    return sequence_;
}

YAMLParser::Value YAMLParser::Mapping::getValue(const std::string &name) const {
    auto res = map_.find(name);
    if (res == map_.end())
        throw std::runtime_error("Wrong key");
    return (*res).second;
}

std::string YAMLParser::Mapping::getString(const std::string &name) const {
    auto res = map_.find(name);
    if (res == map_.end())
        throw std::runtime_error("Wrong key");
    return (*res).second.getString();
}

YAMLParser::Mapping YAMLParser::Mapping::getMapping(const std::string &name) const {
    auto res = map_.find(name);
    if (res == map_.end())
        throw std::runtime_error("Wrong key");
    return (*res).second.getMapping();
}

YAMLParser::Sequence YAMLParser::Mapping::getSequence(const std::string &name) const {
    auto res = map_.find(name);
    if (res == map_.end())
        throw std::runtime_error("Wrong key");
    return (*res).second.getSequence();
}

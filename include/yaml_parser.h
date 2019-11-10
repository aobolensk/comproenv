#ifndef INCLUDE_YAML_PARSER_H
#define INCLUDE_YAML_PARSER_H
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <map>
#include <vector>
#include <string_view>
#include "libyaml/include/yaml.h"
#include "utils.h"

namespace comproenv {

class YAMLParser {
 public:
    class Value;
    using Sequence = std::vector <Value>;
    class Mapping {
     private:
        std::map <std::string, Value> map;
     public:
        bool has_key(const std::string_view name) const;
        Value get_value(const std::string_view name) const;
        std::string get_string(const std::string_view name) const;
        Mapping get_mapping(const std::string_view name) const;
        Sequence get_sequence(const std::string_view name) const;
        const std::map <std::string, Value> get_map() const;
        friend class YAMLParser;
    };
    class Value {
     public:
        enum class Type {
            String, Mapping, Sequence
        };
     private:
        Type type;
        std::string string;
        Mapping mapping;
        Sequence sequence;
     public:
        void print(std::ostream &os, int indent);
        Value(const std::string_view str) : type(Type::String), string(str) {}
        Value(const Mapping &map) : type(Type::Mapping), mapping(map) {}
        Value(const Sequence &seq) : type(Type::Sequence), sequence(seq) {}
        const std::string get_string() const;
        const Mapping &get_mapping() const;
        const Sequence &get_sequence() const;
        const Type get_type() const;
    };
 private:
    struct YAMLEvent {
        yaml_event_type_t type;
        std::string value;
        YAMLEvent(const yaml_event_t &event);
    };
    std::string file_name;
    std::unique_ptr<FILE, decltype(&fclose)> file;
    yaml_parser_t parser;
    YAMLEvent get_next_event();
    Mapping read_mapping();
    Sequence read_sequence();
 public:
    YAMLParser(std::string file_name);
    YAMLParser(const YAMLParser &p);
    YAMLParser::Value parse();
    ~YAMLParser();
    friend std::ostream &operator<<(std::ostream &os, YAMLParser::Value &val);
};

}  // namespace comproenv

#endif  // INCLUDE_YAML_PARSER_H

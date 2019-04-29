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

class YAMLParser {
 public:
    class Value;
    using Sequence = std::vector <Value>;
    class Mapping {
     private:
        std::map <std::string, Value> map_;
     public:
        bool hasKey(const std::string_view name) const;
        Value getValue(const std::string_view name) const;
        std::string getString(const std::string_view name) const;
        Mapping getMapping(const std::string_view name) const;
        Sequence getSequence(const std::string_view name) const;
        const std::map <std::string, Value> getMap() const;
        friend class YAMLParser;
    };
    class Value {
     public:
        enum class Type {
            String, Mapping, Sequence
        };
     private:
        Type type_;
        std::string string_;
        Mapping mapping_;
        Sequence sequence_;
     public:
        void print(std::ostream &os, int indent);
        Value(const std::string &str) : type_(Type::String), string_(str) {}
        Value(const Mapping &map) : type_(Type::Mapping), mapping_(map) {}
        Value(const Sequence &seq) : type_(Type::Sequence), sequence_(seq) {}
        const std::string &getString() const;
        const Mapping &getMapping() const;
        const Sequence &getSequence() const;
        const Type getType() const;
    };
 private:
    std::string file_name_;
    std::unique_ptr<FILE, decltype(&fclose)> file_;
    yaml_parser_t parser_;
    struct YAMLEvent {
        yaml_event_type_t type;
        std::string value;
        YAMLEvent(const yaml_event_t &event) :
            type(event.type),
            value(event.type == YAML_SCALAR_EVENT ? (const char *)event.data.scalar.value : "") {}
    };
    YAMLEvent getNextEvent() {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser_, &event))
            throw std::runtime_error("Error parsing YAML at " +
                                     std::to_string(parser_.problem_mark.line) +
                                     std::to_string(parser_.problem_mark.column) +
                                     parser_.problem);
        return YAMLEvent(event);
    }
    Mapping readMapping();
    Sequence readSequence();
 public:
    YAMLParser(std::string file_name);
    YAMLParser(const YAMLParser &p);
    YAMLParser::Value parse();
    ~YAMLParser();
    friend std::ostream &operator<<(std::ostream &os, YAMLParser::Value &val);
};

#endif  // INCLUDE_YAML_PARSER_H

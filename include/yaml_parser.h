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
    class Value;
    using Sequence = std::vector <Value>;
    class Mapping {
     private:
        std::map <std::string, Value> map_;
     public:
        bool hasKey(const std::string &name) const;
        Value getValue(const std::string &name) const;
        std::string getString(const std::string &name) const;
        Mapping getMapping(const std::string &name) const;
        Sequence getSequence(const std::string &name) const;
    };
    class Value {
     private:
        enum class Type {
            String, Mapping, Sequence
        };
        Type type_;
        std::string string_;
        Mapping mapping_;
        Sequence sequence_;
     public:
        Value(std::string &&str) : type_(Type::String), string_(std::move(str)) {}
        Value(Mapping &&map) : type_(Type::Mapping), mapping_(std::move(map)) {}
        Value(Sequence &&seq) : type_(Type::Sequence), sequence_(std::move(seq)) {}
        const std::string &getString() const;
        const Mapping &getMapping() const;
        const Sequence &getSequence() const;
    };
};

#endif  // INCLUDE_YAML_PARSER_H

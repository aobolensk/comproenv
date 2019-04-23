#include "yaml_parser.h"
#include "libyaml/include/yaml.h"

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

YAMLParser::Value YAMLParser::parse() {
    int preparing = 2;
    while (preparing) {
        YAMLEvent event = getNextEvent();
        switch(event.type) {
        case YAML_STREAM_START_EVENT:
            if (preparing != 2)
                throw std::runtime_error("Unexpected YAML_STREAM_START_EVENT");
            preparing = 1;
            break;
        case YAML_DOCUMENT_START_EVENT:
            if (preparing != 1)
                throw std::runtime_error("Unexpected YAML_DOCUMENT_START_EVENT");
            preparing = 0;
            break;
        default:
            throw std::runtime_error("Unexpected event " + std::to_string(event.type));
        }
    }
    YAMLEvent event = getNextEvent();
    switch(event.type) {
    case YAML_SCALAR_EVENT:
        return Value(event.value);
        break;
    case YAML_SEQUENCE_START_EVENT:
        return Value(readSequence());
        break;
    case YAML_MAPPING_START_EVENT:
        return Value(readMapping());
        break;
    default:
        throw std::runtime_error("Unexpected event " + std::to_string(event.type));
    }
}

YAMLParser::Mapping YAMLParser::readMapping() {
    Mapping mapping;
    while (true) {
        std::string key;
        YAMLEvent event = getNextEvent();
        // Reading key
        switch(event.type) {
        case YAML_SCALAR_EVENT:
            key = event.value;
            break;
        case YAML_MAPPING_END_EVENT:
            return mapping;
            break;
        default:
            throw std::runtime_error("Unexpected event " + std::to_string(event.type));
        }
        event = getNextEvent();
        switch(event.type) {
        case YAML_SCALAR_EVENT:
            mapping.map_.emplace(key, event.value);
            break;
        case YAML_SEQUENCE_START_EVENT:
            mapping.map_.emplace(key, readSequence());
            break;
        case YAML_MAPPING_START_EVENT:
            mapping.map_.emplace(key, readMapping());
            break;
        default:
            throw std::runtime_error("Unexpected event " + std::to_string(event.type));
        }
    }
}

YAMLParser::Sequence YAMLParser::readSequence() {
    Sequence seq;
    while (true) {
        YAMLEvent event = getNextEvent();
        switch(event.type) {
        case YAML_SCALAR_EVENT:
            seq.emplace_back(event.value);
            break;
        case YAML_SEQUENCE_START_EVENT:
            seq.emplace_back(Value(readSequence()));
            break;
        case YAML_MAPPING_START_EVENT:
            seq.emplace_back(Value(readMapping()));
            break;
        case YAML_SEQUENCE_END_EVENT:
            return seq;
        default:
            throw std::runtime_error("Unexpected event " + std::to_string(event.type));
        }
    }
}

std::ostream &operator<<(std::ostream &os, YAMLParser::Value &val) {
    val.print(os, 0);
    return os;
}

void YAMLParser::Value::print(std::ostream &os, int indent) {
    switch(type_) {
    case Type::String:
        for (int i = 0; i < indent; ++i)
            os << " ";
        os << string_ << std::endl;
        break;
    case Type::Mapping:
        for (auto &el : mapping_.map_) {
            for (int i = 0; i < indent; ++i)
                os << " ";
            os << el.first << std::endl;
            el.second.print(os, indent + 1);
        }
        break;
    case Type::Sequence:
        for (auto &el : sequence_)
            el.print(os, indent + 1);
        break;
    }
}
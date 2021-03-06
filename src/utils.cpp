#include "utils.h"

namespace comproenv {

void split(std::vector <std::string> &out, const std::string_view str, char delim) {
    std::stringstream ss(str.data());
    std::string tmp;
    out.clear();
    while(std::getline(ss, tmp, delim)) {
        if (tmp.size() != 0) {
            out.push_back(tmp);
        }
    }
}

void replace_all(std::string &str, const std::string_view old_value, const std::string_view new_value) {
    size_t pos = 0;
    while ((pos = str.find(old_value, pos)) != std::string::npos) {
        str.replace(str.begin() + pos, str.begin() + pos + std::size(old_value), new_value);
        pos += std::size(new_value);
    }
}

}  // namespace comproenv

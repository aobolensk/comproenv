#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H
#include <string>
#include <vector>
#include <sstream>

namespace comproenv {

void split(std::vector <std::string> &out, const std::string &str, char delim = ' ');
void replace_all(std::string &str, const std::string_view old_value, const std::string_view new_value);

}  // namespace comproenv

#endif  // INCLUDE_UTILS_H

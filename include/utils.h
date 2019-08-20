#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H
#include <string>
#include <vector>
#include <sstream>

namespace comproenv {

#ifdef COMPROENV_DEBUG
    #define DEBUG_LOG(str) /* Simple debug log */ \
        std::cerr << "DBG: " << str << std::endl;
    #define DEBUG_CLOG(color, str) /* Color should be provided in \033[ format */ \
        std::cerr << color << "DBG: " << str << "\033[0m" << std::endl;
    #define DEBUG_FLLOG(str) /* Debug log with file:line mark */ \
        std::cerr << "DBG at " __FILE__ ":" TOSTRING(__LINE__) ": " << str << std::endl;
    #if defined(_MSC_VER)
        #define FUNC __FUNCSIG__
    #elif defined(__GNUG__) || defined(__clang__)
        #define FUNC __PRETTY_FUNCTION__
    #else
        #define FUNC
    #endif  // _MSC_VER
#else
    #define DEBUG_LOG(str)
    #define DEBUG_CLOG(str)
    #define DEBUG_FLLOG(str)
    #define FUNC(x)
#endif  // COMPROENV_DEBUG

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

void split(std::vector <std::string> &out, const std::string &str, char delim = ' ');
void replace_all(std::string &str, const std::string_view old_value, const std::string_view new_value);

template <typename T>
std::string join(std::string delim, T container) {
    std::string result;
    bool first = true;
    for (const auto &it : container) {
        if (!first) {
            result += delim;
        } else {
            first = false;
        }
        result += it;
    }
    return result;
}

}  // namespace comproenv

#endif  // INCLUDE_UTILS_H

#include "utils.h"

void split(std::vector <std::string> &out, const std::string &str, char delim) {
    std::stringstream ss(str);
    std::string tmp;
    out.clear();
    while(std::getline(ss, tmp, delim))
        out.push_back(tmp);
}

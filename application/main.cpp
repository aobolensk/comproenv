#include <iostream>
#include "yaml_parser.h"

using namespace std;

int main() {
    YAMLParser p1("build/bin/1.yaml");
    auto v = p1.parse();
    cout << v;
    return 0;
}
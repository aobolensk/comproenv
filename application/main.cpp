#include <iostream>
#include "yaml_parser.h"

using namespace std;

int main() {
    YAMLParser p1("1.txt");
    YAMLParser p2(p1);
    return 0;
}
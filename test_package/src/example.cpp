#include "stratoceph/stratoceph.h"
#include <vector>
#include <string>

int main() {
    stratoceph();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    stratoceph_print_vector(vec);
}

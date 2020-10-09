#include <iostream>
#include <list>
#include "mjson.hpp"

template<typename T, std::enable_if_t<param::utils::is_array<T>::value, int> = 0>
void print_array(const T& array, char sep = ' ') {
    for(const auto& el : array) {
        std::cout << el << sep;
    }
    std::cout << std::endl;
}

int main() {

    param::ParaManager pm;
    pm.load("test.json");

    std::cout << "---------- keys ----------\n";
    print_array(pm.keys(), '\n');
    std::cout << "--------------------------\n";

    std::cout << pm.get<bool>("/basic/bool") << std::endl;
    std::cout << pm.get<int>("/basic/integer") << std::endl;
    std::cout << pm.get<double>("/basic/float") << std::endl;
    std::cout << pm.get<std::string>("/basic/string") << std::endl;

    print_array(pm.get<std::list<int>>("/advanced/list"));
    print_array(pm.get<std::vector<double>>("/advanced/vector"));
    print_array(pm.get<std::vector<std::complex<double>>>("/advanced/complex_vector"));

    pm.set("/test/complex", std::complex<double>(6.0, -9.0));
    print_array(pm.keys(), '\n');
    std::cout << pm.get<std::complex<double>>("/test/complex") << std::endl;

    return 0;
}

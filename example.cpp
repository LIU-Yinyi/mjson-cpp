#include <iostream>
#include <list>
#include "mjson.hpp"
#include "contrib.hpp"

template<typename T, std::enable_if_t<param::utils::is_array<T>::value, int> = 0>
void print_array(const T& array, const std::string& sep = " ") {
    const auto* _sep = "";
    for(const auto& el : array) {
        std::cout << _sep << el;
        _sep = sep.c_str();
    }
    std::cout << std::endl;
}

template<typename T, typename C = typename T::value_type, std::enable_if_t<!param::utils::is_array<C>::value, int> = 0>
void print_matrix(const T& matrix, const std::string& sep = " ") {
    print_array(matrix, sep);
}

template<typename T, typename C = typename T::value_type, std::enable_if_t<param::utils::is_array<C>::value, int> = 0>
void print_matrix(const T& matrix, const std::string& sep = " ") {
    for(const auto& el : matrix) {
        print_matrix(el, sep);
    }
    std::cout << std::endl;
}

template <class T> using Mat4D = std::vector<std::vector<std::vector<std::vector<T>>>>;

int main() {

    param::ParaManager pm;
    pm.load("test.json");

    std::cout << "---------- keys ----------\n";
    print_array(pm.keys(), "\n");

    std::cout << "------ Get TEST ------" << std::endl;
    std::cout << pm.get<bool>("/basic/bool") << std::endl;
    std::cout << pm.get<int>("/basic/integer") << std::endl;
    std::cout << pm.get<double>("/basic/float") << std::endl;
    std::cout << pm.get<std::string>("/basic/string") << std::endl;

    print_array(pm.get<std::list<int>>("/advanced/list"), ", ");
    print_array(pm.get<std::vector<double>>("/advanced/vector"), ", ");
    print_array(pm.get<std::vector<std::complex<double>>>("/advanced/complex_vector"), ", ");

    std::cout << "------ Set TEST ------" << std::endl;
    pm.set("/test/bool", true);
    pm.set("/test/integer", 123);
    pm.set("/test/float", 456.789);
    pm.set("/test/c_str", "c-style-string");
    pm.set("/test/complex", std::complex<double>(6.0, -9.0));
    pm.erase("/test/bool");
    print_array(pm.keys(), "\n");
    std::cout << pm.get<std::complex<double>>("/test/complex") << std::endl;

    std::cout << "------ 2D Matrix ------" << std::endl;
    auto mat2d = pm.get<std::vector<std::vector<double>>>("/advanced/matrix2d");
    print_matrix(mat2d, ", ");

    std::cout << "------ 3D Matrix ------" << std::endl;
    auto mat3d = pm.get<std::vector<std::vector<std::vector<double>>>>("/advanced/matrix3d");
    print_matrix(mat3d, ", ");

    std::cout << "------ 4D Matrix ------" << std::endl;
    auto mat4d = pm.get<Mat4D<double>>("/advanced/matrix4d");
    print_matrix(mat4d, ", ");

    std::cout << "------ 2D Matrix Complex------" << std::endl;
    auto mat2cd = pm.get<std::vector<std::vector<std::complex<double>>>>("/advanced/complex_matrix");
    print_matrix(mat2cd, ", ");



    return 0;
}

/**
 * @file mjson/contrib.hpp
 * @brief Contribution Add-Ons with Mathematical JSON
 * @author LIU Yinyi
 * @date October 10, 2020
 * @version 0.1.0
 */

#ifndef MJSON_CPP_CONTRIB
#define MJSON_CPP_CONTRIB

// CMake Compile Configuration
#include "config.h"

#ifdef MJSON_CPP_USE_EIGEN
// Eigen Library
#include <Eigen/Dense>
#endif //MJSON_CPP_USE_EIGEN

namespace param {
    namespace contrib {

#ifdef MJSON_CPP_USE_EIGEN
        template<typename T> struct is_eigen : std::is_base_of<Eigen::EigenBase<T>, T> {};
#endif //MJSON_CPP_USE_EIGEN

    }
}

#endif //MJSON_CPP_CONTRIB

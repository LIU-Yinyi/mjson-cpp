# [mjson-cpp] Mathematical JSON
- **Author:** LIU-Yinyi
- **Date:** October 24, 2020
- **Version:** 0.1.1
- **Abstract:** Mathematical JSON supports scientific formats such as complex number and high-dimensional matrix based on RapidJSON.

![](https://img.shields.io/badge/build-passing-brightgreen)
![](https://img.shields.io/badge/coverage-90%25-green)
![](https://img.shields.io/badge/status-debug-orange)
![](https://img.shields.io/badge/standard-C%2B%2B14-blue)

## Overview
`mjson-cpp` is a header-only add-on, so the integration into your own project is quite easy. It is based on [Tencent/RapidJson](https://github.com/Tencent/rapidjson.git) due to the memory-friendly and high-efficient features. This project is a specific improvement on matrix and complex number reading and writing, making full use of C++14 template and SFINAE.


## Installation 
We froze the version of `Tencent/rapidjson` at `1.1.0` and copied the includes into `include/rapidjson`. Just download this repositories and integrate it into your project.

```bash
# For Debian/Ubuntu User
git clone https://github.com/LIU-Yinyi/mjson-cpp.git
```


### Manually Integration with your project
What you need is just to drag `include/mjson/mjson.hpp` to your project directories. And add the dependencies of `rapidjson` to your project. For example, in **CMakeLists.txt**, add:

```cmake
find_package(rapidjson REQUIRED)
include_directories(${RAPIDJSON_INCLUDE_DIRS})
```


## Interfaces
Here we simplify the APIs of `rapidjson` into only **FIVE** interfaces.

```cpp
//! wrapped in namespace "param"
namespace param {
	//! mjson-cpp main class named "ParaManager"
	class ParaManager {
	public:
		//! load json file by path name
		bool load(const std::string& filename);
		
		//! save json file by path name, use exist filename if nil
		void save(const std::string& filename);
		
		//! list all the keys of json file
		std::vector<std::string> keys();
		
		//! get any type you like from json file by full path
		template<class T> T get(const std::string& keypath);
		
		//! set any type you like into json file by full path
		template<class T> void set(const std::string& keypath, const T& value);
	};
}
```

Assisted by `mjson-cpp` library, you can achieve the functions like:

```cpp
template<class T> using Matrix3D = std::vector<std::vector<std::vector<T>>>;

param::ParaManager pm;
pm.load("/path/to/config.json");

std::string keypath{"/full/path/to/param/complex/matrix"};
auto matrix3d = pm.get<Matrix3D<std::complex<double>>>(keypath);

print_matrix(matrix3d, ", "); //!< utility in "example.cpp"
```

One sample input / output is shown below:

```bash
>>> Inputs:
{
	"matrix3d": [[[1,2],[3,4]],[[5,6],[7,8]],[[9,10],[11,12]]]
}

>>> Outputs:
1, 2
3, 4

5, 6
7, 8

9, 10
11, 12
```

More examples can be found in `example.cpp`.
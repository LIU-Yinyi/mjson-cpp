/**
 * @file mjson.hpp
 * @brief Mathematical JSON as Parameters Manager based on RapidJSON
 * @author LIU Yinyi
 * @date October 10, 2020
 * @version 0.1.0
 */

#ifndef MJSON_CPP
#define MJSON_CPP

// Standard Template Library
#include <type_traits>
#include <fstream>
#include <complex>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

// RapidJSON Library
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

namespace param {
    namespace utils {

    template<typename T> struct is_bool : std::is_same<T, bool> {};

    template<typename T> struct is_integral : std::integral_constant<bool, std::is_integral<T>::value && !is_bool<T>::value> {};

    template<typename T> using is_floating = std::is_floating_point<T>;

    template<typename T> struct is_real : std::integral_constant<bool, is_integral<T>::value || is_floating <T>::value> {};

    template<typename T> struct is_complex : std::false_type {};
    template<typename T> struct is_complex<std::complex<T>> : std::true_type {};

    template<typename T> struct is_c_string : std::integral_constant<bool, std::is_same<char const *, std::decay_t<T>>::value || std::is_same<char *, std::decay_t<T>>::value> {};

    template<typename T> struct is_cpp_string : std::false_type {};
    template<typename charT, typename traits, typename Alloc> struct is_cpp_string<std::basic_string<charT, traits, Alloc>> : std::true_type {};

    template<typename T> struct is_string : std::integral_constant<bool, is_c_string<T>::value || is_cpp_string<T>::value> {};

    template<typename T> struct is_array : std::false_type {};
    template<template<typename, typename> class C, typename T, typename Alloc> struct is_array<C<T, Alloc>> : std::true_type {};

    }
}

namespace param {

    class ParaManager {
    public:
        ParaManager() = default;
        ParaManager(const ParaManager&) = delete;
        ParaManager& operator = (ParaManager&&) noexcept = default;
        ~ParaManager() = default;

        bool load(const std::string& filename) {
            std::fstream ifs(filename);
            if(!ifs.good()) return false;
            this->filename_ = filename;
            rapidjson::IStreamWrapper isw(ifs);
            document_.ParseStream(isw);
            return true;
        }

        void save(const std::string& filename = std::string()) {
            std::string _savepath{};
            if(filename.empty()) {
                _savepath = this->filename_;
            } else {
                _savepath = filename;
            }
            std::ofstream ofs(_savepath);
            rapidjson::OStreamWrapper osw(ofs);
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
            writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
            this->document_.Accept(writer);
        }

        bool erase(const std::string& keypath) {
            return rapidjson::Pointer(keypath.c_str()).Erase(this->document_);
        }

        std::vector<std::string> keys() {
            std::vector<std::string> v{};
            std::function<void(const rapidjson::Value&, const rapidjson::Pointer&)> dumpFunc =
                    [&v, &dumpFunc](const rapidjson::Value& value, const rapidjson::Pointer& parent) -> void {
                        if(value.IsObject()) {
                            for(auto itr = value.MemberBegin(); itr != value.MemberEnd(); itr++) {
                                dumpFunc(itr->value, parent.Append(itr->name.GetString(), itr->name.GetStringLength()));
                            }
                        } else {
                            rapidjson::StringBuffer buf{};
                            parent.Stringify(buf);
                            v.emplace_back(buf.GetString());
                        }
                    };
            rapidjson::Pointer root;
            dumpFunc(this->document_, root);
            return v;
        }

        template<class T, std::enable_if_t<!utils::is_array<T>::value, int> = 0> auto get(const std::string& keypath, const T& default_val = T()) {
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            return get_<T>(_ptr, default_val);
        }

        template<class T, class C = typename T::value_type, std::enable_if_t<utils::is_array<T>::value, int> = 0> auto get(const std::string& keypath, const T& default_val = T()) {
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            T value{};
            if(_ptr->IsArray()) {
                size_t _index = 0;
                for(auto& item : _ptr->GetArray()) {
                    if(!item.IsNull()) {
                        std::string _subpath(keypath);
                        _subpath += "/" + std::to_string(_index++);
                        value.emplace_back(get<C>(_subpath));
                    }
                }
                return value;
            } else {
                return default_val;
            }
        }

        template<class T, std::enable_if_t<!utils::is_array<T>::value, int> = 0> void set(const std::string& keypath, const T& value) {
            rapidjson::Pointer(keypath.c_str()).Create(this->document_);
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            set_<T>(_ptr, value);
        }

        template<class T, class C = typename T::value_type, std::enable_if_t<utils::is_array<T>::value, int> = 0> void set(const std::string& keypath, const T& value) {
            size_t _index = 0;
            for(auto& item: value) {
                std::string _subpath(keypath);
                _subpath += "/" + std::to_string(_index++);
                set<C>(_subpath, item);
            }
        }

    protected:
        template<class T, std::enable_if_t<utils::is_string<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            value_ptr->Accept(writer);
            std::string str = buf.GetString();
            str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
            return static_cast<T>(str);
        }

        template<class T, std::enable_if_t<utils::is_bool<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            if(value_ptr->IsBool()) return static_cast<T>(value_ptr->GetBool()); else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_integral<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            if(value_ptr->IsNumber()) return static_cast<T>(value_ptr->GetInt64()); else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_floating<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            if(value_ptr->IsNumber()) return static_cast<T>(value_ptr->GetDouble()); else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_complex<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            auto _str = get_<std::string>(value_ptr);
            std::istringstream iss(_str);
            if(iss.good()) { T value{}; iss >> value; return static_cast<T>(value); } else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_array<T>::value, int> = 0>
        T get_(rapidjson::Value* , const T& default_value = T()) { return default_value; }

        template<class T, std::enable_if_t<utils::is_cpp_string<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if (value_ptr == nullptr) return;
            value_ptr->SetString(value.data(), value.size(), this->document_.GetAllocator());
        }

        template<class T, std::enable_if_t<utils::is_c_string<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            value_ptr->SetString(value, this->document_.GetAllocator());
        }


        template<class T, std::enable_if_t<utils::is_bool<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            value_ptr->SetBool(value);
        }

        template<class T, std::enable_if_t<utils::is_integral<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            value_ptr->SetInt64(value);
        }

        template<class T, std::enable_if_t<utils::is_floating<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            value_ptr->SetDouble(value);
        }

        template<class T, std::enable_if_t<utils::is_complex<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            std::stringstream ss; ss << value;
            set_<std::string>(value_ptr, ss.str());
        }

    protected:
        std::string filename_;
        rapidjson::Document document_;
    };

}

#endif //MJSON_CPP

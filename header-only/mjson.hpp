

#ifndef MJSON_CPP
#define MJSON_CPP

#define MJSON_CPP_USE_EIGEN

// Standard Template Library
#include <type_traits>
#include <fstream>
#include <complex>
#include <string>
#include <vector>

// RapidJSON Library
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#ifdef MJSON_CPP_USE_EIGEN
// Eigen Library
#include <Eigen/Dense>
#endif //MJSON_CPP_USE_EIGEN

namespace param::utils {

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
    template<template<typename, typename> typename C, typename T, typename Alloc> struct is_array<C<T, Alloc>> : std::true_type {};

#ifdef MJSON_CPP_USE_EIGEN
    template<typename T> struct is_eigen : std::is_base_of<Eigen::EigenBase<T>, T> {};
#endif //MJSON_CPP_USE_EIGEN
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
            return std::move(v);
        }

        template<class T, std::enable_if_t<!utils::is_array<T>::value, int> = 0> auto get(const std::string& keypath, const T& default_val = T()) {
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            return get_<T>(_ptr, default_val);
        }

        template<class T, class C = typename T::value_type, std::enable_if_t<utils::is_array<T>::value, int> = 0> auto get(const std::string& keypath, const T& default_val = T()) {
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            T value{};
            if(_ptr->IsArray()) {
                for(auto& item : _ptr->GetArray()) {
                    value.emplace_back(get_<C>(&item));
                }
                return value;
            } else {
                return default_val;
            }
        }

        template<class T> void set(const std::string& keypath, const T& value) {
            auto _ptr = rapidjson::Pointer(keypath.c_str()).Get(this->document_);
            set_<T>(_ptr, value);
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
            if(value_ptr->IsInt64()) return static_cast<T>(value_ptr->GetInt64()); else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_floating<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            if(value_ptr->IsDouble()) return static_cast<T>(value_ptr->GetDouble()); else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_complex<T>::value, int> = 0>
        T get_(rapidjson::Value* value_ptr, const T& default_value = T()) {
            if(value_ptr == nullptr) return default_value;
            auto _str = get_<std::string>(value_ptr);
            std::istringstream iss(_str);
            if(iss.good()) { T value{}; iss >> value; return static_cast<T>(value); } else return default_value;
        }

        template<class T, std::enable_if_t<utils::is_string<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if (value_ptr == nullptr) return;
            value_ptr->SetString(value, this->document_.GetAllocator());
        }

        template<class T, std::enable_if_t<utils::is_real<T>::value, int> = 0>
        void set_(rapidjson::Value* value_ptr, const T& value) {
            if(value_ptr == nullptr) return;
            rapidjson::Value _val(value, this->document_.GetAllocator());
            value_ptr->Set(_val);
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

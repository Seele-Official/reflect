
#define REFLECT
#ifdef REFLECT

#include <cstddef>
#include <string_view>
#include <cstring>
#include <typeindex>
#include <unordered_map>

struct typeinfo{
    size_t offset;
    size_t size;
    std::type_index typeindex;
};

struct map {
    std::string_view keyname;
    size_t offset;
    size_t size;
};


// must be constexpr
class staticReflectVar {
    public:
    void *classBase;
    const size_t offset;
    const size_t size;
    constexpr staticReflectVar() : classBase(nullptr), offset(0), size(0) {}
    staticReflectVar(const staticReflectVar &other) : classBase(other.classBase), offset(other.offset), size(other.size) {}
    constexpr staticReflectVar(void *classBase, size_t offset, size_t size) : classBase(classBase), offset(offset), size(size) {}

    template <typename T>
    auto operator=(const T &value) const {
        
        if (size > sizeof(T)) {
            // error
        }
        
        memcpy(static_cast<char *>(classBase) + offset, &value, size);
        return *this;
    }

    template <typename T>
    auto getValue() const {
        T value;
        memcpy(&value, static_cast<char *>(classBase) + offset, sizeof(T));
        return value;
    }

    template <typename T>
    auto setValue(const T &value) const {
        return (*this) = value;
    }


    constexpr bool isNull() const {
        return classBase == nullptr;
    }
};



class ReflectVar{
    public:
    void *classBase;
    const typeinfo info;   
    ReflectVar() : classBase(nullptr), info{0, 0, std::type_index(typeid(void))} {}
    ReflectVar(const ReflectVar &other) : classBase(other.classBase), info{other.info} {}
    ReflectVar(void *classBase, typeinfo info) : classBase(classBase), info{info} {}


    template <typename T>
    auto operator=(const T &value) {
        if (std::type_index(typeid(T)) != info.typeindex) {
            // error
        }
        memcpy(static_cast<char *>(classBase) + info.offset, &value, info.size);
        return *this;
    }

    template<typename T>
    auto setValue(const T &value) {
        return (*this) = value;
    }


    template<typename T>
    auto getValue() const {
        if (std::type_index(typeid(T)) != info.typeindex) {
            // error
        }
        T value;
        memcpy(&value, static_cast<char *>(classBase) + info.offset, sizeof(T));
        return value;
    }
    auto& getInfo() const {
        return info;
    }

    bool isNull() const {
        return classBase == nullptr;
    }

};


template <typename T>
auto& typeInfo() {
    // error
    return nullptr;
}



template <typename T>
constexpr staticReflectVar staticReflect(T &c, std::string_view name) {

    // error
    return staticReflectVar{};
}

template <typename T>
ReflectVar reflect(T &c, std::string_view name) {
    
    //error
    return ReflectVar{};
}








#endif
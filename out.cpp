#include "reflect.hpp"
#include <iostream>
struct __attribute__((annotate("reflect"))) test{
    public:
    int testname;
    int testname2;
    double testname3;
    struct test2{
        int testname;
    };
    test2 testname4;

};
//static reflect--------------------------------
template<>
constexpr staticReflectVar staticReflect<test>(test &c, std::string_view name) {
constexpr auto keynames = std::array<map, 4>{
map{"testname", offsetof(test, testname), sizeof(test::testname)},
map{"testname2", offsetof(test, testname2), sizeof(test::testname2)},
map{"testname3", offsetof(test, testname3), sizeof(test::testname3)},
map{"testname4", offsetof(test, testname4), sizeof(test::testname4)},
};
for (const auto& keyname : keynames){
if (keyname.keyname == name){
return staticReflectVar{&c, keyname.offset, keyname.size};
}
}
return staticReflectVar{};
}
//static reflect--------------------------------


//dynamic reflect--------------------------------
template <>
auto& typeInfo<test>() {
const static std::unordered_map<std::string_view, typeinfo> typeinfos = {
{"testname", {offsetof(test, testname), sizeof(test::testname), std::type_index(typeid(test::testname))}},
{"testname2", {offsetof(test, testname2), sizeof(test::testname2), std::type_index(typeid(test::testname2))}},
{"testname3", {offsetof(test, testname3), sizeof(test::testname3), std::type_index(typeid(test::testname3))}},
{"testname4", {offsetof(test, testname4), sizeof(test::testname4), std::type_index(typeid(test::testname4))}},
};
return typeinfos;
}
template <>
ReflectVar reflect<test>(test &c, std::string_view name) {
auto& typeinfos = typeInfo<test>();
auto it = typeinfos.find(name);
if (it != typeinfos.end()){
return ReflectVar{&c, it->second};
}
return ReflectVar{};
}
//dynamic reflect--------------------------------


test temp = {1,2};
int main(){
    
    constexpr auto b = staticReflect(temp, "testname");
    if constexpr (!b.isNull()){
        std::cout << b.getValue<int>() << std::endl;
        b.setValue<int>(3);
        std::cout << b.getValue<int>() << std::endl;
    }


    test temp1 = {1,2};
    std::string_view keyname = "testname";
    auto testname = reflect(temp1, keyname);
    if (!testname.isNull()){
        if (testname.getInfo().typeindex == std::type_index(typeid(int))){
            std::cout << "type is int" << std::endl;
            std::cout << testname.getValue<int>() << std::endl;
            testname.setValue<int>(3);

            std::cout << testname.getValue<int>() << std::endl;
        }
    }
}

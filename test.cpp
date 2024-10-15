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
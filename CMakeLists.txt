cmake_minimum_required(VERSION 3.22)
project(Reflect)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)


include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_executable(Reflect
reflect.cpp
  )
target_link_libraries(Reflect
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
)
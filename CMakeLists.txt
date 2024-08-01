cmake_minimum_required(VERSION 3.16)
project(cpu)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Wextra -Wpedantic)
# add_compile_options(-O3)

add_executable(code
               inst.hpp
               util.hpp
               core.hpp
               main.cpp
              )
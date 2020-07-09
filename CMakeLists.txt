cmake_minimum_required(VERSION 3.5.2)
project(code)

set(CMAKE_CXX_STANDARD 14)

add_executable(code main.cpp funcs_and_tools.hpp memory.hpp program.hpp)

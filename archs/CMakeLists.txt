cmake_minimum_required(VERSION 3.4.1)
project(arch)

set(CMAKE_CXX_STANDARD 11)
include_directories(..)
include_directories(../osutils)
add_library(arch STATIC archbase.cpp aarch64.cpp x86.cpp)

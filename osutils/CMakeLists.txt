cmake_minimum_required(VERSION 3.4.1)
project(osutils)

set(CMAKE_CXX_STANDARD 11)
if (APPLE)
    add_library(osutils STATIC osutils_mac.cpp)
else()
    if (WIN32)
        add_library(osutils STATIC osutils_win.cpp)
    endif()
endif()


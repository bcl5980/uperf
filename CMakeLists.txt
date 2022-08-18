cmake_minimum_required(VERSION 3.4.1)
project(uperf)

set(CMAKE_CXX_STANDARD 11)
add_subdirectory(osutils)
add_executable(uperf uperf.cpp pat_config.cpp)
target_link_libraries(uperf osutils)

# note--this file was written by Claude
cmake_minimum_required(VERSION 3.14)
project(weighted_set_tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 3 REQUIRED)

add_executable(tests weighted_set_tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

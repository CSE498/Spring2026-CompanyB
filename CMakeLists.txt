cmake_minimum_required(VERSION 3.16)
project(Spring2026CompanyB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

include(CTest)

add_subdirectory(third-party/Catch)

add_executable(unit_tests
  tests/test_example.cpp
  tests/tools/test_circle.cpp
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2)
target_include_directories(unit_tests PRIVATE
  source/tools
)

include(third-party/Catch/contrib/Catch.cmake)
catch_discover_tests(unit_tests)

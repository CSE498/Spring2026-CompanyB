# CMake generated Testfile for 
# Source directory: /Users/rancho/Desktop/capstone/Spring2026-CompanyB
# Build directory: /Users/rancho/Desktop/capstone/Spring2026-CompanyB/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/Users/rancho/Desktop/capstone/Spring2026-CompanyB/build/unit_tests_include-b12d07c.cmake")
add_test(BoxUnitTests "/Users/rancho/Desktop/capstone/Spring2026-CompanyB/build/box_unit_tests")
set_tests_properties(BoxUnitTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/rancho/Desktop/capstone/Spring2026-CompanyB/CMakeLists.txt;38;add_test;/Users/rancho/Desktop/capstone/Spring2026-CompanyB/CMakeLists.txt;0;")
subdirs("third-party/Catch")

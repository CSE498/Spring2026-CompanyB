/**
 * @file AnnotationSetTest.cpp
 * @brief Unit tests for the AnnotationSet class.
 * @author Nicholas McComb
 * @date 2/10/2026
 */

#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>
#include <climits>

#include "AnnotationSet.hpp"

// g++ -std=c++23 -Wall -Wextra -I../../../third-party/Catch/single_include AnnotationSet.cpp AnnotationSetTest.cpp -o AnnotationSetTest.exe

//constructor 

TEST_CASE("AnnotationSet constructor", "[Constructor]"){
    AnnotationSet s;
    CHECK(s.size() == 0);
    CHECK(s.empty() == true);
}

//insert
TEST_CASE("AnnotationSet AddTag", "[AddTag]"){
    AnnotationSet s;
    s.addTag("tag1");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("tag1") == true);
}
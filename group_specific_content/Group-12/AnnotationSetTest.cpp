/**
 * @file AnnotationSetTest.cpp
 * @brief Unit tests for the AnnotationSet class.
 * @author Nicholas McComb
 * @date 2/10/2026
 */

#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>
#include <climits>

#include "AnnotationSet.hpp"


//constructor 

TEST_CASE("Annotation set constructor", "[AnnotationSet]"){
    AnnotationSet s;
    CHECK(s.size() == 0);
    CHECK(s.empty() == true);
}
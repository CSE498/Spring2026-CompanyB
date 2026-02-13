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
    CHECK(s.hasTag("help") == false);
}

//insert
TEST_CASE("AddTag one element", "[addTag]"){
    AnnotationSet s;
    s.addTag("tag1");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("tag1"));
}

TEST_CASE("AddTag multiple elements", "[addTag]"){
    AnnotationSet s; 
    s.addTag("fancy");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("fancy"));

    s.addTag("groovy");
    s.addTag("slick");
    CHECK(s.size() == 3);
    CHECK((s.hasTag("groovy")&& s.hasTag("slick")));
    CHECK(s.getTags() == std::unordered_set<std::string>{"fancy","groovy","slick"});
}

TEST_CASE("AddTag multiple of the same element", "[addTag]"){
    AnnotationSet s;
    s.addTag("agent");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("agent"));

    s.addTag("agent");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("agent"));

    s.addTag("agent");
    s.addTag("agent");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("agent"));
}

TEST_CASE("AddTag insert element then move memory", "[addTag]"){
    AnnotationSet s;
    s.addTag("s");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("s")); 
    CHECK(s.getTags() == std::unordered_set<std::string>{"s"});  

    AnnotationSet d = std::move(s);
    CHECK(d.size()== 1);
    CHECK(d.hasTag("s"));
    CHECK(d.getTags() == std::unordered_set<std::string>{"s"});
}

//remove 

TEST_CASE("removeTag add one tag and then remove", "[removeTag]"){
    AnnotationSet set; 
    set.addTag("happy");
    CHECK(set.size() == 1);
    CHECK(set.hasTag("happy"));
    CHECK(set.getTags() == std::unordered_set<std::string>{"happy"});

    set.removeTag("happy");
    CHECK(set.size() == 0);
    CHECK(set.empty());
    CHECK(set.hasTag("happy")==false);
    CHECK(set.getTags() == std::unordered_set<std::string>{});
}

TEST_CASE("remove tag that doesn't exsist", "[removeTag]"){
    AnnotationSet g;
    CHECK(g.size() == 0);
    CHECK(g.empty());
    CHECK(g.removeTag("here") == false);
    CHECK(g.size() == 0);
    CHECK(g.empty());
}

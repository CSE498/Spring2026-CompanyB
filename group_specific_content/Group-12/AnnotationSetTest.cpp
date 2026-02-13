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
    CHECK_FALSE(s.hasTag("help"));
}

//insert
TEST_CASE("AddTag one element", "[addTag]"){
    AnnotationSet s;
    s.addTag("tag1");
    CHECK(s.size() == 1);
    CHECK(s.hasTag("tag1"));
}

TEST_CASE("Insert empty string", "[addTag]") {
    AnnotationSet s;
    s.addTag("");

    CHECK(s.size() == 1);
    CHECK(s.hasTag(""));
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

TEST_CASE("Case sensitivity", "[addTag]") {
    AnnotationSet s;
    s.addTag("Hello");
    s.addTag("hello");

    CHECK(s.size() == 2);
    CHECK(s.hasTag("Hello"));
    CHECK(s.hasTag("hello"));
}

TEST_CASE("Insert many elements stress test", "[addTag]") {
    AnnotationSet s;

    for (int i = 0; i < 1000; ++i) {
        s.addTag("tag" + std::to_string(i));
    }

    CHECK(s.size() == 1000);

    for (int i = 0; i < 1000; ++i) {
        CHECK(s.hasTag("tag" + std::to_string(i)));
    }
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
    CHECK_FALSE(set.hasTag("happy"));
    CHECK(set.getTags() == std::unordered_set<std::string>{});
}

TEST_CASE("remove tag that doesn't exsist", "[removeTag]"){
    AnnotationSet g;
    CHECK(g.size() == 0);
    CHECK(g.empty());
    CHECK_FALSE(g.removeTag("here"));
    CHECK(g.size() == 0);
    CHECK(g.empty());
}

TEST_CASE("Moved-from object remains valid", "[removeTag]") {

    AnnotationSet s;
    s.addTag("x");
    AnnotationSet moved = std::move(s);

    CHECK(moved.size() == 1);
    CHECK(moved.hasTag("x"));
    
    CHECK((s.size() == 0 || s.size() == 1)); 
    CHECK_NOTHROW(s.hasTag("x"));
    CHECK_NOTHROW(s.addTag("y"));
}

TEST_CASE("Remove same tag twice", "[removeTag]") {
    AnnotationSet s;
    s.addTag("dup");

    CHECK(s.removeTag("dup") == true);
    CHECK(s.size() == 0);

    CHECK_FALSE(s.removeTag("dup"));
    CHECK(s.size() == 0);
}

#include <algorithm>
#include <vector>
#include <random>

TEST_CASE("removeTag randomized stress", "[removeTag]") {
    AnnotationSet s;
    const int N = 1000;
    std::vector<std::string> tags;

    for (int i = 0; i < N; ++i) {
        std::string t = "tag" + std::to_string(i);
        tags.push_back(t);
        s.addTag(t);
    }
    std::shuffle(tags.begin(), tags.end(), std::mt19937{std::random_device{}()});

    for (int i = 0; i < N; ++i) {
        CHECK(s.removeTag(tags[i]) == true);
    }

    CHECK(s.empty());
    CHECK(s.size() == 0);
}

//gettag 

TEST_CASE("getTag returns correct pointer", "[getTag]") {
    AnnotationSet s;
    s.addTag("alpha");

    const auto* ptr = s.getTag("alpha");
    REQUIRE(ptr != nullptr);
    CHECK(*ptr == "alpha");

    CHECK(s.getTag("beta") == nullptr);
}

//gettags

TEST_CASE("getTags returns const reference", "[getTags]") {
    AnnotationSet s;
    s.addTag("a");

    const auto& ref = s.getTags();
    CHECK(ref.size() == 1);
    CHECK(ref.count("a") == 1);
}

//clear

TEST_CASE("clear removes all tags", "[clear]") {
    AnnotationSet s;

    s.addTag("one");
    s.addTag("two");
    s.addTag("three");

    REQUIRE(s.size() == 3);
    REQUIRE_FALSE(s.empty());

    s.clear();

    CHECK(s.size() == 0);
    CHECK(s.empty());
    CHECK_FALSE(s.hasTag("one"));
    CHECK_FALSE(s.hasTag("two"));
    CHECK_FALSE(s.hasTag("three"));
    CHECK(s.getTags() == std::unordered_set<std::string>{});
}

TEST_CASE("clear on empty set is safe", "[clear]") {
    AnnotationSet s;

    CHECK(s.empty());
    CHECK(s.size() == 0);

    s.clear();  

    CHECK(s.empty());
    CHECK(s.size() == 0);
}

TEST_CASE("clear can be called multiple times safely", "[clear]") {
    AnnotationSet s;

    s.addTag("a");
    s.addTag("b");

    REQUIRE(s.size() == 2);

    s.clear();
    CHECK(s.empty());

    s.clear();
    CHECK(s.empty());
    CHECK(s.size() == 0);
}

TEST_CASE("clear after move works correctly", "[clear]") {
    AnnotationSet s;
    s.addTag("x");
    s.addTag("y");
    AnnotationSet moved = std::move(s);
    moved.clear();

    CHECK(moved.empty());
    CHECK(moved.size() == 0);
    CHECK_FALSE(moved.hasTag("x"));
}

//empty 

TEST_CASE("empty reflects correct state", "[empty]") {
    AnnotationSet s;

    CHECK(s.empty());
    CHECK(s.size() == 0);

    s.addTag("hello");
    CHECK_FALSE(s.empty());
    CHECK(s.size() == 1);

    s.removeTag("hello");
    CHECK(s.empty());
    CHECK(s.size() == 0);
}

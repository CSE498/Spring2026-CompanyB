#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>
#include "DataMap.hpp"

TEST_CASE("DataMap can store and retrieve values", "[DataMap]") {
    DataMap data;
    
    SECTION("Store and retrieve a double") {
        data.Set("health", 45.0);
        double health = data.Get<double>("health");
        
        REQUIRE(health == 45.0);
    }
    
    SECTION("Store and retrieve an int") {
        data.Set("score", 100);
        int score = data.Get<int>("score");
        
        REQUIRE(score == 100);
    }
    
    SECTION("Store and retrieve a string") {
        data.Set("playerName", std::string("Alice"));
        std::string name = data.Get<std::string>("playerName");
        
        REQUIRE(name == "Alice");
    }
    
    SECTION("Store and retrieve a bool") {
        data.Set("isAlive", true);
        bool alive = data.Get<bool>("isAlive");
        
        REQUIRE(alive == true);
    }
}

TEST_CASE("DataMap can update existing values", "[DataMap]") {
    DataMap data;
    
    data.Set("health", 100.0);
    data.Set("health", 50.0);  // Update
    
    double health = data.Get<double>("health");
    REQUIRE(health == 50.0);
}

TEST_CASE("DataMap can store multiple different types", "[DataMap]") {
    DataMap data;
    
    data.Set("health", 45.0);
    data.Set("score", 100);
    data.Set("playerName", std::string("Bob"));
    data.Set("isAlive", true);
    
    SECTION("All values can be retrieved correctly") {
        double health = data.Get<double>("health");
        int score = data.Get<int>("score");
        std::string name = data.Get<std::string>("playerName");
        bool alive = data.Get<bool>("isAlive");
        
        REQUIRE(health == 45.0);
        REQUIRE(score == 100);
        REQUIRE(name == "Bob");
        REQUIRE(alive == true);
    }
}

TEST_CASE("DataMap handles many items", "[DataMap]") {
    DataMap data;
    
    // Add many items
    for (int i = 0; i < 20; ++i) {
        data.Set("key" + std::to_string(i), i * 10);
    }
    
    SECTION("All items can still be retrieved") {
        for (int i = 0; i < 20; ++i) {
            int value = data.Get<int>("key" + std::to_string(i));
            REQUIRE(value == i * 10);
        }
    }
}

TEST_CASE("DataMap can store complex types", "[DataMap]") {
    DataMap data;
    
    SECTION("Store and retrieve a vector") {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        data.Set("numbers", vec);
        
        std::vector<int> retrieved = data.Get<std::vector<int>>("numbers");
        REQUIRE(retrieved == vec);
    }
    
    SECTION("Store and retrieve a pair") {
        std::pair<int, std::string> p = {42, "answer"};
        data.Set("pair", p);
        
        auto retrieved = data.Get<std::pair<int, std::string>>("pair");
        REQUIRE(retrieved.first == 42);
        REQUIRE(retrieved.second == "answer");
    }
}
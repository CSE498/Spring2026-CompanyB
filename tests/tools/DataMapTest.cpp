#include "../../source/tools/DataMap.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>

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

TEST_CASE("DataMap can overwrite with different type", "[DataMap]") {
    DataMap data;
    data.Set("value", 42);
    data.Set("value", std::string("hello"));  // Different type
    std::string result = data.Get<std::string>("value");
    REQUIRE(result == "hello");
}

TEST_CASE("DataMap handles empty strings", "[DataMap]") {
    DataMap data;
    data.Set("empty", std::string(""));
    std::string result = data.Get<std::string>("empty");
    REQUIRE(result == "");
}

TEST_CASE("DataMap handles zero and negative numbers", "[DataMap]") {
    DataMap data;
    
    SECTION("Zero values") {
        data.Set("zero_int", 0);
        data.Set("zero_double", 0.0);
        REQUIRE(data.Get<int>("zero_int") == 0);
        REQUIRE(data.Get<double>("zero_double") == 0.0);
    }
    
    SECTION("Negative values") {
        data.Set("neg_int", -42);
        data.Set("neg_double", -3.14);
        REQUIRE(data.Get<int>("neg_int") == -42);
        REQUIRE(data.Get<double>("neg_double") == -3.14);
    }
}

TEST_CASE("DataMap handles special floating point values", "[DataMap]") {
    DataMap data;
    
    SECTION("Very large numbers") {
        data.Set("large", 1e308);
        REQUIRE(data.Get<double>("large") == 1e308);
    }
    
    SECTION("Very small numbers") {
        data.Set("small", 1e-308);
        REQUIRE(data.Get<double>("small") == 1e-308);
    }
}

TEST_CASE("DataMap handles special characters in keys", "[DataMap]") {
    DataMap data;
    data.Set("key_with_underscores", 1);
    data.Set("key-with-dashes", 2);
    data.Set("key.with.dots", 3);
    data.Set("key with spaces", 4);
    
    REQUIRE(data.Get<int>("key_with_underscores") == 1);
    REQUIRE(data.Get<int>("key-with-dashes") == 2);
    REQUIRE(data.Get<int>("key.with.dots") == 3);
    REQUIRE(data.Get<int>("key with spaces") == 4);
}

TEST_CASE("DataMap handles nested containers", "[DataMap]") {
    DataMap data;
    
    SECTION("Vector of vectors") {
        std::vector<std::vector<int>> nested = {{1, 2}, {3, 4, 5}};
        data.Set("nested", nested);
        auto result = data.Get<std::vector<std::vector<int>>>("nested");
        REQUIRE(result == nested);
        REQUIRE(result[0][0] == 1);
        REQUIRE(result[1][2] == 5);
    }
    
    SECTION("Vector of pairs") {
        std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}};
        data.Set("vec_pairs", vec);
        auto result = data.Get<std::vector<std::pair<int, std::string>>>("vec_pairs");
        REQUIRE(result == vec);
        REQUIRE(result[0].second == "one");
    }
}

TEST_CASE("DataMap handles large strings", "[DataMap]") {
    DataMap data;
    std::string large(10000, 'a');  // 10,000 character string
    data.Set("large_string", large);
    std::string result = data.Get<std::string>("large_string");
    REQUIRE(result == large);
    REQUIRE(result.length() == 10000);
}

TEST_CASE("DataMap handles unicode strings", "[DataMap]") {
    DataMap data;
    std::string unicode = "Hello 世界 🌍";
    data.Set("unicode", unicode);
    std::string result = data.Get<std::string>("unicode");
    REQUIRE(result == unicode);
}

TEST_CASE("DataMap stress test with alternating types", "[DataMap]") {
    DataMap data;
    
    // Store and update the same key multiple times with different types
    data.Set("multi", 42);
    REQUIRE(data.Get<int>("multi") == 42);
    
    data.Set("multi", 3.14);
    REQUIRE(data.Get<double>("multi") == 3.14);
    
    data.Set("multi", std::string("text"));
    REQUIRE(data.Get<std::string>("multi") == "text");
    
    data.Set("multi", true);
    REQUIRE(data.Get<bool>("multi") == true);
}

TEST_CASE("DataMap handles custom struct types", "[DataMap]") {
    struct Player {
        std::string name;
        int level;
        double health;
        
        bool operator==(const Player& other) const {
            return name == other.name && level == other.level && health == other.health;
        }
    };
    
    DataMap data;
    Player p1{"Hero", 10, 100.0};
    data.Set("player", p1);
    
    Player retrieved = data.Get<Player>("player");
    REQUIRE(retrieved == p1);
    REQUIRE(retrieved.name == "Hero");
    REQUIRE(retrieved.level == 10);
    REQUIRE(retrieved.health == 100.0);
}

// NOTE: If any of the following happens, program will trigger assert() and abort:
// 
// 1. Getting a non-existent key:
//    DataMap data;
//    data.Get<int>("nonexistent");  // FAILS: assert(it != mData.end())
//
// 2. Getting with wrong type:
//    DataMap data;
//    data.Set("value", 42);
//    data.Get<double>("value");  // FAILS: assert(it->second.type() == typeid(V))
//
// This behavior is intended as referenced from the class description, but it can't be tested
// automatically because assert() terminates the program.
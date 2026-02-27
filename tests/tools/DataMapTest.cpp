#include "../../source/tools/DataMap.hpp"
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>
#include <vector>
#include <set>

TEST_CASE("DataMap can store and retrieve values", "[DataMap]") {
    cse498::DataMap data;

    SECTION("Store and retrieve a double") {
        data.Set("health", 45.0);
        auto health = data.Get<double>("health");
        REQUIRE(health.has_value());
        REQUIRE(health.value() == 45.0);
    }

    SECTION("Store and retrieve an int") {
        data.Set("score", 100);
        auto score = data.Get<int>("score");
        REQUIRE(score.has_value());
        REQUIRE(score.value() == 100);
    }

    SECTION("Store and retrieve a string") {
        data.Set("playerName", std::string("Alice"));
        auto name = data.Get<std::string>("playerName");
        REQUIRE(name.has_value());
        REQUIRE(name.value() == "Alice");
    }

    SECTION("Store and retrieve a bool") {
        data.Set("isAlive", true);
        auto alive = data.Get<bool>("isAlive");
        REQUIRE(alive.has_value());
        REQUIRE(alive.value() == true);
    }
}

TEST_CASE("DataMap can update existing values", "[DataMap]") {
    cse498::DataMap data;
    data.Set("health", 100.0);
    data.Set("health", 50.0);  // Update
    auto health = data.Get<double>("health");
    REQUIRE(health.has_value());
    REQUIRE(health.value() == 50.0);
}

TEST_CASE("DataMap can store multiple different types", "[DataMap]") {
    cse498::DataMap data;
    data.Set("health", 45.0);
    data.Set("score", 100);
    data.Set("playerName", std::string("Bob"));
    data.Set("isAlive", true);

    SECTION("All values can be retrieved correctly") {
        REQUIRE(data.Get<double>("health").value() == 45.0);
        REQUIRE(data.Get<int>("score").value() == 100);
        REQUIRE(data.Get<std::string>("playerName").value() == "Bob");
        REQUIRE(data.Get<bool>("isAlive").value() == true);
    }
}

TEST_CASE("DataMap handles many items", "[DataMap]") {
    cse498::DataMap data;

    for (int i = 0; i < 20; ++i) {
        data.Set("key" + std::to_string(i), i * 10);
    }

    SECTION("All items can still be retrieved") {
        for (int i = 0; i < 20; ++i) {
            auto value = data.Get<int>("key" + std::to_string(i));
            REQUIRE(value.has_value());
            REQUIRE(value.value() == i * 10);
        }
    }
}

TEST_CASE("DataMap can store complex types", "[DataMap]") {
    cse498::DataMap data;

    SECTION("Store and retrieve a vector") {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        data.Set("numbers", vec);
        auto retrieved = data.Get<std::vector<int>>("numbers");
        REQUIRE(retrieved.has_value());
        REQUIRE(retrieved.value() == vec);
    }

    SECTION("Store and retrieve a pair") {
        std::pair<int, std::string> p = {42, "answer"};
        data.Set("pair", p);
        auto retrieved = data.Get<std::pair<int, std::string>>("pair");
        REQUIRE(retrieved.has_value());
        REQUIRE(retrieved.value().first == 42);
        REQUIRE(retrieved.value().second == "answer");
    }
}

TEST_CASE("DataMap can overwrite with different type", "[DataMap]") {
    cse498::DataMap data;
    data.Set("value", 42);
    data.Set("value", std::string("hello"));  // Different type
    auto result = data.Get<std::string>("value");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "hello");
}

TEST_CASE("DataMap handles empty strings", "[DataMap]") {
    cse498::DataMap data;
    data.Set("empty", std::string(""));
    auto result = data.Get<std::string>("empty");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "");
}

TEST_CASE("DataMap handles zero and negative numbers", "[DataMap]") {
    cse498::DataMap data;

    SECTION("Zero values") {
        data.Set("zero_int", 0);
        data.Set("zero_double", 0.0);
        REQUIRE(data.Get<int>("zero_int").value() == 0);
        REQUIRE(data.Get<double>("zero_double").value() == 0.0);
    }

    SECTION("Negative values") {
        data.Set("neg_int", -42);
        data.Set("neg_double", -3.14);
        REQUIRE(data.Get<int>("neg_int").value() == -42);
        REQUIRE(data.Get<double>("neg_double").value() == -3.14);
    }
}

TEST_CASE("DataMap handles special floating point values", "[DataMap]") {
    cse498::DataMap data;

    SECTION("Very large numbers") {
        data.Set("large", 1e308);
        REQUIRE(data.Get<double>("large").value() == 1e308);
    }

    SECTION("Very small numbers") {
        data.Set("small", 1e-308);
        REQUIRE(data.Get<double>("small").value() == 1e-308);
    }
}

TEST_CASE("DataMap handles special characters in keys", "[DataMap]") {
    cse498::DataMap data;
    data.Set("key_with_underscores", 1);
    data.Set("key-with-dashes", 2);
    data.Set("key.with.dots", 3);
    data.Set("key with spaces", 4);

    REQUIRE(data.Get<int>("key_with_underscores").value() == 1);
    REQUIRE(data.Get<int>("key-with-dashes").value() == 2);
    REQUIRE(data.Get<int>("key.with.dots").value() == 3);
    REQUIRE(data.Get<int>("key with spaces").value() == 4);
}

TEST_CASE("DataMap handles nested containers", "[DataMap]") {
    cse498::DataMap data;

    SECTION("Vector of vectors") {
        std::vector<std::vector<int>> nested = {{1, 2}, {3, 4, 5}};
        data.Set("nested", nested);
        auto result = data.Get<std::vector<std::vector<int>>>("nested");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == nested);
        REQUIRE(result.value()[0][0] == 1);
        REQUIRE(result.value()[1][2] == 5);
    }

    SECTION("Vector of pairs") {
        std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}};
        data.Set("vec_pairs", vec);
        auto result = data.Get<std::vector<std::pair<int, std::string>>>("vec_pairs");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == vec);
        REQUIRE(result.value()[0].second == "one");
    }
}

TEST_CASE("DataMap handles large strings", "[DataMap]") {
    cse498::DataMap data;
    std::string large(10000, 'a');
    data.Set("large_string", large);
    auto result = data.Get<std::string>("large_string");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == large);
    REQUIRE(result.value().length() == 10000);
}

TEST_CASE("DataMap handles unicode strings", "[DataMap]") {
    cse498::DataMap data;
    std::string unicode = "Hello 世界 🌍";
    data.Set("unicode", unicode);
    auto result = data.Get<std::string>("unicode");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == unicode);
}

TEST_CASE("DataMap stress test with alternating types", "[DataMap]") {
    cse498::DataMap data;

    data.Set("multi", 42);
    REQUIRE(data.Get<int>("multi").value() == 42);

    data.Set("multi", 3.14);
    REQUIRE(data.Get<double>("multi").value() == 3.14);

    data.Set("multi", std::string("text"));
    REQUIRE(data.Get<std::string>("multi").value() == "text");

    data.Set("multi", true);
    REQUIRE(data.Get<bool>("multi").value() == true);
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

    cse498::DataMap data;
    Player p1{"Hero", 10, 100.0};
    data.Set("player", p1);

    auto retrieved = data.Get<Player>("player");
    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved.value() == p1);
    REQUIRE(retrieved.value().name == "Hero");
    REQUIRE(retrieved.value().level == 10);
    REQUIRE(retrieved.value().health == 100.0);
}

// ============================================================================
// Error handling tests (now possible with std::expected)
// ============================================================================

TEST_CASE("DataMap Get returns error for non-existent key", "[DataMap]") {
    cse498::DataMap data;

    auto result = data.Get<int>("nonexistent");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Key not found");
}

TEST_CASE("DataMap Get returns error for type mismatch", "[DataMap]") {
    cse498::DataMap data;
    data.Set("value", 42);

    auto result = data.Get<double>("value");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Type mismatch for key");
}

TEST_CASE("DataMap Get error cases with various types", "[DataMap]") {
    cse498::DataMap data;
    data.Set("number", 42);
    data.Set("text", std::string("hello"));

    SECTION("int stored, string requested") {
        auto result = data.Get<std::string>("number");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("string stored, int requested") {
        auto result = data.Get<int>("text");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("key not found on non-empty map") {
        auto result = data.Get<int>("missing");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == "Key not found");
    }
}

// ============================================================================
// Iterator tests
// ============================================================================

TEST_CASE("DataMap iterator over empty map", "[DataMap][Iterator]") {
    cse498::DataMap data;
    REQUIRE(data.begin() == data.end());

    int count = 0;
    for (auto& [key, val] : data) {
        ++count;
        (void)key; // suppress unused variable warning
        (void)val; 
    }
    REQUIRE(count == 0);
}

TEST_CASE("DataMap iterator visits all elements", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);
    data.Set("c", 3);

    std::set<std::string> keys;
    int sum = 0;
    for (auto it = data.begin(); it != data.end(); ++it) {
        keys.insert(it.Key());
        sum += it.As<int>();
    }

    REQUIRE(keys.size() == 3);
    REQUIRE(keys.count("a") == 1);
    REQUIRE(keys.count("b") == 1);
    REQUIRE(keys.count("c") == 1);
    REQUIRE(sum == 6);
}

TEST_CASE("DataMap range-based for loop", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("x", 10);
    data.Set("y", 20);

    std::set<std::string> keys;
    for (auto& [key, val] : data) {
        keys.insert(key);
        (void)val; // suppress unused variable warning
    }

    REQUIRE(keys.size() == 2);
    REQUIRE(keys.count("x") == 1);
    REQUIRE(keys.count("y") == 1);
}

TEST_CASE("DataMap iterator Key() and Value() methods", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("name", std::string("Alice"));

    auto it = data.begin();
    REQUIRE(it.Key() == "name");
    REQUIRE(it.As<std::string>() == "Alice");
}

TEST_CASE("DataMap iterator As() with different types", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("intVal", 42);
    data.Set("doubleVal", 3.14);
    data.Set("strVal", std::string("hello"));

    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.Key() == "intVal") {
            REQUIRE(it.As<int>() == 42);
        } else if (it.Key() == "doubleVal") {
            REQUIRE(it.As<double>() == 3.14);
        } else if (it.Key() == "strVal") {
            REQUIRE(it.As<std::string>() == "hello");
        }
    }
}

TEST_CASE("DataMap iterator AsRef() allows mutation", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("counter", 0);

    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.Key() == "counter") {
            it.AsRef<int>() = 99;
        }
    }

    REQUIRE(data.Get<int>("counter").value() == 99);
}

TEST_CASE("DataMap iterator post-increment", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);

    auto it = data.begin();
    auto prev = it++;  // post-increment: prev points to first, it advances
    REQUIRE(prev != it);
    REQUIRE(it != data.end());  // should still have one more element
}

TEST_CASE("DataMap const iterator on const DataMap", "[DataMap][Iterator]") {
    cse498::DataMap mutable_data;
    mutable_data.Set("x", 10);
    mutable_data.Set("y", 20);
    mutable_data.Set("z", 30);

    const cse498::DataMap& data = mutable_data;

    std::set<std::string> keys;
    for (auto it = data.begin(); it != data.end(); ++it) {
        keys.insert(it.Key());
    }

    REQUIRE(keys.size() == 3);
    REQUIRE(keys.count("x") == 1);
    REQUIRE(keys.count("y") == 1);
    REQUIRE(keys.count("z") == 1);
}

TEST_CASE("DataMap cbegin/cend return const iterators", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("val", 42);

    auto it = data.cbegin();
    REQUIRE(it != data.cend());
    REQUIRE(it.Key() == "val");
    REQUIRE(it.As<int>() == 42);
}

TEST_CASE("DataMap iterator with Clear()", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);

    REQUIRE(data.begin() != data.end());

    data.Clear();

    REQUIRE(data.begin() == data.end());
}

// ============================================================================
// Utility method tests
// ============================================================================

TEST_CASE("DataMap Contains checks key existence", "[DataMap]") {
    cse498::DataMap data;
    data.Set("exists", 1);

    REQUIRE(data.Contains("exists") == true);
    REQUIRE(data.Contains("missing") == false);
}

TEST_CASE("DataMap IsEmpty and Size", "[DataMap]") {
    cse498::DataMap data;

    REQUIRE(data.IsEmpty() == true);
    REQUIRE(data.Size() == 0);

    data.Set("a", 1);
    REQUIRE(data.IsEmpty() == false);
    REQUIRE(data.Size() == 1);

    data.Set("b", 2);
    REQUIRE(data.Size() == 2);

    data.Clear();
    REQUIRE(data.IsEmpty() == true);
    REQUIRE(data.Size() == 0);
}

TEST_CASE("DataMap GetRef returns mutable reference", "[DataMap]") {
    cse498::DataMap data;
    data.Set("counter", 0);

    int& ref = data.GetRef<int>("counter");
    ref = 42;

    REQUIRE(data.Get<int>("counter").value() == 42);
}

TEST_CASE("DataMap GetRef with string type", "[DataMap]") {
    cse498::DataMap data;
    data.Set("greeting", std::string("hello"));

    std::string& ref = data.GetRef<std::string>("greeting");
    ref += " world";

    REQUIRE(data.Get<std::string>("greeting").value() == "hello world");
}

TEST_CASE("DataMap GetRef with vector type", "[DataMap]") {
    cse498::DataMap data;
    data.Set("nums", std::vector<int>{1, 2, 3});

    auto& ref = data.GetRef<std::vector<int>>("nums");
    ref.push_back(4);

    auto result = data.Get<std::vector<int>>("nums").value();
    REQUIRE(result.size() == 4);
    REQUIRE(result.back() == 4);
}

// ============================================================================
// Iterator operator* and operator-> tests
// ============================================================================

TEST_CASE("DataMap iterator operator* returns key-value pair", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("only", 77);

    auto it = data.begin();
    auto& pair = *it;
    REQUIRE(pair.first == "only");
    REQUIRE(std::any_cast<int>(pair.second) == 77);
}

TEST_CASE("DataMap iterator operator-> accesses pair members", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("key", std::string("value"));

    auto it = data.begin();
    REQUIRE(it->first == "key");
    REQUIRE(std::any_cast<std::string>(it->second) == "value");
}

TEST_CASE("DataMap iterator equality between positions", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);

    auto it1 = data.begin();
    auto it2 = data.begin();
    REQUIRE(it1 == it2);

    ++it1;
    REQUIRE(it1 != it2);

    ++it2;
    REQUIRE(it1 == it2);

    ++it1;
    ++it2;
    REQUIRE(it1 == data.end());
    REQUIRE(it2 == data.end());
    REQUIRE(it1 == it2);
}

// ============================================================================
// ConstIterator-specific tests
// ============================================================================

TEST_CASE("DataMap const iterator As() with different types", "[DataMap][Iterator]") {
    cse498::DataMap mutable_data;
    mutable_data.Set("num", 42);
    mutable_data.Set("pi", 3.14);
    mutable_data.Set("msg", std::string("hi"));

    const cse498::DataMap& data = mutable_data;

    for (auto it = data.begin(); it != data.end(); ++it) {
        if (it.Key() == "num") {
            REQUIRE(it.As<int>() == 42);
        } else if (it.Key() == "pi") {
            REQUIRE(it.As<double>() == 3.14);
        } else if (it.Key() == "msg") {
            REQUIRE(it.As<std::string>() == "hi");
        }
    }
}

TEST_CASE("DataMap const iterator operator-> works", "[DataMap][Iterator]") {
    cse498::DataMap mutable_data;
    mutable_data.Set("key", 99);

    const cse498::DataMap& data = mutable_data;

    auto it = data.begin();
    REQUIRE(it->first == "key");
    REQUIRE(std::any_cast<int>(it->second) == 99);
}

TEST_CASE("DataMap const iterator post-increment", "[DataMap][Iterator]") {
    cse498::DataMap mutable_data;
    mutable_data.Set("a", 1);
    mutable_data.Set("b", 2);

    const cse498::DataMap& data = mutable_data;

    auto it = data.begin();
    auto prev = it++;
    REQUIRE(prev != it);
    REQUIRE(prev != data.end());
    REQUIRE(it != data.end());

    ++it;
    REQUIRE(it == data.end());
}

TEST_CASE("DataMap const iterator Value() returns const ref", "[DataMap][Iterator]") {
    cse498::DataMap mutable_data;
    mutable_data.Set("val", 123);

    const cse498::DataMap& data = mutable_data;

    auto it = data.begin();
    const std::any& val = it.Value();
    REQUIRE(std::any_cast<int>(val) == 123);
}

// ============================================================================
// Iterator mutation via AsRef - multiple elements
// ============================================================================

TEST_CASE("DataMap iterator AsRef mutates multiple elements", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);
    data.Set("c", 3);

    // Double every value via iterator
    for (auto it = data.begin(); it != data.end(); ++it) {
        it.AsRef<int>() *= 2;
    }

    REQUIRE(data.Get<int>("a").value() == 2);
    REQUIRE(data.Get<int>("b").value() == 4);
    REQUIRE(data.Get<int>("c").value() == 6);
}

TEST_CASE("DataMap iterator Value() allows mutation via any_cast", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("x", 10);

    auto it = data.begin();
    std::any& val = it.Value();
    val = 999;

    REQUIRE(data.Get<int>("x").value() == 999);
}

// ============================================================================
// Iterator with map modifications
// ============================================================================

TEST_CASE("DataMap iterator reflects Size()", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("a", 1);
    data.Set("b", 2);
    data.Set("c", 3);

    int count = 0;
    for (auto it = data.begin(); it != data.end(); ++it) {
        ++count;
    }
    REQUIRE(count == data.Size());
}

TEST_CASE("DataMap iterator on single-element map", "[DataMap][Iterator]") {
    cse498::DataMap data;
    data.Set("solo", std::string("only one"));

    auto it = data.begin();
    REQUIRE(it != data.end());
    REQUIRE(it.Key() == "solo");
    REQUIRE(it.As<std::string>() == "only one");

    ++it;
    REQUIRE(it == data.end());
}
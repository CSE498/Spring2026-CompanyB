/**
 * @file RobinHoodMapTest.cpp
 * @brief Test suite for RobinHoodMap.
 * @author John Stouffer
 * @date 2026-2-2
 */

/**
 * This test suite contains code writeen by Claude Opus 4.5
 * 
 * Prompt: 
 * "[Attached RobinHoodMap.cpp, RobinHoodMap.hpp, and RobinHoodMapTest.cpp]
 * Please write my more tests for my RobinHoodMap class in C++. Test any 
 * edges cases you may think of and let me know what those are and why we should
 * test them. Additionally use the gtest framework as I have already used below."
 * 
 * @date 2026-2-2
 * @author Claude Opus 4.5
 */

// #include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <string>
#include <climits>
#include <type_traits>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <array>
#include <unordered_set>

#include "../../source/tools/RobinHoodMap.hpp"

using cse498::RobinHoodMap;

// ============================================================================
// COMPILE-TIME ASSERTIONS
// ============================================================================

// Verify RobinHoodMap meets basic type requirements
static_assert(std::is_default_constructible<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be default constructible");

static_assert(std::is_copy_constructible<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be copy constructible");

static_assert(std::is_copy_assignable<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be copy assignable");

static_assert(std::is_move_constructible<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be move constructible");

static_assert(std::is_move_assignable<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be move assignable");

static_assert(std::is_destructible<RobinHoodMap<int, int>>::value,
              "RobinHoodMap should be destructible");

static_assert(std::is_copy_constructible<RobinHoodMap<int, int>::Iterator>::value,
              "Iterator should be copy constructible");

static_assert(std::is_copy_assignable<RobinHoodMap<int, int>::Iterator>::value,
              "Iterator should be copy assignable");

static_assert(std::is_default_constructible<RobinHoodMap<std::string, double>>::value,
              "RobinHoodMap should work with string keys and double values");

static_assert(std::is_default_constructible<RobinHoodMap<char, std::string>>::value,
              "RobinHoodMap should work with char keys and string values");

// helper class to be able to access private functions and members
class RobinHoodMapTest {
public:
    template<typename K, typename V>
    static void callResize(RobinHoodMap<K, V>& map) { map._resize(); }
        
    template<typename K, typename V>
    static void callInsertWithHash(RobinHoodMap<K, V>& map, K key, V value, size_t hash) {
        map._insertWithHash(std::move(key), std::move(value), hash);
    }
    
    template<typename K, typename V>
    static size_t getTableSize(const RobinHoodMap<K, V>& map) { return map.mTable.size(); }
    
    template<typename K, typename V>
    static size_t getInternalSize(const RobinHoodMap<K, V>& map) { return map.mSize; }
    
    template<typename K, typename V>
    static size_t getStoredHash(const RobinHoodMap<K, V>& map, size_t index) {
        return map.mTable[index].hash;
    }
    
    template<typename K, typename V>
    static bool isSlotFilled(const RobinHoodMap<K, V>& map, size_t index) {
        return map.mTable[index].filled;
    }

    template<typename K, typename V>
    static size_t getVectorSize(const RobinHoodMap<K, V>& map) {
        return map.mTable.size();
    }

    static void addMoreItems(RobinHoodMap<int, int>& map, const int& start, const int& end) {
        for (int i = start; i < end; ++i) {
            map.insert(i, i * 100);
        }
    }

    static double loadFactor(const RobinHoodMap<int, int>& map) {
        return map.RHM_LOAD_FACTOR;
    }
};

// constructor

TEST_CASE("Constructor creates empty map", "[constructor]") {
    RobinHoodMap<int, int> map;
    REQUIRE(map.size() == 0u);
}

TEST_CASE("Constructor works with string types", "[constructor]") {
    RobinHoodMap<std::string, std::string> map;
    REQUIRE(map.size() == 0u);
}

// insert

TEST_CASE("Insert single and multiple elements", "[insert]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    REQUIRE(map.size() == 1u);
    
    map.insert(2, 200);
    map.insert(3, 300);
    REQUIRE(map.size() == 3u);
}

TEST_CASE("Insert duplicate key updates value", "[insert]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(1, 999);
    REQUIRE(map.size() == 1u);
    
    auto value = map.at(1);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 999);
}

TEST_CASE("Insert string keys and values", "[insert]") {
    RobinHoodMap<std::string, std::string> map;
    map.insert("hello", "world");
    
    auto value = map.at("hello");
    REQUIRE(value.has_value());
    REQUIRE(value.value() == "world");
}

TEST_CASE("Insert negative and zero keys", "[insert]") {
    RobinHoodMap<int, int> map;
    map.insert(-1, 100);
    map.insert(0, 200);
    REQUIRE(map.size() == 2u);
    
    auto value1 = map.at(-1);
    auto value2 = map.at(0);
    REQUIRE(value1.has_value());
    REQUIRE(value1.value() == 100);
    REQUIRE(value2.has_value());
    REQUIRE(value2.value() == 200);
}

// at

TEST_CASE("At existing and nonexistent keys", "[at]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    auto value1 = map.at(1);
    REQUIRE(value1.has_value());
    REQUIRE(value1.value() == 100);
    
    auto value2 = map.at(999);
    REQUIRE_FALSE(value2.has_value());
}

TEST_CASE("At empty map", "[at]") {
    RobinHoodMap<int, int> map;
    auto value = map.at(1);
    REQUIRE_FALSE(value.has_value());
}

// operator[]

TEST_CASE("Const subscript operator behaves like at") {
    RobinHoodMap<int, int> map;
    map.insert(5, 500);
    
    const RobinHoodMap<int, int>& constMap = map;
    auto value1 = constMap[5];
    REQUIRE(value1.has_value());
    REQUIRE(value1.value() == 500);
    
    auto value2 = constMap[999];
    REQUIRE_FALSE(value2.has_value());
}

// remove

TEST_CASE("Remove existing key", "[remove]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.remove(1);
    
    REQUIRE(map.size() == 1u);
    auto value = map.at(1);
    REQUIRE_FALSE(value.has_value());

}

TEST_CASE("Remove nonexistent key does nothing", "[remove]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.remove(999);
    REQUIRE(map.size() == 1u);
}

TEST_CASE("Remove from empty map", "[remove]") {
    RobinHoodMap<int, int> map;
    map.remove(1);
    REQUIRE(map.size() == 0u);
}

TEST_CASE("Remove then reinsert", "[remove]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.remove(1);
    map.insert(1, 999);
    
    auto value = map.at(1);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 999);
}

// assignment operator

TEST_CASE("Assignment operator creates independent copy", "[assignment]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> map2;
    map2 = map1;
    
    REQUIRE(map2.size() == 2u);
    
    // Modify original, copy should be unchanged
    map1.insert(1, 999);
    auto value = map2.at(1);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 100);
}

// verifying resize values using the map's load factor

TEST_CASE("Checking default resizing does get called", "[resize]") {
    RobinHoodMap<int, int> map;
    double lf = RobinHoodMapTest::loadFactor(map);
    int nextKey = 1;

    // Test resize across multiple capacity doublings (8 → 16 → ... → 4096)
    for (int doublings = 0; doublings < 10; ++doublings) {
        size_t capacity = RobinHoodMapTest::getVectorSize(map);
        size_t threshold = static_cast<size_t>(capacity * lf);

        // Fill up to one below the threshold
        RobinHoodMapTest::addMoreItems(map, nextKey, static_cast<int>(threshold));
        nextKey = static_cast<int>(threshold);

        // Should still be at the same capacity
        REQUIRE(RobinHoodMapTest::getVectorSize(map) == capacity);

        // Insert the element that triggers resize
        map.insert(nextKey, nextKey * 100);
        ++nextKey;

        // Capacity should have doubled
        REQUIRE(map.size() == threshold);
        REQUIRE(RobinHoodMapTest::getVectorSize(map) == capacity * 2);
    }
}

// resizing (private)

TEST_CASE("Resize doubles table size", "[resize]") {
    RobinHoodMap<int, int> map;
    size_t initialSize = RobinHoodMapTest::getTableSize(map);
    
    RobinHoodMapTest::callResize(map);
    
    REQUIRE(RobinHoodMapTest::getTableSize(map) == initialSize * 2);
}

TEST_CASE("Resize preserves elements", "[resize]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    RobinHoodMapTest::callResize(map);
    
    REQUIRE(map.size() == 2u);
    auto value1 = map.at(1);
    auto value2 = map.at(2);
    REQUIRE(value1.has_value());
    REQUIRE(value2.has_value());
}

// internal state

TEST_CASE("Internal state initial table size", "[internal]") {
    RobinHoodMap<int, int> map;
    REQUIRE(RobinHoodMapTest::getTableSize(map) == 8u);
}

TEST_CASE("Internal state size consistency", "[internal]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    REQUIRE(RobinHoodMapTest::getInternalSize(map) == map.size());
}

// ============================================================================
// AI GENERATED TESTS (Claude Opus 4.5)
// ============================================================================

// Stress Tests

TEST_CASE("Stress test many insertions", "[stress]") {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 100000; ++i) {
        map.insert(i, i * 10);
    }
    REQUIRE(map.size() == 100000u);
    
    auto value = map.at(500);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 5000);
}

TEST_CASE("Stress test insert and remove", "[stress]") {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 100; ++i) {
        map.insert(i, i);
    }
    for (int i = 0; i < 50; ++i) {
        map.remove(i);
    }
    
    REQUIRE(map.size() == 50u);
    
    auto value1 = map.at(25);
    auto value2 = map.at(75);
    REQUIRE_FALSE(value1.has_value());
    REQUIRE(value2.has_value());
    REQUIRE(value2.value() == 75);
}

// Collision Handling

TEST_CASE("Collisions colliding keys work", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    REQUIRE(map.size() == 3u);
    
    auto value0 = map.at(0);
    auto value8 = map.at(8);
    auto value16 = map.at(16);
    
    REQUIRE(value0.has_value());
    REQUIRE(value8.has_value());
    REQUIRE(value16.has_value());
    REQUIRE(value0.value() == 100);
    REQUIRE(value8.value() == 200);
    REQUIRE(value16.value() == 300);
}

TEST_CASE("Collisions remove middle colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(8);
    
    auto value0 = map.at(0);
    auto value8 = map.at(8);
    auto value16 = map.at(16);
    
    REQUIRE(map.size() == 2u);
    REQUIRE(value0.has_value());
    REQUIRE_FALSE(value8.has_value());
    REQUIRE(value16.has_value());
    REQUIRE(value0.value() == 100);
    REQUIRE(value16.value() == 300);
}

TEST_CASE("Collisions remove first colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(0);
    
    auto value0 = map.at(0);
    auto value8 = map.at(8);
    auto value16 = map.at(16);
    
    REQUIRE(map.size() == 2u);
    REQUIRE_FALSE(value0.has_value());
    REQUIRE(value8.has_value());
    REQUIRE(value16.has_value());
    REQUIRE(value8.value() == 200);
    REQUIRE(value16.value() == 300);
}

TEST_CASE("Collisions remove last colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(16);
    
    auto value0 = map.at(0);
    auto value8 = map.at(8);
    auto value16 = map.at(16);
    
    REQUIRE(value0.has_value());
    REQUIRE(value8.has_value());
    REQUIRE_FALSE(value16.has_value());
    REQUIRE(value0.value() == 100);
    REQUIRE(value8.value() == 200);
}

TEST_CASE("Collisions many colliding keys", "[collisions]") {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 50; ++i) {
        map.insert(i * 8, i);
    }
    
    REQUIRE(map.size() == 50u);
    
    for (int i = 0; i < 50; ++i) {
        auto value = map.at(i * 8);
        REQUIRE(value.has_value());
        REQUIRE(value.value() == i);
    }
}

// Boundary Values

TEST_CASE("Boundary values int min/max", "[boundary]") {
    RobinHoodMap<int, int> map;
    map.insert(INT_MAX, 100);
    map.insert(INT_MIN, 200);
    
    auto value1 = map.at(INT_MAX);
    auto value2 = map.at(INT_MIN);
    
    REQUIRE(value1.has_value());
    REQUIRE(value2.has_value());
    REQUIRE(value1.value() == 100);
    REQUIRE(value2.value() == 200);
}

TEST_CASE("Boundary values size_t max", "[boundary]") {
    RobinHoodMap<size_t, int> map;
    map.insert(SIZE_MAX, 42);
    
    auto value = map.at(SIZE_MAX);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 42);
}

// String Edge Cases

TEST_CASE("String keys empty and long strings", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("", 1);
    map.insert(std::string(1000, 'x'), 2);
    
    auto value1 = map.at("");
    auto value2 = map.at(std::string(1000, 'x'));
    
    REQUIRE(value1.has_value());
    REQUIRE(value2.has_value());
    REQUIRE(value1.value() == 1);
    REQUIRE(value2.value() == 2);
}

TEST_CASE("String keys case sensitive", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("test", 1);
    map.insert("TEST", 2);
    
    REQUIRE(map.size() == 2u);
    
    auto value1 = map.at("test");
    auto value2 = map.at("TEST");
    REQUIRE(value1.value() == 1);
    REQUIRE(value2.value() == 2);
}

TEST_CASE("String keys similar strings", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("aa", 2);
    map.insert("aaa", 3);
    map.insert("ab", 4);
    map.insert("ba", 5);
    
    REQUIRE(map.size() == 5u);
    
    auto value1 = map.at("a");
    auto value2 = map.at("aa");
    auto value3 = map.at("ab");
    
    REQUIRE(value1.value() == 1);
    REQUIRE(value2.value() == 2);
    REQUIRE(value3.value() == 4);
}

TEST_CASE("String keys whitespace keys", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert(" ", 1);
    map.insert("  ", 2);
    map.insert("\t", 3);
    map.insert("\n", 4);
    
    REQUIRE(map.size() == 4u);
}

// Iterator Tests

TEST_CASE("Iterator basic iteration", "[iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        ++count;
    }
    
    REQUIRE(count == 3);
}

TEST_CASE("Iterator empty map iteration", "[iterator]") {
    RobinHoodMap<int, int> map;
    
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        ++count;
    }
    
    REQUIRE(count == 0);
}

TEST_CASE("Iterator range based for", "[iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    int sum = 0;
    for (auto& entry : map) {
        sum += entry.value;
    }
    
    REQUIRE(sum == 300);
}

// Repeated Operations

TEST_CASE("Repeated ops insert same key many times", "[repeated]") {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 100; ++i) {
        map.insert(42, i);
    }
    
    REQUIRE(map.size() == 1u);
    auto value = map.at(42);
    REQUIRE(value.value() == 99);
}

TEST_CASE("Repeated ops remove same key many times", "[repeated]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    for (int i = 0; i < 100; ++i) {
        map.remove(1);
    }
    
    REQUIRE(map.size() == 0u);
}

TEST_CASE("Repeated ops insert remove cycle", "[repeated]") {
    RobinHoodMap<int, int> map;
    
    for (int cycle = 0; cycle < 10; ++cycle) {
        for (int i = 0; i < 50; ++i) {
            map.insert(i, i + cycle);
        }
        REQUIRE(map.size() == 50u);
        
        for (int i = 0; i < 50; ++i) {
            map.remove(i);
        }
        REQUIRE(map.size() == 0u);
    }
}

// Type Variations

TEST_CASE("Type variations char key", "[types]") {
    RobinHoodMap<char, int> map;
    
    for (char c = 'a'; c <= 'z'; ++c) {
        map.insert(c, c - 'a');
    }
    
    REQUIRE(map.size() == 26u);
    
    auto value = map.at('m');
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 12);
}

TEST_CASE("Type variations bool key", "[types]") {
    RobinHoodMap<bool, std::string> map;
    map.insert(true, "yes");
    map.insert(false, "no");
    
    REQUIRE(map.size() == 2u);
    
    auto value1 = map.at(true);
    auto value2 = map.at(false);
    REQUIRE(value1.value() == "yes");
    REQUIRE(value2.value() == "no");
}

TEST_CASE("Type variations double value", "[types]") {
    RobinHoodMap<int, double> map;
    map.insert(1, 3.14159);
    map.insert(2, 2.71828);
    
    auto value1 = map.at(1);
    auto value2 = map.at(2);
    
    REQUIRE_THAT(value1.value(), Catch::Matchers::WithinRel(3.14159));
    REQUIRE_THAT(value2.value(), Catch::Matchers::WithinRel(2.71828));
}

// Resize Behavior

TEST_CASE("Resize behavior auto resize preserves data", "[resize]") {
    RobinHoodMap<int, int> map;
    size_t initialTableSize = RobinHoodMapTest::getTableSize(map);
    
    for (int i = 0; i < 1000; ++i) {
        map.insert(i, i * 2);
    }
    
    REQUIRE(RobinHoodMapTest::getTableSize(map) > initialTableSize);
    
    for (int i = 0; i < 1000; ++i) {
        auto value = map.at(i);
        REQUIRE(value.has_value());
        REQUIRE(value.value() == i * 2);
    }
}

TEST_CASE("Resize behavior multiple manual resizes", "[resize]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    REQUIRE(map.size() == 2u);
    auto value1 = map.at(1);
    auto value2 = map.at(2);
    REQUIRE(value1.has_value());
    REQUIRE(value2.has_value());
}

// Assignment Edge Cases

TEST_CASE("Assignment assign empty to full", "[assignment]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> emptyMap;
    map1 = emptyMap;
    
    REQUIRE(map1.size() == 0u);
}

TEST_CASE("Assignment assign full to empty", "[assignment]") {
    RobinHoodMap<int, int> emptyMap;
    
    RobinHoodMap<int, int> fullMap;
    fullMap.insert(1, 100);
    
    emptyMap = fullMap;
    
    REQUIRE(emptyMap.size() == 1u);
}

// Backshift Verification (Robin Hood specific)

TEST_CASE("Backshift removal triggers backshift", "[backshift]") {
    RobinHoodMap<int, int> map;
    
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    map.insert(24, 400);
    
    map.remove(8);
    map.remove(16);
    
    auto value0 = map.at(0);
    auto value24 = map.at(24);
    
    REQUIRE(map.size() == 2u);
    REQUIRE(value0.has_value());
    REQUIRE(value24.has_value());
    REQUIRE(value0.value() == 100);
    REQUIRE(value24.value() == 400);
}

// Hash Stored Optimization Verification

TEST_CASE("Hash optimization stored hash works correctly", "[hash]") {
    RobinHoodMap<std::string, int> map;
    
    // Long strings where hash comparison saves time
    std::string key1(100, 'a');
    std::string key2(100, 'b');
    std::string key3(100, 'c');
    
    map.insert(key1, 1);
    map.insert(key2, 2);
    map.insert(key3, 3);
    
    auto value1 = map.at(key1);
    auto value2 = map.at(key2);
    auto value3 = map.at(key3);
    
    REQUIRE(value1.has_value());
    REQUIRE(value2.has_value());
    REQUIRE(value3.has_value());
    REQUIRE(value1.value() == 1);
    REQUIRE(value2.value() == 2);
    REQUIRE(value3.value() == 3);
}

// _insertWithHash Tests (private method)

TEST_CASE("InsertWithHash basic insert", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(42);
    RobinHoodMapTest::callInsertWithHash(map, 42, 100, hash);
    
    REQUIRE(map.size() == 1u);
    auto value = map.at(42);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 100);
}

TEST_CASE("InsertWithHash preserves provided hash", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    int key = 42;
    size_t expectedHash = hasher(key);
    RobinHoodMapTest::callInsertWithHash(map, key, 100, expectedHash);
    
    // Find where the element was stored
    size_t mask = RobinHoodMapTest::getTableSize(map) - 1;
    size_t index = expectedHash & mask;
    
    // Search for filled slot
    while (!RobinHoodMapTest::isSlotFilled(map, index)) {
        index = (index + 1) & mask;
    }
    
    REQUIRE(RobinHoodMapTest::getStoredHash(map, index) == expectedHash);
}

TEST_CASE("InsertWithHash multiple inserts", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    // Resize first so we have room (table size 8 -> 16 -> 32)
    // _insertWithHash doesn't trigger resize, so we need enough space
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    for (int i = 0; i < 10; ++i) {
        RobinHoodMapTest::callInsertWithHash(map, i, i * 100, hasher(i));
    }
    
    REQUIRE(map.size() == 10u);
    
    for (int i = 0; i < 10; ++i) {
        auto value = map.at(i);
        REQUIRE(value.has_value());
        REQUIRE(value.value() == i * 100);
    }
}

TEST_CASE("InsertWithHash duplicate key updates", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(5);
    RobinHoodMapTest::callInsertWithHash(map, 5, 100, hash);
    RobinHoodMapTest::callInsertWithHash(map, 5, 999, hash);
    
    REQUIRE(map.size() == 1u);
    auto value = map.at(5);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 999);
}

TEST_CASE("InsertWithHash handles collisions", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    // Resize to ensure we have room for colliding keys
    RobinHoodMapTest::callResize(map);
    
    // Insert keys that will collide (multiples of 8)
    RobinHoodMapTest::callInsertWithHash(map, 0, 100, hasher(0));
    RobinHoodMapTest::callInsertWithHash(map, 8, 200, hasher(8));
    RobinHoodMapTest::callInsertWithHash(map, 16, 300, hasher(16));
    
    REQUIRE(map.size() == 3u);
    
    auto value0 = map.at(0);
    auto value8 = map.at(8);
    auto value16 = map.at(16);
    
    REQUIRE(value0.has_value());
    REQUIRE(value8.has_value());
    REQUIRE(value16.has_value());
    REQUIRE(value0.value() == 100);
    REQUIRE(value8.value() == 200);
    REQUIRE(value16.value() == 300);
}

TEST_CASE("InsertWithHash works with strings", "[insertWithHash]") {
    RobinHoodMap<std::string, int> map;
    std::hash<std::string> hasher;
    
    std::string key = "test_key";
    size_t hash = hasher(key);
    RobinHoodMapTest::callInsertWithHash(map, key, 42, hash);
    
    auto value = map.at("test_key");
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 42);
}

TEST_CASE("InsertWithHash used by resize correctly", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    
    // Insert elements
    for (int i = 0; i < 5; ++i) {
        map.insert(i, i * 10);
    }
    
    // Force resize (which uses _insertWithHash internally)
    RobinHoodMapTest::callResize(map);
    
    // Verify all elements still accessible with correct values
    for (int i = 0; i < 5; ++i) {
        auto value = map.at(i);
        REQUIRE(value.has_value());
        REQUIRE(value.value() == i * 10);
    }
}

TEST_CASE("InsertWithHash hash preserved after resize", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    
    int key = 42;
    map.insert(key, 100);
    
    // Force multiple resizes
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    // Element should still be findable (hash was preserved during resize)
    auto value = map.at(key);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 100);
}

// ============================================================================
// BENCHMARKS
// ============================================================================

#include <chrono>
#include <random>
#include <unordered_map>
#include <iostream>

TEST_CASE("Benchmark", "[benchmark]") {
    const int N = 100000;
    std::vector<int> randomKeys(N);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dist(0, N * 10);
    for (int i = 0; i < N; ++i) {
        randomKeys[i] = dist(gen);
    }

    std::cout << "\n=== RobinHoodMap vs std::unordered_map ===\n\n";

    // Sequential Insert
    {
        auto start = std::chrono::high_resolution_clock::now();
        RobinHoodMap<int, int> map;
        for (int i = 0; i < N; ++i) map.insert(i, i);
        auto end = std::chrono::high_resolution_clock::now();
        double rh = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        std::unordered_map<int, int> umap;
        for (int i = 0; i < N; ++i) umap[i] = i;
        end = std::chrono::high_resolution_clock::now();
        double um = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Sequential Insert:  RH=" << rh << "ms  UM=" << um << "ms\n";
    }

    // Random Insert
    {
        auto start = std::chrono::high_resolution_clock::now();
        RobinHoodMap<int, int> map;
        for (int key : randomKeys) map.insert(key, key);
        auto end = std::chrono::high_resolution_clock::now();
        double rh = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        std::unordered_map<int, int> umap;
        for (int key : randomKeys) umap[key] = key;
        end = std::chrono::high_resolution_clock::now();
        double um = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Random Insert:      RH=" << rh << "ms  UM=" << um << "ms\n";
    }

    // Lookup Hit
    {
        RobinHoodMap<int, int> map;
        std::unordered_map<int, int> umap;
        for (int i = 0; i < N; ++i) { map.insert(i, i); umap[i] = i; }

        volatile int sink = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto r = map.at(i);
            if (r) sink = *r;
        }
        auto end = std::chrono::high_resolution_clock::now();
        double rh = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto it = umap.find(i);
            if (it != umap.end()) sink = it->second;
        }
        end = std::chrono::high_resolution_clock::now();
        double um = std::chrono::duration<double, std::milli>(end - start).count();
        (void)sink;  // Suppress unused variable warning

        std::cout << "Lookup Hit:         RH=" << rh << "ms  UM=" << um << "ms\n";
    }

    // Lookup Miss
    {
        RobinHoodMap<int, int> map;
        std::unordered_map<int, int> umap;
        for (int i = 0; i < N; ++i) { map.insert(i * 2, i); umap[i * 2] = i; }

        volatile int sink = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto r = map.at(i * 2 + 1);
            if (r) sink = *r;
        }
        auto end = std::chrono::high_resolution_clock::now();
        double rh = std::chrono::duration<double, std::milli>(end - start).count();
        (void)sink;  // weird unused warning, just supressing it

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto it = umap.find(i * 2 + 1);
            if (it != umap.end()) sink = it->second;
        }
        end = std::chrono::high_resolution_clock::now();
        double um = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Lookup Miss:        RH=" << rh << "ms  UM=" << um << "ms\n";
    }

    // Remove
    {
        auto start = std::chrono::high_resolution_clock::now();
        RobinHoodMap<int, int> map;
        for (int i = 0; i < N; ++i) map.insert(i, i);
        for (int i = 0; i < N; ++i) map.remove(i);
        auto end = std::chrono::high_resolution_clock::now();
        double rh = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        std::unordered_map<int, int> umap;
        for (int i = 0; i < N; ++i) umap[i] = i;
        for (int i = 0; i < N; ++i) umap.erase(i);
        end = std::chrono::high_resolution_clock::now();
        double um = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "Insert+Remove:      RH=" << rh << "ms  UM=" << um << "ms\n";
    }

    std::cout << "\n";
    REQUIRE(true);
}

// ============================================================================
// NEW FEATURE TESTS
// ============================================================================

// clear() Tests

TEST_CASE("Clear empty map", "[clear]") {
    RobinHoodMap<int, int> map;
    map.clear();
    REQUIRE(map.size() == 0u);
    REQUIRE(map.empty());
}

TEST_CASE("Clear map with elements", "[clear]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    REQUIRE(map.size() == 3u);
    
    map.clear();
    
    REQUIRE(map.size() == 0u);
    REQUIRE(map.empty());
    auto value = map.at(1);
    REQUIRE_FALSE(value.has_value());
}

TEST_CASE("Clear then reinsert", "[clear]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.clear();
    map.insert(1, 999);
    
    REQUIRE(map.size() == 1u);
    auto value = map.at(1);
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 999);
}

TEST_CASE("Clear multiple times", "[clear]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.clear();
    map.clear();
    map.clear();
    REQUIRE(map.size() == 0u);
    REQUIRE(map.empty());
}

// empty() Tests

TEST_CASE("Empty on new map", "[empty]") {
    RobinHoodMap<int, int> map;
    REQUIRE(map.empty());
}

TEST_CASE("Empty after insert", "[empty]") {
    RobinHoodMap<int, int> map;
    REQUIRE(map.empty());
    map.insert(1, 100);
    REQUIRE_FALSE(map.empty());
}

TEST_CASE("Empty after remove", "[empty]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    REQUIRE_FALSE(map.empty());
    map.remove(1);
    REQUIRE(map.empty());
}

TEST_CASE("Empty with multiple elements", "[empty]") {
    RobinHoodMap<int, int> map;
    for (int i = 0; i < 10; ++i) {
        map.insert(i, i * 10);
    }
    REQUIRE_FALSE(map.empty());
    for (int i = 0; i < 10; ++i) {
        map.remove(i);
    }
    REQUIRE(map.empty());
}

// contains() Tests

TEST_CASE("Contains existing key", "[contains]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    REQUIRE(map.contains(1));
}

TEST_CASE("Contains nonexistent key", "[contains]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    REQUIRE_FALSE(map.contains(999));
}

TEST_CASE("Contains on empty map", "[contains]") {
    RobinHoodMap<int, int> map;
    REQUIRE_FALSE(map.contains(1));
}

TEST_CASE("Contains after remove", "[contains]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    REQUIRE(map.contains(1));
    map.remove(1);
    REQUIRE_FALSE(map.contains(1));
}

TEST_CASE("Contains with string keys", "[contains]") {
    RobinHoodMap<std::string, int> map;
    map.insert("hello", 1);
    map.insert("world", 2);
    REQUIRE(map.contains("hello"));
    REQUIRE(map.contains("world"));
    REQUIRE_FALSE(map.contains("test"));
}

TEST_CASE("Contains with colliding keys", "[contains]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    REQUIRE(map.contains(0));
    REQUIRE(map.contains(8));
    REQUIRE(map.contains(16));
    REQUIRE_FALSE(map.contains(24));
}

// Copy Constructor Tests

TEST_CASE("Copy constructor creates independent copy", "[copy-constructor]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> map2(map1);
    
    REQUIRE(map2.size() == 2u);
    REQUIRE(map2.at(1).value() == 100);
    REQUIRE(map2.at(2).value() == 200);
    
    // Modify original
    map1.insert(3, 300);
    REQUIRE(map1.size() == 3u);
    REQUIRE(map2.size() == 2u);
}

TEST_CASE("Copy constructor with empty map", "[copy-constructor]") {
    RobinHoodMap<int, int> map1;
    RobinHoodMap<int, int> map2(map1);
    REQUIRE(map2.empty());
    REQUIRE(map2.size() == 0u);
}

TEST_CASE("Copy constructor preserves all elements", "[copy-constructor]") {
    RobinHoodMap<std::string, int> map1;
    for (int i = 0; i < 50; ++i) {
        map1.insert("key" + std::to_string(i), i);
    }
    
    RobinHoodMap<std::string, int> map2(map1);
    
    REQUIRE(map2.size() == 50u);
    for (int i = 0; i < 50; ++i) {
        REQUIRE(map2.contains("key" + std::to_string(i)));
        REQUIRE(map2.at("key" + std::to_string(i)).value() == i);
    }
}

// Move Constructor Tests

TEST_CASE("Move constructor transfers ownership", "[move-constructor]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> map2(std::move(map1));
    
    REQUIRE(map2.size() == 2u);
    REQUIRE(map2.at(1).value() == 100);
    REQUIRE(map2.at(2).value() == 200);
    REQUIRE(map1.size() == 0u);
}

TEST_CASE("Move constructor with empty map", "[move-constructor]") {
    RobinHoodMap<int, int> map1;
    RobinHoodMap<int, int> map2(std::move(map1));
    REQUIRE(map2.empty());
}

TEST_CASE("Move constructor with many elements", "[move-constructor]") {
    RobinHoodMap<int, int> map1;
    for (int i = 0; i < 100; ++i) {
        map1.insert(i, i * 10);
    }
    
    RobinHoodMap<int, int> map2(std::move(map1));
    
    REQUIRE(map2.size() == 100u);
    REQUIRE(map1.size() == 0u);
    for (int i = 0; i < 100; ++i) {
        REQUIRE(map2.at(i).value() == i * 10);
    }
}

// Move Assignment Tests

TEST_CASE("Move assignment transfers ownership", "[move-assignment]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> map2;
    map2.insert(99, 999);
    
    map2 = std::move(map1);
    
    REQUIRE(map2.size() == 2u);
    REQUIRE(map2.at(1).value() == 100);
    REQUIRE(map2.at(2).value() == 200);
    REQUIRE_FALSE(map2.contains(99));
    REQUIRE(map1.size() == 0u);
}

TEST_CASE("Move assignment to empty map", "[move-assignment]") {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    
    RobinHoodMap<int, int> map2;
    map2 = std::move(map1);
    
    REQUIRE(map2.size() == 1u);
    REQUIRE(map1.size() == 0u);
}


// reserve() Tests

TEST_CASE("Reserve increases capacity", "[reserve]") {
    RobinHoodMap<int, int> map;
    size_t initialSize = RobinHoodMapTest::getTableSize(map);
    
    map.reserve(100);
    
    REQUIRE(RobinHoodMapTest::getTableSize(map) > initialSize);
}

TEST_CASE("Reserve preserves existing elements", "[reserve]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    map.reserve(100);
    
    REQUIRE(map.size() == 3u);
    REQUIRE(map.at(1).value() == 100);
    REQUIRE(map.at(2).value() == 200);
    REQUIRE(map.at(3).value() == 300);
}

TEST_CASE("Reserve with smaller capacity does nothing", "[reserve]") {
    RobinHoodMap<int, int> map;
    for (int i = 0; i < 50; ++i) {
        map.insert(i, i * 10);
    }
    size_t tableSize = RobinHoodMapTest::getTableSize(map);
    
    map.reserve(5);
    
    REQUIRE(RobinHoodMapTest::getTableSize(map) == tableSize);
}

TEST_CASE("Reserve on empty map", "[reserve]") {
    RobinHoodMap<int, int> map;
    map.reserve(100);
    REQUIRE(map.empty());
    REQUIRE(RobinHoodMapTest::getTableSize(map) >= 200);
}

TEST_CASE("Reserve prevents automatic resizes", "[reserve]") {
    RobinHoodMap<int, int> map;
    map.reserve(100);
    size_t tableSize = RobinHoodMapTest::getTableSize(map);
    
    for (int i = 0; i < 50; ++i) {
        map.insert(i, i * 10);
    }
    
    REQUIRE(RobinHoodMapTest::getTableSize(map) == tableSize);
}

// Non-const operator[] Tests

TEST_CASE("Non-const operator() <- square brackets inserts default value") {
    RobinHoodMap<int, int> map;
    int& value = map[1];
    value = 100;
    
    REQUIRE(map.size() == 1u);
    REQUIRE(map.at(1).value() == 100);
}

TEST_CASE("Non-const operator() <- square brackets returns existing value") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    int& value = map[1];
    REQUIRE(value == 100);
    
    value = 999;
    REQUIRE(map.at(1).value() == 999);
}

TEST_CASE("Non-const operator() <- square brackets with string values") {
    RobinHoodMap<std::string, std::string> map;
    map["hello"] = "world";
    
    REQUIRE(map.size() == 1u);
    REQUIRE(map.at("hello").value() == "world");
}

TEST_CASE("Non-const operator() <- square brackets modifies in place") {
    RobinHoodMap<int, int> map;
    map[1] = 100;
    map[1] = 200;
    map[1] = 300;
    
    REQUIRE(map.size() == 1u);
    REQUIRE(map.at(1).value() == 300);
}

TEST_CASE("Non-const operator() <- square brackets multiple keys") {
    RobinHoodMap<int, int> map;
    map[1] = 100;
    map[2] = 200;
    map[3] = 300;
    
    REQUIRE(map.size() == 3u);
    REQUIRE(map[1] == 100);
    REQUIRE(map[2] == 200);
    REQUIRE(map[3] == 300);
}

// Const Iterator Tests

TEST_CASE("Const iterator basic iteration", "[const-iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    const auto& constMap = map;
    int count = 0;
    for (auto it = constMap.begin(); it != constMap.end(); ++it) {
        ++count;
    }
    
    REQUIRE(count == 3);
}

TEST_CASE("Const iterator with cbegin/cend", "[const-iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    int count = 0;
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        ++count;
    }
    
    REQUIRE(count == 2);
}

TEST_CASE("Const iterator on empty map", "[const-iterator]") {
    RobinHoodMap<int, int> map;
    const auto& constMap = map;
    
    int count = 0;
    for (auto it = constMap.begin(); it != constMap.end(); ++it) {
        ++count;
    }
    
    REQUIRE(count == 0);
}

TEST_CASE("Const iterator range-based for", "[const-iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    const auto& constMap = map;
    int sum = 0;
    for (const auto& entry : constMap) {
        sum += entry.value;
    }
    
    REQUIRE(sum == 600);
}

TEST_CASE("Const iterator with cbegin on const map", "[const-iterator]") {
    RobinHoodMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("b", 2);
    map.insert("c", 3);
    
    const auto& constMap = map;
    int count = 0;
    for (auto it = constMap.cbegin(); it != constMap.cend(); ++it) {
        count++;
    }
    
    REQUIRE(count == 3);
}

TEST_CASE("Const iterator post-increment", "[const-iterator]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    const auto& constMap = map;
    auto it = constMap.begin();
    auto old_it = it++;
    
    REQUIRE(old_it != it);
}

// Combined Feature Tests

TEST_CASE("Combined: reserve, insert, contains", "[combined]") {
    RobinHoodMap<int, int> map;
    map.reserve(50);
    
    for (int i = 0; i < 20; ++i) {
        map.insert(i, i * 10);
    }
    
    REQUIRE(map.size() == 20u);
    
    for (int i = 0; i < 20; ++i) {
        REQUIRE(map.contains(i));
    }
    REQUIRE_FALSE(map.contains(100));
}

TEST_CASE("Combined: copy, modify, compare", "[combined]") {
    RobinHoodMap<int, int> map1;
    map1[1] = 100;
    map1[2] = 200;
    
    RobinHoodMap<int, int> map2(map1);
    
    map1[3] = 300;
    map2[4] = 400;
    
    REQUIRE(map1.contains(1));
    REQUIRE(map1.contains(2));
    REQUIRE(map1.contains(3));
    REQUIRE_FALSE(map1.contains(4));
    
    REQUIRE(map2.contains(1));
    REQUIRE(map2.contains(2));
    REQUIRE_FALSE(map2.contains(3));
    REQUIRE(map2.contains(4));
}

TEST_CASE("Combined: clear, empty, reserve", "[combined]") {
    RobinHoodMap<int, int> map;
    map.reserve(100);
    
    for (int i = 0; i < 50; ++i) {
        map[i] = i * 10;
    }
    
    REQUIRE_FALSE(map.empty());
    REQUIRE(map.size() == 50u);
    
    map.clear();
    
    REQUIRE(map.empty());
    REQUIRE(map.size() == 0u);
    
    // Can still use map after clear
    map[1] = 100;
    REQUIRE(map.contains(1));
}

TEST_CASE("Combined: move and contains", "[combined]") {
    RobinHoodMap<std::string, int> map1;
    map1.insert("a", 1);
    map1.insert("b", 2);
    map1.insert("c", 3);
    
    RobinHoodMap<std::string, int> map2(std::move(map1));
    
    REQUIRE(map2.contains("a"));
    REQUIRE(map2.contains("b"));
    REQUIRE(map2.contains("c"));
    REQUIRE(map1.empty());
}

// Testing latest changes

TEST_CASE("Iterator dereference on valid entry does not assert", "[iterator-assert]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    auto it = map.begin();
    auto& entry = *it;
    REQUIRE(entry.filled);
    REQUIRE((entry.key == 1 || entry.key == 2));
}

TEST_CASE("Iterator begin equals end on empty map", "[iterator-assert]") {
    RobinHoodMap<int, int> map;
    REQUIRE(map.begin() == map.end());
}

TEST_CASE("Iterator dereference after increment stays valid", "[iterator-assert]") {
    RobinHoodMap<int, int> map;
    map.insert(10, 1);
    map.insert(20, 2);
    map.insert(30, 3);
    
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        REQUIRE((*it).filled);
        ++count;
    }
    REQUIRE(count == 3);
}

TEST_CASE("Iterator pre-decrement skips unfilled entries", "[iterator-decrement]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    std::vector<int> forwardKeys;
    for (auto it = map.begin(); it != map.end(); ++it) {
        forwardKeys.push_back((*it).key);
    }
    
    auto it = map.begin();
    auto lastIt = it;
    while (it != map.end()) {
        lastIt = it;
        ++it;
    }
    
    std::vector<int> reverseKeys;
    reverseKeys.push_back((*lastIt).key);
    
    --lastIt;
    if ((*lastIt).filled) {
        reverseKeys.push_back((*lastIt).key);
    }
    
    for (auto& k : reverseKeys) {
        REQUIRE((k == 1 || k == 2 || k == 3));
    }
}

TEST_CASE("Iterator post-decrement returns old position", "[iterator-decrement]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    auto it = map.begin();
    auto first = it;
    ++it;
    
    auto old = it--;
    REQUIRE((*old).filled);
    REQUIRE((*it).filled);
    REQUIRE(it == first);
}

TEST_CASE("Iterator decrement visits only filled entries", "[iterator-decrement]") {
    RobinHoodMap<int, int> map;

    for (int i = 0; i < 20; ++i) {
        map.insert(i, i * 10);
    }

    for (int i = 0; i < 20; i += 2) {
        map.remove(i);
    }
    REQUIRE(map.size() == 10u);
    
    std::vector<int> forwardKeys;
    for (auto it = map.begin(); it != map.end(); ++it) {
        REQUIRE((*it).filled);
        forwardKeys.push_back((*it).key);
    }
    REQUIRE(forwardKeys.size() == 10u);
    
    auto it = map.begin();
    auto lastIt = it;
    while (it != map.end()) {
        lastIt = it;
        ++it;
    }
    
    std::vector<int> backwardKeys;
    backwardKeys.push_back((*lastIt).key);
    for (size_t i = 1; i < forwardKeys.size(); ++i) {
        --lastIt;
        REQUIRE((*lastIt).filled); 
        backwardKeys.push_back((*lastIt).key);
    }
    
    std::reverse(backwardKeys.begin(), backwardKeys.end());
    REQUIRE(forwardKeys == backwardKeys);
}

TEST_CASE("Iterator decrement on single element map", "[iterator-decrement]") {
    RobinHoodMap<int, int> map;
    map.insert(42, 999);
    
    auto it = map.begin();
    REQUIRE((*it).key == 42);
    REQUIRE((*it).value == 999);
    
    ++it;
    REQUIRE(it == map.end());
}

// ===========================================================================
// TESTING OTHER DATA STRUCTURES
// ===========================================================================

// --- Vector as value ---

TEST_CASE("Vector as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<std::string, std::vector<int>> map;
    map.insert("scores", {90, 85, 100, 72});

    auto result = map.at("scores");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 4u);
    REQUIRE(result.value()[0] == 90);
    REQUIRE(result.value()[3] == 72);
}

TEST_CASE("Vector as value - update overwrites entire vector", "[data-structures]") {
    RobinHoodMap<std::string, std::vector<int>> map;
    map.insert("data", {1, 2, 3});
    map.insert("data", {10, 20});

    auto result = map.at("data");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 2u);
    REQUIRE(result.value()[0] == 10);
    REQUIRE(result.value()[1] == 20);
}

TEST_CASE("Vector as value - empty vector", "[data-structures]") {
    RobinHoodMap<std::string, std::vector<int>> map;
    map.insert("empty", {});

    auto result = map.at("empty");
    REQUIRE(result.has_value());
    REQUIRE(result.value().empty());
}

TEST_CASE("Vector as value - multiple keys with vectors", "[data-structures]") {
    RobinHoodMap<int, std::vector<std::string>> map;
    map.insert(1, {"hello", "world"});
    map.insert(2, {"foo", "bar", "baz"});
    map.insert(3, {"single"});

    REQUIRE(map.size() == 3u);
    REQUIRE(map.at(1).value().size() == 2u);
    REQUIRE(map.at(2).value().size() == 3u);
    REQUIRE(map.at(3).value()[0] == "single");
}

// --- List as value ---

TEST_CASE("List as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<int, std::list<int>> map;
    map.insert(1, {10, 20, 30});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 3u);
    REQUIRE(result.value().front() == 10);
    REQUIRE(result.value().back() == 30);
}

TEST_CASE("List as value - remove key with list value", "[data-structures]") {
    RobinHoodMap<int, std::list<double>> map;
    map.insert(1, {1.1, 2.2, 3.3});
    map.insert(2, {4.4, 5.5});

    map.remove(1);
    REQUIRE(map.size() == 1u);
    REQUIRE_FALSE(map.at(1).has_value());
    REQUIRE(map.at(2).has_value());
}

// --- Set as value ---

TEST_CASE("Set as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<std::string, std::set<int>> map;
    map.insert("primes", {2, 3, 5, 7, 11});

    auto result = map.at("primes");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 5u);
    REQUIRE(result.value().count(5) == 1u);
    REQUIRE(result.value().count(4) == 0u);
}

TEST_CASE("Set as value - duplicates in initializer are deduplicated", "[data-structures]") {
    RobinHoodMap<int, std::set<std::string>> map;
    map.insert(1, {"a", "b", "a", "c", "b"});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 3u);
}

// --- Map as value (nested map) ---

TEST_CASE("Map as value - nested map insert and retrieve", "[data-structures]") {
    RobinHoodMap<std::string, std::map<std::string, int>> map;
    std::map<std::string, int> inner;
    inner["age"] = 25;
    inner["score"] = 100;
    map.insert("player1", inner);

    auto result = map.at("player1");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 2u);
    REQUIRE(result.value().at("age") == 25);
    REQUIRE(result.value().at("score") == 100);
}

TEST_CASE("Map as value - multiple nested maps", "[data-structures]") {
    RobinHoodMap<int, std::map<int, int>> map;
    map.insert(1, {{10, 100}, {20, 200}});
    map.insert(2, {{30, 300}});

    REQUIRE(map.size() == 2u);
    REQUIRE(map.at(1).value().size() == 2u);
    REQUIRE(map.at(2).value().at(30) == 300);
}

// --- Deque as value ---

TEST_CASE("Deque as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<int, std::deque<int>> map;
    map.insert(1, {1, 2, 3, 4, 5});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 5u);
    REQUIRE(result.value().front() == 1);
    REQUIRE(result.value().back() == 5);
}

TEST_CASE("Deque as value - update with different size deque", "[data-structures]") {
    RobinHoodMap<std::string, std::deque<std::string>> map;
    map.insert("items", {"a", "b"});
    map.insert("items", {"x", "y", "z"});

    auto result = map.at("items");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 3u);
    REQUIRE(result.value()[0] == "x");
}

// --- Array as value ---

TEST_CASE("Array as value - fixed size array", "[data-structures]") {
    RobinHoodMap<int, std::array<int, 3>> map;
    map.insert(1, {10, 20, 30});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value()[0] == 10);
    REQUIRE(result.value()[1] == 20);
    REQUIRE(result.value()[2] == 30);
}

TEST_CASE("Array as value - multiple entries", "[data-structures]") {
    RobinHoodMap<std::string, std::array<double, 2>> map;
    map.insert("point1", {1.5, 2.5});
    map.insert("point2", {3.0, 4.0});

    REQUIRE(map.size() == 2u);
    REQUIRE(map.at("point1").value()[0] == 1.5);
    REQUIRE(map.at("point2").value()[1] == 4.0);
}

// --- Pair as value ---

TEST_CASE("Pair as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<int, std::pair<std::string, int>> map;
    map.insert(1, {"hello", 42});
    map.insert(2, {"world", 99});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().first == "hello");
    REQUIRE(result.value().second == 42);
}

// --- Unordered set as value ---

TEST_CASE("Unordered set as value - insert and retrieve", "[data-structures]") {
    RobinHoodMap<int, std::unordered_set<int>> map;
    map.insert(1, {10, 20, 30, 40});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 4u);
    REQUIRE(result.value().count(20) == 1u);
    REQUIRE(result.value().count(50) == 0u);
}

// --- Vector of vectors (nested containers) ---

TEST_CASE("Vector of vectors as value", "[data-structures]") {
    RobinHoodMap<int, std::vector<std::vector<int>>> map;
    map.insert(1, {{1, 2}, {3, 4, 5}, {6}});

    auto result = map.at(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 3u);
    REQUIRE(result.value()[0].size() == 2u);
    REQUIRE(result.value()[1].size() == 3u);
    REQUIRE(result.value()[2][0] == 6);
}

// --- Cross-type interactions ---

TEST_CASE("Multiple data structure value types across maps", "[data-structures]") {
    RobinHoodMap<int, std::vector<int>> vecMap;
    RobinHoodMap<int, std::set<int>> setMap;
    RobinHoodMap<int, std::list<int>> listMap;

    for (int i = 0; i < 20; ++i) {
        vecMap.insert(i, {i, i * 2, i * 3});
        setMap.insert(i, {i, i * 2, i * 3});
        listMap.insert(i, {i, i * 2, i * 3});
    }

    REQUIRE(vecMap.size() == 20u);
    REQUIRE(setMap.size() == 20u);
    REQUIRE(listMap.size() == 20u);

    // All maps should be retrievable for every key
    for (int i = 0; i < 20; ++i) {
        REQUIRE(vecMap.at(i).has_value());
        REQUIRE(setMap.at(i).has_value());
        REQUIRE(listMap.at(i).has_value());
    }
}

TEST_CASE("Data structure values survive resize", "[data-structures]") {
    RobinHoodMap<int, std::vector<std::string>> map;

    // Insert enough to trigger multiple resizes
    for (int i = 0; i < 100; ++i) {
        map.insert(i, {"item_" + std::to_string(i), "extra"});
    }

    REQUIRE(map.size() == 100u);

    // Verify all values survived resizing intact
    for (int i = 0; i < 100; ++i) {
        auto result = map.at(i);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 2u);
        REQUIRE(result.value()[0] == "item_" + std::to_string(i));
        REQUIRE(result.value()[1] == "extra");
    }
}

TEST_CASE("Copy assignment with data structure values", "[data-structures]") {
    RobinHoodMap<int, std::vector<int>> map1;
    map1.insert(1, {10, 20, 30});
    map1.insert(2, {40, 50});

    RobinHoodMap<int, std::vector<int>> map2;
    map2 = map1;

    REQUIRE(map2.size() == 2u);
    REQUIRE(map2.at(1).value() == std::vector<int>({10, 20, 30}));

    // Modify original, copy should be independent
    map1.insert(1, {999});
    REQUIRE(map2.at(1).value().size() == 3u);
    REQUIRE(map2.at(1).value()[0] == 10);
}

TEST_CASE("Iterator with data structure values", "[data-structures]") {
    RobinHoodMap<int, std::vector<int>> map;
    map.insert(1, {10, 20});
    map.insert(2, {30, 40, 50});
    map.insert(3, {60});

    size_t totalElements = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        totalElements += (*it).value.size();
    }
    REQUIRE(totalElements == 6u);
}

TEST_CASE("Remove with data structure values", "[data-structures]") {
    RobinHoodMap<std::string, std::map<std::string, int>> map;
    map.insert("a", {{"x", 1}, {"y", 2}});
    map.insert("b", {{"z", 3}});
    map.insert("c", {{"w", 4}, {"v", 5}, {"u", 6}});

    map.remove("b");
    REQUIRE(map.size() == 2u);
    REQUIRE_FALSE(map.at("b").has_value());
    REQUIRE(map.at("a").value().size() == 2u);
    REQUIRE(map.at("c").value().size() == 3u);
}

TEST_CASE("Non-const operator() <- square brackets with vector value", "[data-structures]") {
    RobinHoodMap<std::string, std::vector<int>> map;

    // operator[] should default-insert an empty vector
    auto& vec = map["new_key"];
    REQUIRE(vec.empty());

    // Modify through the reference
    vec.push_back(42);
    vec.push_back(99);

    auto result = map.at("new_key");
    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 2u);
    REQUIRE(result.value()[0] == 42);
    REQUIRE(result.value()[1] == 99);
}

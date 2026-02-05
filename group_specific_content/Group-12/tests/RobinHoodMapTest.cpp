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

#define CATCH_CONFIG_MAIN
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include <string>
#include <climits>

#include "../RobinHoodMap.hpp"
#include "../RobinHoodMap.cpp"

// helper class to be able to acess private functions and members
class RobinHoodMapTest {
public:
    template<typename K, typename V>
    static void callResize(RobinHoodMap<K, V>& map) { map._resize(); }
    
    template<typename K, typename V>
    static auto callFindFirstElement(RobinHoodMap<K, V>& map) { return map._findFirstElement(); }
    
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
    
    auto [found, value] = map.at(1);
    REQUIRE(found);
    REQUIRE(value == 999);
}

TEST_CASE("Insert string keys and values", "[insert]") {
    RobinHoodMap<std::string, std::string> map;
    map.insert("hello", "world");
    
    auto [found, value] = map.at("hello");
    REQUIRE(found);
    REQUIRE(value == "world");
}

TEST_CASE("Insert negative and zero keys", "[insert]") {
    RobinHoodMap<int, int> map;
    map.insert(-1, 100);
    map.insert(0, 200);
    REQUIRE(map.size() == 2u);
    
    auto [f1, v1] = map.at(-1);
    auto [f2, v2] = map.at(0);
    REQUIRE(f1);
    REQUIRE(f2);
}

// at

TEST_CASE("At existing and nonexistent keys", "[at]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    auto [found1, value1] = map.at(1);
    REQUIRE(found1);
    REQUIRE(value1 == 100);
    
    auto [found2, value2] = map.at(999);
    REQUIRE_FALSE(found2);
}

TEST_CASE("At empty map", "[at]") {
    RobinHoodMap<int, int> map;
    auto [found, value] = map.at(1);
    REQUIRE_FALSE(found);
}

// operator[]

TEST_CASE("Subscript operator behaves like at", "[operator[]]") {
    RobinHoodMap<int, int> map;
    map.insert(5, 500);
    
    auto [found1, value1] = map[5];
    REQUIRE(found1);
    REQUIRE(value1 == 500);
    
    auto [found2, value2] = map[999];
    REQUIRE_FALSE(found2);
}

// remove

TEST_CASE("Remove existing key", "[remove]") {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.remove(1);
    
    REQUIRE(map.size() == 1u);
    auto [found, value] = map.at(1);
    REQUIRE_FALSE(found);
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
    
    auto [found, value] = map.at(1);
    REQUIRE(found);
    REQUIRE(value == 999);
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
    auto [found, value] = map2.at(1);
    REQUIRE(value == 100);
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
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    REQUIRE(f1);
    REQUIRE(f2);
}

// finding first element (private)

TEST_CASE("Find first element empty and non-empty", "[find-first]") {
    RobinHoodMap<int, int> map;
    auto [worked1, key1] = RobinHoodMapTest::callFindFirstElement(map);
    REQUIRE_FALSE(worked1);
    
    map.insert(42, 100);
    auto [worked2, key2] = RobinHoodMapTest::callFindFirstElement(map);
    REQUIRE(worked2);
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
    
    for (int i = 0; i < 1000; ++i) {
        map.insert(i, i * 10);
    }
    REQUIRE(map.size() == 1000u);
    
    auto [found, value] = map.at(500);
    REQUIRE(found);
    REQUIRE(value == 5000);
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
    
    auto [found1, v1] = map.at(25);
    auto [found2, v2] = map.at(75);
    REQUIRE_FALSE(found1);
    REQUIRE(found2);
}

// Collision Handling

TEST_CASE("Collisions colliding keys work", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    REQUIRE(map.size() == 3u);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    REQUIRE(f0);
    REQUIRE(f8);
    REQUIRE(f16);
}

TEST_CASE("Collisions remove middle colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(8);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    REQUIRE(f0);
    REQUIRE_FALSE(f8);
    REQUIRE(f16);
}

TEST_CASE("Collisions remove first colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(0);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    REQUIRE_FALSE(f0);
    REQUIRE(f8);
    REQUIRE(f16);
}

TEST_CASE("Collisions remove last colliding key", "[collisions]") {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(16);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    REQUIRE(f0);
    REQUIRE(f8);
    REQUIRE_FALSE(f16);
}

TEST_CASE("Collisions many colliding keys", "[collisions]") {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 50; ++i) {
        map.insert(i * 8, i);
    }
    
    REQUIRE(map.size() == 50u);
    
    for (int i = 0; i < 50; ++i) {
        auto [found, value] = map.at(i * 8);
        REQUIRE(found);
        REQUIRE(value == i);
    }
}

// Boundary Values

TEST_CASE("Boundary values int min/max", "[boundary]") {
    RobinHoodMap<int, int> map;
    map.insert(INT_MAX, 100);
    map.insert(INT_MIN, 200);
    
    auto [f1, v1] = map.at(INT_MAX);
    auto [f2, v2] = map.at(INT_MIN);
    
    REQUIRE(f1);
    REQUIRE(f2);
    REQUIRE(v1 == 100);
    REQUIRE(v2 == 200);
}

TEST_CASE("Boundary values size_t max", "[boundary]") {
    RobinHoodMap<size_t, int> map;
    map.insert(SIZE_MAX, 42);
    
    auto [found, value] = map.at(SIZE_MAX);
    REQUIRE(found);
    REQUIRE(value == 42);
}

// String Edge Cases

TEST_CASE("String keys empty and long strings", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("", 1);
    map.insert(std::string(1000, 'x'), 2);
    
    auto [f1, v1] = map.at("");
    auto [f2, v2] = map.at(std::string(1000, 'x'));
    
    REQUIRE(f1);
    REQUIRE(f2);
}

TEST_CASE("String keys case sensitive", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("test", 1);
    map.insert("TEST", 2);
    
    REQUIRE(map.size() == 2u);
    
    auto [f1, v1] = map.at("test");
    auto [f2, v2] = map.at("TEST");
    REQUIRE(v1 == 1);
    REQUIRE(v2 == 2);
}

TEST_CASE("String keys similar strings", "[string]") {
    RobinHoodMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("aa", 2);
    map.insert("aaa", 3);
    map.insert("ab", 4);
    map.insert("ba", 5);
    
    REQUIRE(map.size() == 5u);
    
    auto [f1, v1] = map.at("a");
    auto [f2, v2] = map.at("aa");
    auto [f3, v3] = map.at("ab");
    
    REQUIRE(v1 == 1);
    REQUIRE(v2 == 2);
    REQUIRE(v3 == 4);
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
    auto [found, value] = map.at(42);
    REQUIRE(value == 99);
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
    
    auto [found, value] = map.at('m');
    REQUIRE(found);
    REQUIRE(value == 12);
}

TEST_CASE("Type variations bool key", "[types]") {
    RobinHoodMap<bool, std::string> map;
    map.insert(true, "yes");
    map.insert(false, "no");
    
    REQUIRE(map.size() == 2u);
    
    auto [f1, v1] = map.at(true);
    auto [f2, v2] = map.at(false);
    REQUIRE(v1 == "yes");
    REQUIRE(v2 == "no");
}

TEST_CASE("Type variations double value", "[types]") {
    RobinHoodMap<int, double> map;
    map.insert(1, 3.14159);
    map.insert(2, 2.71828);
    
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    
    REQUIRE(v1 == Approx(3.14159));
    REQUIRE(v2 == Approx(2.71828));
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
        auto [found, value] = map.at(i);
        REQUIRE(found);
        REQUIRE(value == i * 2);
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
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    REQUIRE(f1);
    REQUIRE(f2);
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
    
    auto [f0, v0] = map.at(0);
    auto [f24, v24] = map.at(24);
    
    REQUIRE(f0);
    REQUIRE(f24);
    REQUIRE(v0 == 100);
    REQUIRE(v24 == 400);
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
    
    auto [f1, v1] = map.at(key1);
    auto [f2, v2] = map.at(key2);
    auto [f3, v3] = map.at(key3);
    
    REQUIRE(f1);
    REQUIRE(f2);
    REQUIRE(f3);
    REQUIRE(v1 == 1);
    REQUIRE(v2 == 2);
    REQUIRE(v3 == 3);
}

// _insertWithHash Tests (private method)

TEST_CASE("InsertWithHash basic insert", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(42);
    RobinHoodMapTest::callInsertWithHash(map, 42, 100, hash);
    
    REQUIRE(map.size() == 1u);
    auto [found, value] = map.at(42);
    REQUIRE(found);
    REQUIRE(value == 100);
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
        auto [found, value] = map.at(i);
        REQUIRE(found);
        REQUIRE(value == i * 100);
    }
}

TEST_CASE("InsertWithHash duplicate key updates", "[insertWithHash]") {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(5);
    RobinHoodMapTest::callInsertWithHash(map, 5, 100, hash);
    RobinHoodMapTest::callInsertWithHash(map, 5, 999, hash);
    
    REQUIRE(map.size() == 1u);
    auto [found, value] = map.at(5);
    REQUIRE(value == 999);
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
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    REQUIRE(f0);
    REQUIRE(f8);
    REQUIRE(f16);
}

TEST_CASE("InsertWithHash works with strings", "[insertWithHash]") {
    RobinHoodMap<std::string, int> map;
    std::hash<std::string> hasher;
    
    std::string key = "test_key";
    size_t hash = hasher(key);
    RobinHoodMapTest::callInsertWithHash(map, key, 42, hash);
    
    auto [found, value] = map.at("test_key");
    REQUIRE(found);
    REQUIRE(value == 42);
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
        auto [found, value] = map.at(i);
        REQUIRE(found);
        REQUIRE(value == i * 10);
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
    auto [found, value] = map.at(key);
    REQUIRE(found);
    REQUIRE(value == 100);
}
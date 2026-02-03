/**
 * @file RobinHoodMapTest.cpp
 * @brief Test suite for RobinHoodMap using Google Test.
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

#include <gtest/gtest.h>
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

TEST(Constructor, CreatesEmptyMap) {
    RobinHoodMap<int, int> map;
    EXPECT_EQ(map.size(), 0u);
}

TEST(Constructor, WorksWithStringTypes) {
    RobinHoodMap<std::string, std::string> map;
    EXPECT_EQ(map.size(), 0u);
}

// insert

TEST(Insert, SingleAndMultipleElements) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    EXPECT_EQ(map.size(), 1u);
    
    map.insert(2, 200);
    map.insert(3, 300);
    EXPECT_EQ(map.size(), 3u);
}

TEST(Insert, DuplicateKeyUpdatesValue) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(1, 999);
    EXPECT_EQ(map.size(), 1u);
    
    auto [found, value] = map.at(1);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 999);
}

TEST(Insert, StringKeysAndValues) {
    RobinHoodMap<std::string, std::string> map;
    map.insert("hello", "world");
    
    auto [found, value] = map.at("hello");
    EXPECT_TRUE(found);
    EXPECT_EQ(value, "world");
}

TEST(Insert, NegativeAndZeroKeys) {
    RobinHoodMap<int, int> map;
    map.insert(-1, 100);
    map.insert(0, 200);
    EXPECT_EQ(map.size(), 2u);
    
    auto [f1, v1] = map.at(-1);
    auto [f2, v2] = map.at(0);
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
}

// at

TEST(At, ExistingAndNonexistentKeys) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    auto [found1, value1] = map.at(1);
    EXPECT_TRUE(found1);
    EXPECT_EQ(value1, 100);
    
    auto [found2, value2] = map.at(999);
    EXPECT_FALSE(found2);
}

TEST(At, EmptyMap) {
    RobinHoodMap<int, int> map;
    auto [found, value] = map.at(1);
    EXPECT_FALSE(found);
}

// operator[]

TEST(SubscriptOperator, BehavesLikeAt) {
    RobinHoodMap<int, int> map;
    map.insert(5, 500);
    
    auto [found1, value1] = map[5];
    EXPECT_TRUE(found1);
    EXPECT_EQ(value1, 500);
    
    auto [found2, value2] = map[999];
    EXPECT_FALSE(found2);
}

// remove

TEST(Remove, ExistingKey) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.remove(1);
    
    EXPECT_EQ(map.size(), 1u);
    auto [found, value] = map.at(1);
    EXPECT_FALSE(found);
}

TEST(Remove, NonexistentKeyDoesNothing) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.remove(999);
    EXPECT_EQ(map.size(), 1u);
}

TEST(Remove, FromEmptyMap) {
    RobinHoodMap<int, int> map;
    map.remove(1);
    EXPECT_EQ(map.size(), 0u);
}

TEST(Remove, ThenReinsert) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.remove(1);
    map.insert(1, 999);
    
    auto [found, value] = map.at(1);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 999);
}

// assignment operator

TEST(AssignmentOperator, CreatesIndependentCopy) {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> map2;
    map2 = map1;
    
    EXPECT_EQ(map2.size(), 2u);
    
    // Modify original, copy should be unchanged
    map1.insert(1, 999);
    auto [found, value] = map2.at(1);
    EXPECT_EQ(value, 100);
}

// resizing (private)

TEST(Resize, DoublesTableSize) {
    RobinHoodMap<int, int> map;
    size_t initialSize = RobinHoodMapTest::getTableSize(map);
    
    RobinHoodMapTest::callResize(map);
    
    EXPECT_EQ(RobinHoodMapTest::getTableSize(map), initialSize * 2);
}

TEST(Resize, PreservesElements) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    RobinHoodMapTest::callResize(map);
    
    EXPECT_EQ(map.size(), 2u);
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
}

// finding first element (private)

TEST(FindFirstElement, EmptyAndNonEmpty) {
    RobinHoodMap<int, int> map;
    auto [worked1, key1] = RobinHoodMapTest::callFindFirstElement(map);
    EXPECT_FALSE(worked1);
    
    map.insert(42, 100);
    auto [worked2, key2] = RobinHoodMapTest::callFindFirstElement(map);
    EXPECT_TRUE(worked2);
}

// internal state

TEST(InternalState, InitialTableSize) {
    RobinHoodMap<int, int> map;
    EXPECT_EQ(RobinHoodMapTest::getTableSize(map), 8u);
}

TEST(InternalState, SizeConsistency) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    EXPECT_EQ(RobinHoodMapTest::getInternalSize(map), map.size());
}

// ============================================================================
// AI GENERATED TESTS (Claude Opus 4.5)
// ============================================================================

// Stress Tests

TEST(StressTest, ManyInsertions) {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 1000; ++i) {
        map.insert(i, i * 10);
    }
    EXPECT_EQ(map.size(), 1000u);
    
    auto [found, value] = map.at(500);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 5000);
}

TEST(StressTest, InsertAndRemove) {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 100; ++i) {
        map.insert(i, i);
    }
    for (int i = 0; i < 50; ++i) {
        map.remove(i);
    }
    
    EXPECT_EQ(map.size(), 50u);
    
    auto [found1, v1] = map.at(25);
    auto [found2, v2] = map.at(75);
    EXPECT_FALSE(found1);
    EXPECT_TRUE(found2);
}

// Collision Handling

TEST(Collisions, CollidingKeysWork) {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    EXPECT_EQ(map.size(), 3u);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    EXPECT_TRUE(f0);
    EXPECT_TRUE(f8);
    EXPECT_TRUE(f16);
}

TEST(Collisions, RemoveMiddleCollidingKey) {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(8);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    EXPECT_TRUE(f0);
    EXPECT_FALSE(f8);
    EXPECT_TRUE(f16);
}

TEST(Collisions, RemoveFirstCollidingKey) {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(0);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    EXPECT_FALSE(f0);
    EXPECT_TRUE(f8);
    EXPECT_TRUE(f16);
}

TEST(Collisions, RemoveLastCollidingKey) {
    RobinHoodMap<int, int> map;
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    
    map.remove(16);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    EXPECT_TRUE(f0);
    EXPECT_TRUE(f8);
    EXPECT_FALSE(f16);
}

TEST(Collisions, ManyCollidingKeys) {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 50; ++i) {
        map.insert(i * 8, i);
    }
    
    EXPECT_EQ(map.size(), 50u);
    
    for (int i = 0; i < 50; ++i) {
        auto [found, value] = map.at(i * 8);
        EXPECT_TRUE(found);
        EXPECT_EQ(value, i);
    }
}

// Boundary Values

TEST(BoundaryValues, IntMinMax) {
    RobinHoodMap<int, int> map;
    map.insert(INT_MAX, 100);
    map.insert(INT_MIN, 200);
    
    auto [f1, v1] = map.at(INT_MAX);
    auto [f2, v2] = map.at(INT_MIN);
    
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
    EXPECT_EQ(v1, 100);
    EXPECT_EQ(v2, 200);
}

TEST(BoundaryValues, SizeTMax) {
    RobinHoodMap<size_t, int> map;
    map.insert(SIZE_MAX, 42);
    
    auto [found, value] = map.at(SIZE_MAX);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 42);
}

// String Edge Cases

TEST(StringKeys, EmptyAndLongStrings) {
    RobinHoodMap<std::string, int> map;
    map.insert("", 1);
    map.insert(std::string(1000, 'x'), 2);
    
    auto [f1, v1] = map.at("");
    auto [f2, v2] = map.at(std::string(1000, 'x'));
    
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
}

TEST(StringKeys, CaseSensitive) {
    RobinHoodMap<std::string, int> map;
    map.insert("test", 1);
    map.insert("TEST", 2);
    
    EXPECT_EQ(map.size(), 2u);
    
    auto [f1, v1] = map.at("test");
    auto [f2, v2] = map.at("TEST");
    EXPECT_EQ(v1, 1);
    EXPECT_EQ(v2, 2);
}

TEST(StringKeys, SimilarStrings) {
    RobinHoodMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("aa", 2);
    map.insert("aaa", 3);
    map.insert("ab", 4);
    map.insert("ba", 5);
    
    EXPECT_EQ(map.size(), 5u);
    
    auto [f1, v1] = map.at("a");
    auto [f2, v2] = map.at("aa");
    auto [f3, v3] = map.at("ab");
    
    EXPECT_EQ(v1, 1);
    EXPECT_EQ(v2, 2);
    EXPECT_EQ(v3, 4);
}

TEST(StringKeys, WhitespaceKeys) {
    RobinHoodMap<std::string, int> map;
    map.insert(" ", 1);
    map.insert("  ", 2);
    map.insert("\t", 3);
    map.insert("\n", 4);
    
    EXPECT_EQ(map.size(), 4u);
}

// Iterator Tests

TEST(Iterator, BasicIteration) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        ++count;
    }
    
    EXPECT_EQ(count, 3);
}

TEST(Iterator, EmptyMapIteration) {
    RobinHoodMap<int, int> map;
    
    int count = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        ++count;
    }
    
    EXPECT_EQ(count, 0);
}

TEST(Iterator, RangeBasedFor) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    int sum = 0;
    for (auto& entry : map) {
        sum += entry.value;
    }
    
    EXPECT_EQ(sum, 300);
}

// Repeated Operations

TEST(RepeatedOps, InsertSameKeyManyTimes) {
    RobinHoodMap<int, int> map;
    
    for (int i = 0; i < 100; ++i) {
        map.insert(42, i);
    }
    
    EXPECT_EQ(map.size(), 1u);
    auto [found, value] = map.at(42);
    EXPECT_EQ(value, 99);
}

TEST(RepeatedOps, RemoveSameKeyManyTimes) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    
    for (int i = 0; i < 100; ++i) {
        map.remove(1);
    }
    
    EXPECT_EQ(map.size(), 0u);
}

TEST(RepeatedOps, InsertRemoveCycle) {
    RobinHoodMap<int, int> map;
    
    for (int cycle = 0; cycle < 10; ++cycle) {
        for (int i = 0; i < 50; ++i) {
            map.insert(i, i + cycle);
        }
        EXPECT_EQ(map.size(), 50u);
        
        for (int i = 0; i < 50; ++i) {
            map.remove(i);
        }
        EXPECT_EQ(map.size(), 0u);
    }
}

// Type Variations

TEST(TypeVariations, CharKey) {
    RobinHoodMap<char, int> map;
    
    for (char c = 'a'; c <= 'z'; ++c) {
        map.insert(c, c - 'a');
    }
    
    EXPECT_EQ(map.size(), 26u);
    
    auto [found, value] = map.at('m');
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 12);
}

TEST(TypeVariations, BoolKey) {
    RobinHoodMap<bool, std::string> map;
    map.insert(true, "yes");
    map.insert(false, "no");
    
    EXPECT_EQ(map.size(), 2u);
    
    auto [f1, v1] = map.at(true);
    auto [f2, v2] = map.at(false);
    EXPECT_EQ(v1, "yes");
    EXPECT_EQ(v2, "no");
}

TEST(TypeVariations, DoubleValue) {
    RobinHoodMap<int, double> map;
    map.insert(1, 3.14159);
    map.insert(2, 2.71828);
    
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    
    EXPECT_NEAR(v1, 3.14159, 0.00001);
    EXPECT_NEAR(v2, 2.71828, 0.00001);
}

// Resize Behavior

TEST(ResizeBehavior, AutoResizePreservesData) {
    RobinHoodMap<int, int> map;
    size_t initialTableSize = RobinHoodMapTest::getTableSize(map);
    
    for (int i = 0; i < 1000; ++i) {
        map.insert(i, i * 2);
    }
    
    EXPECT_GT(RobinHoodMapTest::getTableSize(map), initialTableSize);
    
    for (int i = 0; i < 1000; ++i) {
        auto [found, value] = map.at(i);
        EXPECT_TRUE(found);
        EXPECT_EQ(value, i * 2);
    }
}

TEST(ResizeBehavior, MultipleManualResizes) {
    RobinHoodMap<int, int> map;
    map.insert(1, 100);
    map.insert(2, 200);
    
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    EXPECT_EQ(map.size(), 2u);
    auto [f1, v1] = map.at(1);
    auto [f2, v2] = map.at(2);
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
}

// Assignment Edge Cases

TEST(Assignment, AssignEmptyToFull) {
    RobinHoodMap<int, int> map1;
    map1.insert(1, 100);
    map1.insert(2, 200);
    
    RobinHoodMap<int, int> emptyMap;
    map1 = emptyMap;
    
    EXPECT_EQ(map1.size(), 0u);
}

TEST(Assignment, AssignFullToEmpty) {
    RobinHoodMap<int, int> emptyMap;
    
    RobinHoodMap<int, int> fullMap;
    fullMap.insert(1, 100);
    
    emptyMap = fullMap;
    
    EXPECT_EQ(emptyMap.size(), 1u);
}

// Backshift Verification (Robin Hood specific)

TEST(Backshift, RemovalTriggersBackshift) {
    RobinHoodMap<int, int> map;
    
    map.insert(0, 100);
    map.insert(8, 200);
    map.insert(16, 300);
    map.insert(24, 400);
    
    map.remove(8);
    map.remove(16);
    
    auto [f0, v0] = map.at(0);
    auto [f24, v24] = map.at(24);
    
    EXPECT_TRUE(f0);
    EXPECT_TRUE(f24);
    EXPECT_EQ(v0, 100);
    EXPECT_EQ(v24, 400);
}

// Hash Stored Optimization Verification

TEST(HashOptimization, StoredHashWorksCorrectly) {
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
    
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
    EXPECT_TRUE(f3);
    EXPECT_EQ(v1, 1);
    EXPECT_EQ(v2, 2);
    EXPECT_EQ(v3, 3);
}

// _insertWithHash Tests (private method)

TEST(InsertWithHash, BasicInsert) {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(42);
    RobinHoodMapTest::callInsertWithHash(map, 42, 100, hash);
    
    EXPECT_EQ(map.size(), 1u);
    auto [found, value] = map.at(42);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 100);
}

TEST(InsertWithHash, PreservesProvidedHash) {
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
    
    EXPECT_EQ(RobinHoodMapTest::getStoredHash(map, index), expectedHash);
}

TEST(InsertWithHash, MultipleInserts) {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    // Resize first so we have room (table size 8 -> 16 -> 32)
    // _insertWithHash doesn't trigger resize, so we need enough space
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    for (int i = 0; i < 10; ++i) {
        RobinHoodMapTest::callInsertWithHash(map, i, i * 100, hasher(i));
    }
    
    EXPECT_EQ(map.size(), 10u);
    
    for (int i = 0; i < 10; ++i) {
        auto [found, value] = map.at(i);
        EXPECT_TRUE(found);
        EXPECT_EQ(value, i * 100);
    }
}

TEST(InsertWithHash, DuplicateKeyUpdates) {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    size_t hash = hasher(5);
    RobinHoodMapTest::callInsertWithHash(map, 5, 100, hash);
    RobinHoodMapTest::callInsertWithHash(map, 5, 999, hash);
    
    EXPECT_EQ(map.size(), 1u);
    auto [found, value] = map.at(5);
    EXPECT_EQ(value, 999);
}

TEST(InsertWithHash, HandlesCollisions) {
    RobinHoodMap<int, int> map;
    std::hash<int> hasher;
    
    // Resize to ensure we have room for colliding keys
    RobinHoodMapTest::callResize(map);
    
    // Insert keys that will collide (multiples of 8)
    RobinHoodMapTest::callInsertWithHash(map, 0, 100, hasher(0));
    RobinHoodMapTest::callInsertWithHash(map, 8, 200, hasher(8));
    RobinHoodMapTest::callInsertWithHash(map, 16, 300, hasher(16));
    
    EXPECT_EQ(map.size(), 3u);
    
    auto [f0, v0] = map.at(0);
    auto [f8, v8] = map.at(8);
    auto [f16, v16] = map.at(16);
    
    EXPECT_TRUE(f0);
    EXPECT_TRUE(f8);
    EXPECT_TRUE(f16);
}

TEST(InsertWithHash, WorksWithStrings) {
    RobinHoodMap<std::string, int> map;
    std::hash<std::string> hasher;
    
    std::string key = "test_key";
    size_t hash = hasher(key);
    RobinHoodMapTest::callInsertWithHash(map, key, 42, hash);
    
    auto [found, value] = map.at("test_key");
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 42);
}

TEST(InsertWithHash, UsedByResizeCorrectly) {
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
        EXPECT_TRUE(found);
        EXPECT_EQ(value, i * 10);
    }
}

TEST(InsertWithHash, HashPreservedAfterResize) {
    RobinHoodMap<int, int> map;
    
    int key = 42;
    map.insert(key, 100);
    
    // Force multiple resizes
    RobinHoodMapTest::callResize(map);
    RobinHoodMapTest::callResize(map);
    
    // Element should still be findable (hash was preserved during resize)
    auto [found, value] = map.at(key);
    EXPECT_TRUE(found);
    EXPECT_EQ(value, 100);
}
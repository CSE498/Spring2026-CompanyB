#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <set>
#include <vector>

#include "weighted_set.hpp"

TEST_CASE("WeightedSet insert and contains", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("empty set") {
    REQUIRE(ws.empty());
    REQUIRE(ws.total_weight() == 0.0);
  }

  SECTION("insert and contains") {
    ws.insert(1, 1.0);
    REQUIRE(ws.contains(1));
    REQUIRE_FALSE(ws.contains(2));
    REQUIRE(ws.total_weight() == 1.0);

    ws.insert(2, 2.0);
    ws.insert(3, 3.0);
    REQUIRE(ws.contains(1));
    REQUIRE(ws.contains(2));
    REQUIRE(ws.contains(3));

    REQUIRE_FALSE(ws.contains(0));

    REQUIRE(ws.total_weight() == 6.0);
  }

  SECTION("bad insertions") {
    bool success = ws.insert(1, 0.0);
    REQUIRE_FALSE(success);
    REQUIRE_FALSE(ws.contains(1));
    REQUIRE(ws.total_weight() == 0.0);

    success = ws.insert(1, -1.0);
    REQUIRE_FALSE(success);
    REQUIRE_FALSE(ws.contains(1));
    REQUIRE(ws.total_weight() == 0.0);
  }
}

TEST_CASE("WeightedSet indexing", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("indexing with 1 and 2 elements, equal weights") {
    ws.insert(1, 1.0);
    std::optional<int> result = ws.getElementAt(0);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);

    result = ws.getElementAt(0.5);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);

    result = ws.getElementAt(2);
    REQUIRE_FALSE(result.has_value());

    ws.insert(2, 1.0);
    auto result_at_zero = ws.getElementAt(0.0);
    auto result_at_one = ws.getElementAt(1.0);
    auto result_at_two = ws.getElementAt(2.0);

    REQUIRE(result_at_zero.has_value());
    REQUIRE(result_at_one.has_value());
    REQUIRE(result_at_two.has_value());
    // the point behind the following tests:
    // we could, in principle, end up with 1 having indexes in the range [0, 1)
    // and 2 having indexes in the range [1, 2]--or vice versa. The spec doesn't
    // specify, and the user shouldn't count on a particular one of those
    // possibilities being true. So we can test for one or the other happening
    // without requiring that a specific one happens.
    REQUIRE(((result_at_zero.value() == 1 && result_at_one.value() == 2) ||
             (result_at_zero.value() == 2 && result_at_one.value() == 1)));

    auto result_at_half = ws.getElementAt(0.5);
    auto result_at_one_and_a_half = ws.getElementAt(1.5);
    REQUIRE(result_at_zero.value() == result_at_half.value());
    REQUIRE(result_at_one.value() == result_at_one_and_a_half.value());
  }

  SECTION("indexing with 2 elements, unequal weights") {
    ws.insert(1, 2.0);

    double index = 0;
    while (index <= 2) {
      REQUIRE(ws.getElementAt(index) == 1);
      index += 0.5;
    }
    ws.insert(2, 1.0);

    auto result_at_zero = ws.getElementAt(0);
    auto result_at_one = ws.getElementAt(1);
    auto result_at_two = ws.getElementAt(2);
    auto result_at_three = ws.getElementAt(3);
    // two possibilities: 1 gets [0, 2) and 2 gets [2, 3]
    // or 2 gets [0, 1) and 1 gets [1, 3]
    REQUIRE(((result_at_zero == 1 && result_at_one == 1 && result_at_two == 2 &&
              result_at_three == 2) ||
             (result_at_zero == 2 && result_at_one == 1 && result_at_two == 1 &&
              result_at_three == 1)));
  }

  SECTION("handling many elements") {
    for (int i = 0; i < 10; i++) {
      ws.insert(i, 1.0);
    }
    REQUIRE(ws.total_weight() == 10);
    std::set<int> elements_seen{};
    for (int i = 0; i < 10; i++) {
      std::optional<int> result = ws.getElementAt(i);
      REQUIRE(result.has_value());
      int el = result.value();
      REQUIRE((0 <= el && el < 10));
      REQUIRE(!elements_seen.contains(el));
      REQUIRE(ws.getElementAt(i + 0.5).value() == el);
      elements_seen.insert(el);
    }
  }
}

TEST_CASE("WeightedSet deletion", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("single element") {
    ws.insert(1, 1.0);
    REQUIRE(ws.contains(1));
    REQUIRE(ws.total_weight() == 1);

    auto result = ws.remove(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);
    REQUIRE(!ws.contains(1));
    REQUIRE(ws.total_weight() == 0);
  }

  SECTION("three elements") {
    ws.insert(1, 1.0);
    ws.insert(2, 2.0);
    ws.insert(3, 3.0);

    REQUIRE(ws.total_weight() == 6);
    std::vector<std::vector<int>> removal_orders = {
        {1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1}};
    for (std::vector<int> elements_to_remove : removal_orders) {
      SECTION("try removing elements in order " +
              std::to_string(elements_to_remove[0]) +
              std::to_string(elements_to_remove[1]) +
              std::to_string(elements_to_remove[2])) {
        double target_weight = 6.0;
        for (int to_remove : elements_to_remove) {
          auto result = ws.remove(to_remove);
          REQUIRE(result.has_value());
          REQUIRE(result.value() == to_remove);
          REQUIRE(!ws.contains(to_remove));
          target_weight -= to_remove;
          REQUIRE(ws.total_weight() == target_weight);
        }
      }
    }
  }

  for (int i = 0; i < 10; ++i) {
    ws.insert(i, 1.0);
  }
  // the real point of this test/set of tests is to try removing elements at
  // different places on the tree--elements at the root, not at the root but
  // with 2 children, with only 1 child, and so on.
  SECTION("many elements, removing only one per test") {
    for (int i = 0; i < 10; ++i) {
      SECTION("just remove element " + std::to_string(i)) {
        auto result = ws.remove(i);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == i);
        REQUIRE(ws.total_weight() == 9.0);
        REQUIRE(!ws.contains(i));
        for (int j = 0; j < 10; ++j) {
          if (j != i) {
            REQUIRE(ws.contains(j));
          }
        }
      }
    }
  }

  SECTION("many elements, removing two per test") {
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 10; ++j) {
        if (i == j) continue;
        SECTION("Removing element " + std::to_string(i) + ", then element " +
                std::to_string(j)) {
          auto result1 = ws.remove(i);
          auto result2 = ws.remove(j);

          REQUIRE(result2.has_value());
          REQUIRE(result2.value() == j);
          REQUIRE(ws.total_weight() == 8.0);
          REQUIRE(!ws.contains(i));
          REQUIRE(!ws.contains(j));
          for (int k = 0; k < 10; ++k) {
            if (k != i && k != j) {
              REQUIRE(ws.contains(k));
            }
          }
        }
      }
    }
  }
}
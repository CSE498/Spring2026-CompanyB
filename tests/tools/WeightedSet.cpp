#include "../../source/tools/WeightedSet.hpp"

#include <expected>
#include <set>
#include <vector>

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

TEST_CASE("WeightedSet Insert and Contains", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("empty set") {
    REQUIRE(ws.empty());
    REQUIRE(ws.total_weight() == 0.0);
  }

  SECTION("Insert and Contains") {
    ws.Insert(1, 1.0);
    REQUIRE(ws.Contains(1));
    REQUIRE_FALSE(ws.Contains(2));
    REQUIRE(ws.total_weight() == 1.0);

    ws.Insert(2, 2.0);
    ws.Insert(3, 3.0);
    REQUIRE(ws.Contains(1));
    REQUIRE(ws.Contains(2));
    REQUIRE(ws.Contains(3));

    REQUIRE_FALSE(ws.Contains(0));

    REQUIRE(ws.total_weight() == 6.0);

    ws.Insert(1, 2.0);
    REQUIRE(ws.total_weight() == 7.0);
  }

  SECTION("bad insertions") {
    bool success = ws.Insert(1, 0.0);
    REQUIRE_FALSE(success);
    REQUIRE_FALSE(ws.Contains(1));
    REQUIRE(ws.total_weight() == 0.0);

    success = ws.Insert(1, -1.0);
    REQUIRE_FALSE(success);
    REQUIRE_FALSE(ws.Contains(1));
    REQUIRE(ws.total_weight() == 0.0);
  }
}

TEST_CASE("WeightedSet assignment", "[weighted_set]") {
  cse498::WeightedSet<int> ws1;
  ws1.Insert(1, 1.0);
  ws1.Insert(2, 2.0);
  ws1.Insert(3, 3.0);

  cse498::WeightedSet<int> ws2;
  ws2.Insert(4, 4.0);
  ws2.Insert(5, 5.0);
  ws2.Insert(6, 6.0);

  ws2 = ws1;
  REQUIRE(ws2.total_weight() == 6.0);
  REQUIRE(ws2.Contains(1));
  REQUIRE(ws2.Contains(2));
  REQUIRE(ws2.Contains(3));
  REQUIRE(!ws2.Contains(4));
  REQUIRE(!ws2.Contains(5));
  REQUIRE(!ws2.Contains(6));
}

void AddStuffToReferencedSet(cse498::WeightedSet<int>& ws) {
  ws.Insert(1, 1.0);
  ws.Insert(2, 2.0);
  ws.Insert(3, 3.0);
}

TEST_CASE("WeightedSet reference/copy constructor", "[weighted_set]") {
  cse498::WeightedSet<int> ws;
  AddStuffToReferencedSet(ws);
  REQUIRE(ws.total_weight() == 6.0);
  REQUIRE(ws.size() == 3);
  REQUIRE(ws.Contains(1));
  REQUIRE(ws.Contains(2));
  REQUIRE(ws.Contains(3));

  cse498::WeightedSet<int> ws2(ws);
  REQUIRE(ws2.total_weight() == 6.0);
  REQUIRE(ws2.size() == 3);
  REQUIRE(ws2.Contains(1));
  REQUIRE(ws2.Contains(2));
  REQUIRE(ws2.Contains(3));

  REQUIRE(ws.total_weight() == 6.0);
  REQUIRE(ws.size() == 3);
  REQUIRE(ws.Contains(1));
  REQUIRE(ws.Contains(2));
  REQUIRE(ws.Contains(3));

  cse498::WeightedSet<int> ws3{};
  ws2 = ws3;
  REQUIRE(ws2.empty());
} 

TEST_CASE("WeightedSet move") {
  cse498::WeightedSet<int> ws{};
  ws.Insert(1, 1.0);
  ws.Insert(2, 2.0);
  ws.Insert(3, 3.0);

  cse498::WeightedSet<int> ws2{};
  ws2.Insert(4, 4.0);
  ws2.Insert(5, 5.0);
  ws2.Insert(6, 6.0);

  ws2 = std::move(ws);
  REQUIRE(ws.empty());
  REQUIRE(ws2.Contains(1));
  REQUIRE(ws2.Contains(2));
  REQUIRE(ws2.Contains(3));
  REQUIRE(ws2.total_weight() == 6.0);
  REQUIRE(!ws2.Contains(4));
  REQUIRE(!ws2.Contains(5));
  REQUIRE(!ws2.Contains(6));
}

TEST_CASE("WeightedSet indexing", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("indexing with 1 and 2 elements, equal weights") {
    ws.Insert(1, 1.0);
    std::optional<int> result = ws.GetElementAt(0);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);

    result = ws.GetElementAt(0.5);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);

    result = ws.GetElementAt(2);
    REQUIRE_FALSE(result.has_value());

    ws.Insert(2, 1.0);
    auto result_at_zero = ws.GetElementAt(0.0);
    auto result_at_one = ws.GetElementAt(1.0);
    auto result_at_two = ws.GetElementAt(2.0);

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

    auto result_at_half = ws.GetElementAt(0.5);
    auto result_at_one_and_a_half = ws.GetElementAt(1.5);
    REQUIRE(result_at_zero.value() == result_at_half.value());
    REQUIRE(result_at_one.value() == result_at_one_and_a_half.value());
  }

  SECTION("indexing with 2 elements, unequal weights") {
    ws.Insert(1, 2.0);

    double index = 0;
    while (index <= 2) {
      REQUIRE(ws.GetElementAt(index) == 1);
      index += 0.5;
    }
    ws.Insert(2, 1.0);

    auto result_at_zero = ws.GetElementAt(0);
    auto result_at_one = ws.GetElementAt(1);
    auto result_at_two = ws.GetElementAt(2);
    auto result_at_three = ws.GetElementAt(3);
    // two possibilities: 1 gets [0, 2) and 2 gets [2, 3]
    // or 2 gets [0, 1) and 1 gets [1, 3]
    REQUIRE(((result_at_zero == 1 && result_at_one == 1 && result_at_two == 2 &&
              result_at_three == 2) ||
             (result_at_zero == 2 && result_at_one == 1 && result_at_two == 1 &&
              result_at_three == 1)));
  }

  SECTION("handling many elements") {
    for (int i = 0; i < 10; i++) {
      ws.Insert(i, 1.0);
    }
    REQUIRE(ws.total_weight() == 10);
    std::set<int> elements_seen{};
    for (int i = 0; i < 10; i++) {
      std::optional<int> result = ws.GetElementAt(i);
      REQUIRE(result.has_value());
      int el = result.value();
      REQUIRE((0 <= el && el < 10));
      REQUIRE(!elements_seen.contains(el));
      REQUIRE(ws.GetElementAt(i + 0.5).value() == el);
      elements_seen.insert(el);
    }
  }
}

TEST_CASE("WeightedSet deletion", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("single element") {
    ws.Insert(1, 1.0);
    REQUIRE(ws.Contains(1));
    REQUIRE(ws.total_weight() == 1);

    auto result = ws.Remove(1);
    REQUIRE(result.has_value());
    REQUIRE(result.value() == 1);
    REQUIRE(!ws.Contains(1));
    REQUIRE(ws.total_weight() == 0);
  }

  SECTION("three elements") {
    ws.Insert(1, 1.0);
    ws.Insert(2, 2.0);
    ws.Insert(3, 3.0);

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
          auto result = ws.Remove(to_remove);
          REQUIRE(result.has_value());
          REQUIRE(result.value() == to_remove);
          REQUIRE(!ws.Contains(to_remove));
          target_weight -= to_remove;
          REQUIRE(ws.total_weight() == target_weight);
        }
      }
    }
  }

  for (int i = 0; i < 10; ++i) {
    ws.Insert(i, 1.0);
  }
  // the real point of this test/set of tests is to try removing elements at
  // different places on the tree--elements at the root, not at the root but
  // with 2 children, with only 1 child, and so on.
  SECTION("many elements, removing only one per test") {
    for (int i = 0; i < 10; ++i) {
      SECTION("just remove element " + std::to_string(i)) {
        auto result = ws.Remove(i);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == i);
        REQUIRE(ws.total_weight() == 9.0);
        REQUIRE(!ws.Contains(i));
        for (int j = 0; j < 10; ++j) {
          if (j != i) {
            REQUIRE(ws.Contains(j));
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
          ws.Remove(i);
          auto result = ws.Remove(j);

          REQUIRE(result.has_value());
          REQUIRE(result.value() == j);
          REQUIRE(ws.total_weight() == 8.0);
          REQUIRE(!ws.Contains(i));
          REQUIRE(!ws.Contains(j));
          for (int k = 0; k < 10; ++k) {
            if (k != i && k != j) {
              REQUIRE(ws.Contains(k));
            }
          }
        }
      }
    }
  }
}
#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <set>

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
    REQUIRE(((result_at_zero == 1 && result_at_one == 1 && result_at_two == 2 && result_at_three == 2) ||
            (result_at_zero == 2 && result_at_one == 1 && result_at_two == 1 && result_at_three == 1)));
    
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
#include "../../source/tools/WeightedSet.hpp"

#include <cmath>
#include <set>
#include <unordered_map>
#include <vector>

#include "../../third-party/Catch/single_include/catch2/catch.hpp"

void FillSetWithInts(cse498::WeightedSet<int>& ws, const int lower,
                     const int upper, const bool differentWeights = false) {
  for (int i = lower; i <= upper; ++i) {
    ws.Insert(i, differentWeights ? static_cast<double>(i) : 1.0);
  }
}

TEST_CASE("WeightedSet Insert and Contains", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("empty set") {
    CHECK(ws.empty());
    CHECK(ws.total_weight() == 0.0);
    CHECK(ws.size() == 0);
  }

  SECTION("Insert and Contains") {
    ws.Insert(1, 1.0);
    CHECK(ws.Contains(1));
    CHECK_FALSE(ws.Contains(2));
    CHECK(ws.total_weight() == 1.0);
    CHECK(ws.size() == 1);

    ws.Insert(2, 2.0);
    ws.Insert(3, 3.0);
    CHECK(ws.Contains(1));
    CHECK(ws.Contains(2));
    CHECK(ws.Contains(3));

    CHECK_FALSE(ws.Contains(0));

    CHECK(ws.total_weight() == 6.0);
    CHECK(ws.size() == 3);

    ws.Insert(1, 2.0);
    CHECK(ws.total_weight() == 7.0);
    CHECK(ws.size() == 3);
  }

  SECTION("bad insertions") {
    bool success = ws.Insert(1, 0.0);
    CHECK_FALSE(success);
    CHECK_FALSE(ws.Contains(1));
    CHECK(ws.total_weight() == 0.0);

    success = ws.Insert(1, -1.0);
    CHECK_FALSE(success);
    CHECK_FALSE(ws.Contains(1));
    CHECK(ws.total_weight() == 0.0);

    CHECK(ws.empty());
  }
}

TEST_CASE("WeightedSet assignment", "[weighted_set]") {
  cse498::WeightedSet<int> ws1;
  FillSetWithInts(ws1, 1, 3, true);

  cse498::WeightedSet<int> ws2;
  FillSetWithInts(ws2, 4, 6, true);

  ws2 = ws1;
  CHECK(ws2.total_weight() == 6.0);
  CHECK(ws2.size() == 3);
  CHECK(ws2.Contains(1));
  CHECK(ws2.Contains(2));
  CHECK(ws2.Contains(3));
  CHECK_FALSE(ws2.Contains(4));
  CHECK_FALSE(ws2.Contains(5));
  CHECK_FALSE(ws2.Contains(6));
}

TEST_CASE("WeightedSet copy constructor", "[weighted_set]") {
  cse498::WeightedSet<int> ws;
  FillSetWithInts(ws, 1, 3, true);

  cse498::WeightedSet<int> ws2(ws);
  CHECK(ws2.total_weight() == 6.0);
  CHECK(ws2.size() == 3);
  CHECK(ws2.Contains(1));
  CHECK(ws2.Contains(2));
  CHECK(ws2.Contains(3));

  CHECK(ws.total_weight() == 6.0);
  CHECK(ws.size() == 3);
  CHECK(ws.Contains(1));
  CHECK(ws.Contains(2));
  CHECK(ws.Contains(3));

  cse498::WeightedSet<int> ws3{};
  ws2 = ws3;
  CHECK(ws2.empty());
}

TEST_CASE("WeightedSet move", "[weighted_set]") {
  cse498::WeightedSet<int> ws{};
  FillSetWithInts(ws, 1, 3, true);

  cse498::WeightedSet<int> ws2{};
  FillSetWithInts(ws2, 4, 6, true);

  ws2 = std::move(ws);
  CHECK(ws.empty());
  CHECK(ws2.Contains(1));
  CHECK(ws2.Contains(2));
  CHECK(ws2.Contains(3));

  CHECK(ws2.total_weight() == 6.0);
  CHECK(ws2.size() == 3);

  CHECK_FALSE(ws2.Contains(4));
  CHECK_FALSE(ws2.Contains(5));
  CHECK_FALSE(ws2.Contains(6));
}

TEST_CASE("WeightedSet move constructor", "[weighted_set]") {
  cse498::WeightedSet<int> ws{};
  FillSetWithInts(ws, 1, 3, true);

  cse498::WeightedSet ws2(std::move(ws));
  CHECK(ws.empty());
  CHECK(ws2.Contains(1));
  CHECK(ws2.Contains(2));
  CHECK(ws2.Contains(3));
  CHECK(ws2.total_weight() == 6.0);
  CHECK(ws2.size() == 3);
}

TEST_CASE("WeightedSet indexing", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("empty set") {
    auto result = ws.GetElementAt(0);
    CHECK(result == std::nullopt);
    result = ws.GetElementAt(1);
    CHECK(result == std::nullopt);
  }

  SECTION("indexing with 1 and 2 elements, equal weights") {
    ws.Insert(1, 1.0);
    std::optional<int> result = ws.GetElementAt(0);
    CHECK(result.has_value());
    CHECK(result.value() == 1);

    result = ws.GetElementAt(0.5);
    CHECK(result.has_value());
    CHECK(result.value() == 1);

    result = ws.GetElementAt(2);
    CHECK_FALSE(result.has_value());

    ws.Insert(2, 1.0);
    auto result_at_zero = ws.GetElementAt(0.0);
    auto result_at_one = ws.GetElementAt(1.0);
    auto result_at_two = ws.GetElementAt(2.0);

    CHECK(result_at_zero.has_value());
    CHECK(result_at_one.has_value());
    CHECK(result_at_two.has_value());
    // the point behind the following tests:
    // we could, in principle, end up with 1 having indexes in the range [0, 1)
    // and 2 having indexes in the range [1, 2]--or vice versa. The spec doesn't
    // specify, and the user shouldn't count on a particular one of those
    // possibilities being true. So we can test for one or the other happening
    // without requiring that a specific one happens.
    CHECK(((result_at_zero.value() == 1 && result_at_one.value() == 2) ||
             (result_at_zero.value() == 2 && result_at_one.value() == 1)));

    auto result_at_half = ws.GetElementAt(0.5);
    auto result_at_one_and_a_half = ws.GetElementAt(1.5);
    CHECK(result_at_zero.value() == result_at_half.value());
    CHECK(result_at_one.value() == result_at_one_and_a_half.value());

    SECTION("bad indexes") {
      result = ws.GetElementAt(-1);
      CHECK(result == std::nullopt);
      result = ws.GetElementAt(5);
      CHECK(result == std::nullopt);
    }
  }

  SECTION("indexing with 2 elements, unequal weights") {
    ws.Insert(1, 2.0);

    double index = 0;
    while (index <= 2) {
      CHECK(ws.GetElementAt(index) == 1);
      index += 0.5;
    }
    ws.Insert(2, 1.0);

    auto result_at_zero = ws.GetElementAt(0);
    auto result_at_one = ws.GetElementAt(1);
    auto result_at_two = ws.GetElementAt(2);
    auto result_at_three = ws.GetElementAt(3);
    // two possibilities: 1 gets [0, 2) and 2 gets [2, 3]
    // or 2 gets [0, 1) and 1 gets [1, 3]
    CHECK(((result_at_zero == 1 && result_at_one == 1 && result_at_two == 2 &&
              result_at_three == 2) ||
             (result_at_zero == 2 && result_at_one == 1 && result_at_two == 1 &&
              result_at_three == 1)));
  }

  SECTION("handling many elements") {
    FillSetWithInts(ws, 0, 9);
    CHECK(ws.total_weight() == 10);
    CHECK(ws.size() == 10);
    std::set<int> elements_seen{};
    for (int i = 0; i < 10; i++) {
      std::optional<int> result = ws.GetElementAt(i);
      CHECK(result.has_value());
      int el = result.value();
      CHECK((0 <= el && el < 10));
      CHECK_FALSE(elements_seen.contains(el));
      CHECK(ws.GetElementAt(i + 0.5).value() == el);
      elements_seen.insert(el);
    }
  }
}

TEST_CASE("WeightedSet deletion", "[weighted_set]") {
  cse498::WeightedSet<int> ws;

  SECTION("single element") {
    ws.Insert(1, 1.0);
    CHECK(ws.Contains(1));
    CHECK(ws.total_weight() == 1);

    auto result = ws.Remove(1);
    CHECK(result.has_value());
    CHECK(result.value() == 1);
    CHECK_FALSE(ws.Contains(1));
    CHECK(ws.total_weight() == 0);
    CHECK(ws.size() == 0);
  }

  SECTION("bad deletes") {
    ws.Insert(1, 1.0);
    auto result = ws.Remove(2);
    CHECK(result == std::nullopt);
    CHECK(ws.size() == 1);

    result = ws.Remove(1);
    CHECK(result.value() == 1);
    CHECK(ws.size() == 0);
    result = ws.Remove(1);
    CHECK(result == std::nullopt);
  }

  SECTION("three elements") {
    FillSetWithInts(ws, 1, 3, true);

    CHECK(ws.total_weight() == 6);
    std::vector<std::vector<int>> removal_orders = {
        {1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1}};
    for (const auto & elements_to_remove : removal_orders) {
      SECTION("try removing elements in order " +
              std::to_string(elements_to_remove[0]) +
              std::to_string(elements_to_remove[1]) +
              std::to_string(elements_to_remove[2])) {
        double target_weight = 6.0;
        size_t target_size = 3;
        for (int to_remove : elements_to_remove) {
          auto result = ws.Remove(to_remove);
          CHECK(result.has_value());
          CHECK(result.value() == to_remove);
          CHECK_FALSE(ws.Contains(to_remove));
          target_weight -= to_remove;
          target_size--;
          CHECK(ws.total_weight() == target_weight);
          CHECK(ws.size() == target_size);
        }
      }
    }
  }

  FillSetWithInts(ws, 0, 9);
  // the real point of this test and the next one is to try removing elements at
  // different places on the tree--elements at the root, not at the root but
  // with 2 children, with only 1 child, and so on.
  SECTION("many elements, removing only one per test") {
    for (int i = 0; i < 10; ++i) {
      SECTION("just remove element " + std::to_string(i)) {
        auto result = ws.Remove(i);
        CHECK(result.has_value());
        CHECK(result.value() == i);
        CHECK(ws.total_weight() == 9.0);
        CHECK(ws.size() == 9);
        CHECK_FALSE(ws.Contains(i));
        for (int j = 0; j < 10; ++j) {
          if (j != i) {
            CHECK(ws.Contains(j));
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

          CHECK(result.has_value());
          CHECK(result.value() == j);
          CHECK(ws.total_weight() == 8.0);
          CHECK(ws.size() == 8);
          CHECK_FALSE(ws.Contains(i));
          CHECK_FALSE(ws.Contains(j));
          for (int k = 0; k < 10; ++k) {
            if (k != i && k != j) {
              CHECK(ws.Contains(k));
            }
          }
        }
      }
    }
  }
}

// Written by Claude
TEST_CASE("WeightedSet random selection frequencies", "[weighted_set]") {
  cse498::WeightedSet<int> ws;
  // Elements 1..5 with weights 1..5; total weight = 15.
  const int num_elements = 5;
  FillSetWithInts(ws, 1, num_elements, true);

  const double total_weight = ws.total_weight();
  CHECK(total_weight == 15.0);

  const int num_samples = 60000;
  // Number of standard deviations to allow as tolerance. At 4 sigma the
  // probability of a single element failing by chance is ~0.006%, and with
  // 5 elements the overall chance of a spurious failure is still very low.
  const double num_sigma = 4.0;

  std::unordered_map<int, int> counts;
  for (int i = 0; i < num_samples; ++i) {
    auto result = ws.GetRandomElement();
    CHECK(result.has_value());
    ++counts[result.value()];
  }

  // Every element should have been selected at least once.
  CHECK(counts.size() == static_cast<size_t>(num_elements));

  for (int elem = 1; elem <= num_elements; ++elem) {
    const double expected_prob = static_cast<double>(elem) / total_weight;
    const double expected_count = num_samples * expected_prob;
    const double std_dev =
        std::sqrt(num_samples * expected_prob * (1.0 - expected_prob));
    const double tolerance = num_sigma * std_dev;

    INFO("element " << elem << ": count=" << counts[elem]
                    << " expected=" << expected_count
                    << " tolerance=" << tolerance);
    CHECK(std::abs(counts[elem] - expected_count) <= tolerance);
  }
}
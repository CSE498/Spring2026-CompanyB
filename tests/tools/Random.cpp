#include "../../source/tools/Random.hpp"

#include <cmath>
#include <vector>

#include "catch2/catch.hpp"

TEST_CASE("Random basic range behavior", "[random]") {
  cse498::Random rng(12345);

  SECTION("GetDouble stays in [min, max)") {
    for (int i = 0; i < 2000; ++i) {
      double v = rng.GetDouble(-2.5, 7.5);
      CHECK(v >= -2.5);
      CHECK(v < 7.5);
    }
  }

  SECTION("GetDouble handles swapped bounds") {
    for (int i = 0; i < 2000; ++i) {
      double v = rng.GetDouble(7.5, -2.5);
      CHECK(v >= -2.5);
      CHECK(v < 7.5);
    }
  }

  SECTION("GetFloat stays in [min, max)") {
    for (int i = 0; i < 2000; ++i) {
      float v = rng.GetFloat(-3.0f, 2.0f);
      CHECK(v >= -3.0f);
      CHECK(v < 2.0f);
    }
  }

  SECTION("GetInt stays in [min, max]") {
    for (int i = 0; i < 4000; ++i) {
      int v = rng.GetInt(-4, 6);
      CHECK(v >= -4);
      CHECK(v <= 6);
    }
  }

  SECTION("GetInt handles swapped bounds") {
    for (int i = 0; i < 4000; ++i) {
      int v = rng.GetInt(6, -4);
      CHECK(v >= -4);
      CHECK(v <= 6);
    }
  }
}

TEST_CASE("Random reproducibility and reseeding", "[random]") {
  SECTION("same seed gives same sequence") {
    cse498::Random a(424242);
    cse498::Random b(424242);

    for (int i = 0; i < 200; ++i) {
      CHECK(a.GetInt(-10000, 10000) == b.GetInt(-10000, 10000));
    }
  }

  SECTION("reseeding rewinds sequence") {
    cse498::Random rng(1337);
    std::vector<int> first_pass;
    first_pass.reserve(100);

    for (int i = 0; i < 100; ++i) {
      first_pass.push_back(rng.GetInt(0, 1000000));
    }

    rng.Seed(1337);

    for (int i = 0; i < 100; ++i) {
      CHECK(rng.GetInt(0, 1000000) == first_pass[static_cast<size_t>(i)]);
    }
  }
}

TEST_CASE("Random bool edge cases and frequencies", "[random]") {
  cse498::Random rng(987654321);

  SECTION("GetBool endpoints") {
    for (int i = 0; i < 1000; ++i) {
      CHECK_FALSE(rng.GetBool(0.0));
      CHECK(rng.GetBool(1.0));
    }
  }

  SECTION("GetBool(0.7) frequency is close to expected") {
    const int num_samples = 60000;
    const double p = 0.7;
    int true_count = 0;
    for (int i = 0; i < num_samples; ++i) {
      if (rng.GetBool(p)) ++true_count;
    }

    const double expected = num_samples * p;
    const double std_dev = std::sqrt(num_samples * p * (1.0 - p));
    const double tolerance = 4.0 * std_dev;
    INFO("true_count=" << true_count << " expected=" << expected
                       << " tolerance=" << tolerance);
    CHECK(std::abs(static_cast<double>(true_count) - expected) <= tolerance);
  }
}

TEST_CASE("Random integer frequency sanity check", "[random]") {
  cse498::Random rng(2026);

  const int num_samples = 100000;
  std::vector<int> counts(10, 0);
  for (int i = 0; i < num_samples; ++i) {
    ++counts[static_cast<size_t>(rng.GetInt(0, 9))];
  }

  const double expected = static_cast<double>(num_samples) / 10.0;
  const double max_rel_error = 0.08; 
  for (size_t i = 0; i < counts.size(); ++i) {
    const double rel_error = std::abs(counts[i] - expected) / expected;
    INFO("bucket " << i << ": count=" << counts[i] << " expected=" << expected
                   << " rel_error=" << rel_error);
    CHECK(rel_error <= max_rel_error);
  }
}

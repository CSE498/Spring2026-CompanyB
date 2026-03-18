#include "../../source/tools/Random.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>

TEST_CASE("Random basic range behavior", "[random]") {
  cse498::Random rng(12345);

  SECTION("nextDouble defaults to [0, 1)") {
    for (int i = 0; i < 2000; ++i) {
      double v = rng.nextDouble();
      CHECK(v >= 0.0);
      CHECK(v < 1.0);
    }
  }

  SECTION("nextDouble stays in [min, max)") {
    for (int i = 0; i < 2000; ++i) {
      double v = rng.nextDouble(-2.5, 7.5);
      CHECK(v >= -2.5);
      CHECK(v < 7.5);
    }
  }

  SECTION("nextFloat defaults to [0, 1)") {
    for (int i = 0; i < 2000; ++i) {
      float v = rng.nextFloat();
      CHECK(v >= 0.0f);
      CHECK(v < 1.0f);
    }
  }

  SECTION("nextFloat stays in [min, max)") {
    for (int i = 0; i < 2000; ++i) {
      float v = rng.nextFloat(-3.0f, 2.0f);
      CHECK(v >= -3.0f);
      CHECK(v < 2.0f);
    }
  }

  SECTION("nextInt stays in [min, max]") {
    for (int i = 0; i < 4000; ++i) {
      int v = rng.nextInt(-4, 6);
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
      CHECK(a.nextInt(-10000, 10000) == b.nextInt(-10000, 10000));
    }
  }

  SECTION("reseeding rewinds sequence") {
    cse498::Random rng(1337);
    std::vector<int> first_pass;
    first_pass.reserve(100);

    for (int i = 0; i < 100; ++i) {
      first_pass.push_back(rng.nextInt(0, 1000000));
    }

    rng.reseed(1337);

    for (int i = 0; i < 100; ++i) {
      CHECK(rng.nextInt(0, 1000000) == first_pass[static_cast<size_t>(i)]);
    }
  }
}

TEST_CASE("Random bool edge cases and frequencies", "[random]") {
  cse498::Random rng(987654321);

  SECTION("bernoulli endpoints") {
    for (int i = 0; i < 1000; ++i) {
      CHECK_FALSE(rng.bernoulli(0.0));
      CHECK(rng.bernoulli(1.0));
    }
  }

  SECTION("bernoulli(0.7) frequency is close to expected") {
    const int num_samples = 60000;
    const double p = 0.7;
    int true_count = 0;
    for (int i = 0; i < num_samples; ++i) {
      if (rng.bernoulli(p)) ++true_count;
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
    ++counts[static_cast<size_t>(rng.nextInt(0, 9))];
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
// AI helped me with this test case
TEST_CASE("Random normal distribution sanity check", "[random]") {
  cse498::Random rng(314159);

  const int num_samples = 200000;
  const double expected_mean = 3.0;
  const double expected_stddev = 2.0;

  double sum = 0.0;
  double sum_sq = 0.0;
  for (int i = 0; i < num_samples; ++i) {
    const double x = rng.normal(expected_mean, expected_stddev);
    sum += x;
    sum_sq += x * x;
  }

  const double sample_mean = sum / static_cast<double>(num_samples);
  const double variance =
      (sum_sq / static_cast<double>(num_samples)) - (sample_mean * sample_mean);
  const double sample_stddev = std::sqrt(variance);

  INFO("sample_mean=" << sample_mean << " expected_mean=" << expected_mean);
  INFO("sample_stddev=" << sample_stddev
                        << " expected_stddev=" << expected_stddev);
  CHECK(std::abs(sample_mean - expected_mean) < 0.05);
  CHECK(std::abs(sample_stddev - expected_stddev) < 0.05);
}

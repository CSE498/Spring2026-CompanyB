#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <numbers>
#include <optional>

namespace cse498 {

/*
Class: Random
Brief: High quality, pseudo random number generator using Xoshiro256++
algorithm

 This class provides a fast, statistically robust random number generator
 suitable for simulations, games, and general purpose applications.

 Name Xoshiro because of the operations it performs:
 XOR, SHIFT, ROTATE on its states

Features:
    xoshiro256++ algorithm for excellent speed and statistical properties
    SplitMix64 initialization for proper state seeding
    Uniform distribution for integers, floats, and doubles
    Proper bit-level precision handling for floating-point types
    Bernoulli (coin-flip) and Gaussian helpers for common workflows

Usage:
    Random rng;  // Auto-seeds with current time
    Random rng(42);  // Seed with specific value for reproducibility
    double d = rng.nextDouble(0.0, 100.0);
    int i = rng.nextInt(1, 6);  // Dice roll
    bool b = rng.bernoulli(0.5);  // Fair coin flip
*/
class Random {
 public:
  // If no seed is provided, use wall-clock time.
  explicit Random(std::optional<uint64_t> seed = std::nullopt) {
    if (!seed.has_value()) {
      seed = static_cast<uint64_t>(
          std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }
    reseed(seed.value());
  }

  void reseed(uint64_t seed) {
    // Generate individual states with splitmix.
    uint64_t splitmix_seed = seed;
    state[0] = splitMix64(splitmix_seed);
    state[1] = splitMix64(splitmix_seed);
    state[2] = splitMix64(splitmix_seed);
    state[3] = splitMix64(splitmix_seed);
  }

  // Uniform in [0, 1)
  [[nodiscard]] double nextUnitDouble() {
    // 0x1.0p-53 is 1.0 / (2^53), which is the smallest double greater than 0.
    constexpr double kDoubleUnit = 0x1.0p-53;
    const uint64_t random_bits = nextUint64();
    // Drop 11 low bits because double has 53 bits of significand (64-11=53).
    return static_cast<double>(random_bits >> 11) * kDoubleUnit;
  }

  // Uniform in [0, 1)
  [[nodiscard]] float nextUnitFloat() {
    // 0x1.0p-24f is 1.0f / (2^24).
    constexpr float kFloatUnit = 0x1.0p-24f;
    const uint64_t random_bits = nextUint64();
    // Shift down to 24 bits for float significand (64-40=24).
    return static_cast<float>(random_bits >> 40) * kFloatUnit;
  }

  // Uniform in [min, max) for any floating-point type; defaults to [0, 1).
  template <std::floating_point T = double>
  [[nodiscard]] T nextReal(T min = T{0}, T max = T{1}) {
    assert(min <= max &&
           "The minimum value can't be larger than the maximum value.");
    if constexpr (std::is_same_v<T, float>) {
      return min + nextUnitFloat() * (max - min);
    } else {
      return min + nextUnitDouble() * (max - min);
    }
  }

  // Uniform in [min, max].
  [[nodiscard]] int nextInt(int min, int max) {
    assert(min <= max &&
           "The minimum value can't be larger than the maximum value.");
    // Use 64-bit range width to avoid overflow in the spread computation.
    const uint64_t range =
        static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;

    // NOTE: modulo range reduction can introduce slight bias; acceptable here.
    return min + static_cast<int>(nextUint64() % range);
  }

  // Returns true with probability in [0.0, 1.0].
  [[nodiscard]] bool bernoulli(double probability = 0.5) {
    assert(probability >= 0.0 && probability <= 1.0 &&
           "probability must be between 0 and 1");

    if (probability <= 0.0) return false;
    if (probability >= 1.0) return true;
    return nextUnitDouble() < probability;
  }

  // Gaussian distribution N(mean, stddev^2).
  [[nodiscard]] double normal(double mean = 0.0, double stddev = 1.0) {
    assert(stddev >= 0.0 && "stddev must be non-negative");
    if (stddev == 0.0) return mean;

    // Box-Muller transform to convert uniform random variables to standard
    // normal, then scale and shift.

    // AI helped me with the logic here
    double u1 = nextUnitDouble();
    while (u1 <= std::numeric_limits<double>::min()) {
      u1 = nextUnitDouble();
    }
    const double u2 = nextUnitDouble();
    const double z =
        std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * std::numbers::pi * u2);
    return mean + stddev * z;
  }

 private:
  static constexpr unsigned int kWordBits = 64;
  uint64_t state[4]{};
  std::optional<double> cached_normal_{};

  // SplitMix64 for initializing the state.
  // Recommended for initializing the state of xoshiro256++.
  // First constant is the golden ratio.
  // The rest are tried and tested for best performance
  static constexpr uint64_t splitMix64(uint64_t& x) {
    constexpr uint64_t kConst1 = 0x9e3779b97f4a7c15ULL;
    constexpr uint64_t kConst2 = 0xbf58476d1ce4e5b9ULL;
    constexpr uint64_t kConst3 = 0x94d049bb133111ebULL;

    uint64_t z = (x += kConst1);
    z = (z ^ (z >> 30)) * kConst2;
    z = (z ^ (z >> 27)) * kConst3;
    return z ^ (z >> 31);
  }

  static constexpr uint64_t rotL(uint64_t x, unsigned int k) {
    assert(k < kWordBits && "rotation amount must be in [0, 63]");
    return (x << k) | (x >> (kWordBits - k));
  }

  // xoshiro256++ algorithm.
  // Found implementation online and adapted for this class.
  [[nodiscard]] uint64_t nextUint64() {
    const uint64_t result = rotL(state[0] + state[3], 23) + state[0];
    const uint64_t t = state[1] << 17;

    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];

    state[2] ^= t;
    state[3] = rotL(state[3], 45);

    return result;
  }
};

}  // namespace cse498

/*
Sources and implementations:
https://github.com/Quuxplusone/Xoshiro256ss/blob/main/splitmix64.h
https://prng.di.unimi.it/
https://github.com/Reputeless/Xoshiro-cpp
*/

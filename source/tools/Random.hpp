#ifndef RANDOM_H
#define RANDOM_H

#include <cstdint>
#include <chrono>
#include <utility> 
#include <cassert>
#include <limits>

namespace cse498 {

/*
Class: Random
Brief: High quality, pseudo randomn number generator using Xoshiro256** algorithm

 This class provides a fast, statistically robust random number generator
 suitable for simulations, games, and general purpose applications.

 Name Xoshiro because of the operations it performs:
 XOR, SHIFT, ROTATE on its states
 
Features:
    xoshiro256** algorithm for excellent speed and statistical properties
    SplitMix64 initialization for proper state seeding
    Uniform distribution for integers, floats, and doubles
    Proper bit-level precision handling for floating-point types
  
Usage:
    Random rng;  // Auto-seeds with current time
    Random rng(42);  // Seed with specific value for reproducibility
    double d = rng.GetDouble(0.0, 100.0);
    int i = rng.GetInt(1, 6);  // Dice roll
    bool b = rng.GetBool(0.5);  // Fair coin flip

*/
class Random {
public:
    // Initialize with a seed. If 0, use the current time.
    explicit Random(uint64_t seed = 0) {
        if (seed == 0) {
            seed = static_cast<uint64_t>(
                std::chrono::high_resolution_clock::now()
                    .time_since_epoch()
                    .count()
            );
        }
        
        Seed(seed);
    }

    
    void Seed(uint64_t seed) {
        // Generate individual states with splitmix
        state[0] = SplitMix64(seed);
        state[1] = SplitMix64(seed);
        state[2] = SplitMix64(seed);
        state[3] = SplitMix64(seed);
    }

    // Returns a random double in the range [min, max).
    [[nodiscard]] double GetDouble(double min, double max) {
        /*
        Make sure that if the user mixed up the order, we would still be able
        to correctly generate a response
        */
        if (min > max) { 
            std::swap(min, max); 
        }
        
        // Convert strict 64-bit integer to a double in [0, 1)
        // 0x1.0p-53 is 1.0 / (2^53), which is the smallest double greater than 0
        constexpr double DOUBLE_UNIT = 0x1.0p-53; 
        const uint64_t r = Next();
        // Drop 11 low bits becaues double has 53 bits significand (64-11=53)
        // r>>11 ranges from 0 - 2^53 -> this get a number between 0-1
        const double unit = (r >> 11) * DOUBLE_UNIT; 
        
        return min + unit * (max - min);
    }

    // Returns a random integer in the range [min, max].
    [[nodiscard]] int GetInt(int min, int max) {
        if (min > max) {
             std::swap(min, max); 
        }
        
        // Use a 64-bit range to prevent overflow when calculating the spread
        const uint64_t range = static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;
        
        /*
        I have researched and found that there might be a little bias in this part
        but for our use cases this should be fine
        */
        // create offset from next % range, then add min to shift it to [min,max]
        return min + static_cast<int>(Next() % range);
    }

    // Returns true with the given probability [0.0, 1.0].
    [[nodiscard]] bool GetBool(double probability) {
        assert(probability >= 0.0 && probability <= 1.0 && "Probability must be between 0 and 1");
        
        if (probability <= 0.0) return false;
        if (probability >= 1.0) return true;
        
        return GetDouble(0.0, 1.0) < probability;
    }

    // Returns a random float in the range [min, max).
    [[nodiscard]] float GetFloat(float min, float max) {
       if (min > max) {
            std::swap(min, max);
        }

        // 0x1.0p-24f is 1.0f / (2^24)
        /*
        This has almost exactly the sane logic as GetDouble
        Since the significand is different on GetFloat, we use a different const
        to represent that number of bits that we shal be using
        */
        constexpr float FLOAT_UNIT = 0x1.0p-24f;
        const uint64_t r = Next();
        // Shift down to 24 bits for mantissa
        // Similar math to GetDouble, 64-40=24
        // Scale this down to 0,1 range
        const float unit = (r >> 40) * FLOAT_UNIT;
        // Offset the range, and then add min to create the correct random range
        return min + unit * (max - min); 
    }

private:
    uint64_t state[4]{};

    // SplitMix64 for initializing the state.
    // Recommended for initializing the state of xoshiro256**.
    // First constant is the golden ratio
    // CONST2 and CONST3 are numbers that have been tested thoroughly by others for optimization
    static uint64_t SplitMix64(uint64_t& x) {
        constexpr uint64_t CONST1 = 0x9e3779b97f4a7c15ULL;
        constexpr uint64_t CONST2 = 0xbf58476d1ce4e5b9ULL;
        constexpr uint64_t CONST3 = 0x94d049bb133111ebULL;

        uint64_t z = (x += CONST1);
        z = (z ^ (z >> 30)) * CONST2;
        z = (z ^ (z >> 27)) * CONST3;
        return z ^ (z >> 31);
    }

    // Rotate Left helper
    static inline uint64_t RotL(const uint64_t x, const int k) {
        return (x << k) | (x >> (64 - k));
    }

    // xoshiro256** algorithm.
    // Excellent speed and statistical properties.
    // Found implementation online
    uint64_t Next() {
        /*
        This implementation I found to be consistent with all implementation online
        also I confirmed this with LLM
        */
        const uint64_t result = RotL(state[0] + state[3], 23) + state[0];
        const uint64_t t = state[1] << 17;

        state[2] ^= state[0];
        state[3] ^= state[1];
        state[1] ^= state[2];
        state[0] ^= state[3];

        state[2] ^= t;
        state[3] = RotL(state[3], 45);

        return result;
    }
};

} // namespace cse498

#endif

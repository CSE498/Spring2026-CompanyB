/**
 * @file RobinHoodMapBenchmark.cpp
 * @brief Speed comparison: RobinHoodMap vs std::unordered_map
 * @author John Stouffer
 * @date 2026-2-2
 */

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <random>
#include <vector>
#include <iomanip>

#include "../RobinHoodMap.hpp"
#include "../RobinHoodMap.cpp"

using namespace std::chrono;

// Returns elapsed time in milliseconds
template<typename Func>
double timeIt(Func&& func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

void runBenchmark(int numElements) {
    std::cout << "\n=== Benchmark with " << numElements << " elements ===\n\n";
    
    // Generate random keys
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, numElements * 10);
    
    std::vector<int> keys(numElements);
    for (int& k : keys) {
        k = dist(gen);
    }
    
    RobinHoodMap<int, int> rhMap;
    std::unordered_map<int, int> stdMap;
    
    // Insert benchmark
    double rhInsert = timeIt([&]() {
        for (int k : keys) {
            rhMap.insert(k, k);
        }
    });
    
    double stdInsert = timeIt([&]() {
        for (int k : keys) {
            stdMap[k] = k;
        }
    });
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "INSERT:\n";
    std::cout << "  RobinHoodMap:     " << std::setw(10) << rhInsert << " ms\n";
    std::cout << "  std::unordered_map: " << std::setw(8) << stdInsert << " ms\n";
    std::cout << "  Ratio (std/RH):   " << std::setw(10) << stdInsert / rhInsert << "x\n\n";
    
    // Lookup benchmark (run multiple iterations to get measurable time)
    const int lookupIterations = 10;
    volatile int sink = 0;  // Volatile prevents optimization
    
    double rhLookup = timeIt([&]() {
        for (int i = 0; i < lookupIterations; ++i) {
            for (int k : keys) {
                auto [found, val] = rhMap.at(k);
                sink += val;
            }
        }
    });
    
    double stdLookup = timeIt([&]() {
        for (int i = 0; i < lookupIterations; ++i) {
            for (int k : keys) {
                auto it = stdMap.find(k);
                if (it != stdMap.end()) sink += it->second;
            }
        }
    });
    
    std::cout << "LOOKUP (" << lookupIterations << "x iterations):\n";
    std::cout << "  RobinHoodMap:     " << std::setw(10) << rhLookup << " ms\n";
    std::cout << "  std::unordered_map: " << std::setw(8) << stdLookup << " ms\n";
    std::cout << "  Ratio (std/RH):   " << std::setw(10) << stdLookup / rhLookup << "x\n\n";
    
    // Failed lookup benchmark (keys that don't exist)
    std::vector<int> missingKeys(numElements);
    for (int& k : missingKeys) {
        k = dist(gen) + numElements * 100;  // Likely not in map
    }
    
    double rhMiss = timeIt([&]() {
        for (int i = 0; i < lookupIterations; ++i) {
            for (int k : missingKeys) {
                auto [found, val] = rhMap.at(k);
                if (found) sink += val;
            }
        }
    });
    
    double stdMiss = timeIt([&]() {
        for (int i = 0; i < lookupIterations; ++i) {
            for (int k : missingKeys) {
                auto it = stdMap.find(k);
                if (it != stdMap.end()) sink += it->second;
            }
        }
    });
    
    std::cout << "LOOKUP MISS (" << lookupIterations << "x iterations):\n";
    std::cout << "  RobinHoodMap:     " << std::setw(10) << rhMiss << " ms\n";
    std::cout << "  std::unordered_map: " << std::setw(8) << stdMiss << " ms\n";
    std::cout << "  Ratio (std/RH):   " << std::setw(10) << stdMiss / rhMiss << "x\n\n";
    
    double rhRemove = timeIt([&]() {
        for (int k : keys) {
            rhMap.remove(k);
        }
    });
    
    double stdRemove = timeIt([&]() {
        for (int k : keys) {
            stdMap.erase(k);
        }
    });
    
    std::cout << "REMOVE:\n";
    std::cout << "  RobinHoodMap:     " << std::setw(10) << rhRemove << " ms\n";
    std::cout << "  std::unordered_map: " << std::setw(8) << stdRemove << " ms\n";
    std::cout << "  Ratio (std/RH):   " << std::setw(10) << stdRemove / rhRemove << "x\n";
}

int main() {
    std::cout << "\n\nRobinHoodMap vs std::unordered_map Benchmark\n";
    std::cout << "=============================================\n";
    
    runBenchmark(10000);
    runBenchmark(100000);
    runBenchmark(1000000);
    
    return 0;
}
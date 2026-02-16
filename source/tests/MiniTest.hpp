#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>

namespace MiniTest {
    struct TestRegistry {
        struct Test { std::string name; std::function<void()> func; };
        static std::vector<Test>& GetTests() {
            static std::vector<Test> tests;
            return tests;
        }
    };
    struct Registrar {
        Registrar(const char* name, std::function<void()> func) {
            TestRegistry::GetTests().push_back({name, func});
        }
    };
}

// MACRO MAGIC to ensure line numbers expand correctly
#define MT_CONCAT_IMPL(x, y) x##y
#define MT_CONCAT(x, y) MT_CONCAT_IMPL(x, y)

#define TEST_CASE(name, tags) \
void MT_CONCAT(TestFunc_, __LINE__)(); \
static MiniTest::Registrar MT_CONCAT(Reg_, __LINE__)(name, MT_CONCAT(TestFunc_, __LINE__)); \
void MT_CONCAT(TestFunc_, __LINE__)()

// Simple replacement for SECTION: Just runs the code block immediately
#define SECTION(name)

#define REQUIRE(cond) \
do { if (!(cond)) { \
std::cerr << "FAILED: " << #cond << " at line " << __LINE__ << std::endl; \
std::exit(1); \
} } while(0)

#define REQUIRE_NOTHROW(expr) \
do { try { expr; } catch (...) { \
std::cerr << "FAILED: Unexpected exception at line " << __LINE__ << std::endl; \
std::exit(1); \
} } while(0)

int main() {
    std::cout << "Running " << MiniTest::TestRegistry::GetTests().size() << " tests..." << std::endl;
    for (const auto& test : MiniTest::TestRegistry::GetTests()) {
        std::cout << "[RUN] " << test.name << "... ";
        test.func();
        std::cout << "OK" << std::endl;
    }
    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
}
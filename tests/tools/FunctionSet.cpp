// ATTRIBUTIONS: I used Chat GPT to generate the base test cases. I did modify them, however.

#include "catch2/catch.hpp"
#include "tools/FunctionSet.hpp"

TEST_CASE("FunctionSet - Construction and basic operations") {
    cse498::FunctionSet<int, int> fs;
    
    REQUIRE(fs.empty());
    REQUIRE(fs.size() == 0);
}

TEST_CASE("FunctionSet - Initializer list construction") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x * 2; },
        [](int x) { return x + 1; },
        [](int x) { return x - 1; }
    };
    
    REQUIRE(fs.size() == 3);
    REQUIRE_FALSE(fs.empty());
}

TEST_CASE("FunctionSet - Add functions") {
    cse498::FunctionSet<int, int> fs;
    
    fs.add([](int x) { return x * 2; });
    REQUIRE(fs.size() == 1);
    
    fs.add([](int x) { return x + 5; });
    REQUIRE(fs.size() == 2);
}

TEST_CASE("FunctionSet - Invoke all functions") {
    int counter = 0;
    cse498::FunctionSet<void, int> fs;
    
    fs.add([&counter](int x) { counter += x; });
    fs.add([&counter](int x) { counter += x * 2; });
    fs.add([&counter](int x) { counter += x * 3; });
    
    fs.invoke(5);
    
    REQUIRE(counter == 30);
}

TEST_CASE("FunctionSet - Call operator") {
    int sum = 0;
    cse498::FunctionSet<void, int> fs;
    
    fs.add([&sum](int x) { sum += x; });
    fs.add([&sum](int x) { sum += x; });
    
    fs(10);
    
    REQUIRE(sum == 20);
}

TEST_CASE("FunctionSet - Index access with at()") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x * 2; },
        [](int x) { return x + 1; }
    };
    
    REQUIRE(fs.at(0)(5) == 10);
    REQUIRE(fs.at(1)(5) == 6);
    REQUIRE_THROWS_AS(fs.at(2), std::out_of_range);
}

TEST_CASE("FunctionSet - Index access with operator[]") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x * 3; },
        [](int x) { return x - 2; }
    };
    
    REQUIRE(fs[0](4) == 12);
    REQUIRE(fs[1](4) == 2);
}

TEST_CASE("FunctionSet - invoke_catch with no errors") {
    int result = 0;
    cse498::FunctionSet<void, int> fs;
    
    fs.add([&result](int x) { result += x; });
    fs.add([&result]([[maybe_unused]] int x) { result *= 2; });
    
    auto outcome = fs.invoke_catch(5);
    
    REQUIRE(outcome.has_value());
    REQUIRE(result == 10);
}

TEST_CASE("FunctionSet - invoke_catch with errors") {
    cse498::FunctionSet<void, int> fs;
    
    fs.add([](int x) { if (x > 0) throw std::runtime_error("error"); });
    fs.add([]([[maybe_unused]] int x) { });
    fs.add([](int x) { if (x > 0) throw std::logic_error("error"); });
    
    auto outcome = fs.invoke_catch(5);
    
    REQUIRE_FALSE(outcome.has_value());
    REQUIRE(outcome.error().size() == 2);
    auto err = outcome.error();

    auto [index1, err1] = err[0];
    REQUIRE(index1 == 0);
    REQUIRE_THROWS_AS(std::rethrow_exception(err1), std::runtime_error);

    auto [index2, err2] = err[1];
    REQUIRE(index2 == 2); 
    REQUIRE_THROWS_AS(std::rethrow_exception(err2), std::logic_error);
}

TEST_CASE("FunctionSet - pop_at") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x; },
        [](int x) { return x * 2; },
        [](int x) { return x * 3; }
    };
    
    REQUIRE(fs.size() == 3);
    REQUIRE(fs.pop_at(1));
    REQUIRE(fs.size() == 2);
    REQUIRE(fs[1](5) == 15);
   
    auto res = fs.pop_at(10);
    REQUIRE(!res);
    REQUIRE(res.error() == cse498::FunctionSetError::IndexOutOfBounds);
}

TEST_CASE("FunctionSet - pop") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x; },
        [](int x) { return x * 2; }
    };
    
    REQUIRE(fs.size() == 2);
    fs.pop();
    REQUIRE(fs.size() == 1);
    REQUIRE(fs[0](5) == 5);
}

TEST_CASE("FunctionSet - clear") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x; },
        [](int x) { return x * 2; }
    };
    
    REQUIRE(fs.size() == 2);
    fs.clear();
    REQUIRE(fs.empty());
    REQUIRE(fs.size() == 0);
}

TEST_CASE("FunctionSet - front and back") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x * 10; },
        [](int x) { return x * 20; },
        [](int x) { return x * 30; }
    };
    
    REQUIRE(fs.front()(2) == 20);
    REQUIRE(fs.back()(2) == 60);
}

TEST_CASE("FunctionSet - iteration") {
    cse498::FunctionSet<int, int> fs = {
        [](int x) { return x * 1; },
        [](int x) { return x * 2; },
        [](int x) { return x * 3; }
    };
    
    int sum = 0;
    for (const auto& func : fs) {
        sum += func(10);
    }

    REQUIRE(sum == 60);
}

TEST_CASE("FunctionSet - empty iteration") {
    cse498::FunctionSet<void, int&> fs;
    
    int val = 0;
    for (const auto& func : fs) {
        func(val);
    }

    REQUIRE(val == 0);
}

TEST_CASE("FunctionSet - reference iteration") {
    cse498::FunctionSet<void, int&> fs;
    fs.add([](int& x) { x += 1; });
    fs.add([](int& x) { x += 2; });

    int val = 0;
    for (const auto& func : fs) {
        func(val);
    }

    REQUIRE(val == 3);
}

TEST_CASE("FunctionSet - multiple parameters") {
    cse498::FunctionSet<int, int, int> fs;
    
    fs.add([](int a, int b) { return a + b; });
    fs.add([](int a, int b) { return a * b; });
    
    REQUIRE(fs[0](3, 4) == 7);
    REQUIRE(fs[1](3, 4) == 12);
}

TEST_CASE("FunctionSet - void return type") {
    std::vector<int> results;
    cse498::FunctionSet<void, int> fs;
    
    fs.add([&results](int x) { results.push_back(x); });
    fs.add([&results](int x) { results.push_back(x * 2); });
    
    fs.invoke(5);
    
    REQUIRE(results.size() == 2);
    REQUIRE(results[0] == 5);
    REQUIRE(results[1] == 10);
}

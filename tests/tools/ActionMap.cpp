#include "catch2/catch.hpp"
#include <string>
#include "tools/ActionMap.hpp"

// Helper for repeated function generation w/o accidental reuse
std::function<double(int, double)> test_func_factory() {
  return [] (int a, double b) { return a + b; };
}

TEST_CASE("ActionMap basic functionality tests", "[ActionMap]") {
  // Note - Setup outside of the SECTION(...)s below is done once per test.
  // See https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md#test-cases-and-sections
  cse498::ActionMap<int, double, char> action_map;

  REQUIRE(action_map.empty());
  REQUIRE(action_map.size() == 0);

  auto result = action_map.register_callable("test_func", test_func_factory());


  REQUIRE(result.has_value());

  REQUIRE(action_map.size() == 1);
  REQUIRE_FALSE(action_map.empty());

  SECTION("map information is updated") {
    REQUIRE(action_map.exists("test_func"));
  }

  SECTION("entry can be removed") {
    auto result = action_map.deregister_callable("test_func");
    REQUIRE(result.has_value());

    REQUIRE((action_map.empty()
	     && action_map.size() == 0));
  }

  SECTION("new entry can be invoked") {
    auto result = action_map.invoke<double>("test_func", 1, 1.5);
    REQUIRE(result.has_value());
    REQUIRE_THAT(result.value(), Catch::Matchers::WithinRel(2.5));
  }

  SECTION("map can be cleared") {
    auto result = action_map.register_callable("test_func_2", test_func_factory());
    REQUIRE(result.has_value());

    REQUIRE(action_map.size() == 2);

    action_map.clear();

    REQUIRE((action_map.empty()
	     && action_map.size() == 0));

    REQUIRE_FALSE(action_map.exists("test_func"));
    REQUIRE_FALSE(action_map.exists("test_func_2"));
  }
}

TEST_CASE("ActionMap error cases", "[ActionMap]") {
  cse498::ActionMap<int, char, double> action_map;

  REQUIRE((action_map.empty()
	   && action_map.size() == 0));

  std::function<double(int, double)> test_func = [](int a, double b){return a+b; };
  auto result = action_map.register_callable("test_func", test_func_factory());

  REQUIRE(result.has_value());

  REQUIRE(action_map.size() == 1);
  REQUIRE_FALSE(action_map.empty());

  SECTION("invocation with too few arguments") {
    auto invoke_res = action_map.invoke<double>("test_func", 1);
    REQUIRE_FALSE(invoke_res.has_value());
    REQUIRE(invoke_res.error() == cse498::ActionMapErr::TOO_FEW_ARGS);
  }

  SECTION("invocation with too many arguments") {
    auto invoke_res = action_map.invoke<double>("test_func", 1, 1.5, 2.5);
    REQUIRE_FALSE(invoke_res.has_value());
    REQUIRE(invoke_res.error() == cse498::ActionMapErr::TOO_MANY_ARGS);
  }

  SECTION("invocation with incorrect argument type") {
    auto invoke_res = action_map.invoke<double>("test_func", 1.5, 1.5);
    REQUIRE_FALSE(invoke_res.has_value());
    REQUIRE(invoke_res.error() == cse498::ActionMapErr::INVALID_ARG_TYPE);
  }

  SECTION("invocation with incorrect return type") {
    auto invoke_res = action_map.invoke<int>("test_func", 1, 1.5);
    REQUIRE_FALSE(invoke_res.has_value());
    REQUIRE(invoke_res.error() == cse498::ActionMapErr::INVALID_RET_TYPE);
  }

  SECTION("function not found in map") {
    auto invoke_res = action_map.invoke<int>("invalid_func", 1, 1.5);
    REQUIRE_FALSE(invoke_res.has_value());
    REQUIRE(invoke_res.error() == cse498::ActionMapErr::CALLABLE_NOT_FOUND);
  }

  SECTION("function name already present") {
    auto insert_res = action_map.register_callable("test_func", test_func_factory());
    REQUIRE_FALSE(insert_res.has_value());
    REQUIRE(insert_res.error() == cse498::ActionMapErr::NAME_EXISTS);
  }
}

TEST_CASE("ActionMap compile-time behavior", "[ActionMap]") {
  cse498::ActionMap<int, char, double> action_map;

  SECTION("validate permissible types") {
    STATIC_REQUIRE(action_map.is_valid_type<int>());
    STATIC_REQUIRE(action_map.is_valid_signature<char, int, char, double, char>());
  }

  SECTION("reject impermissible types") {
    STATIC_REQUIRE_FALSE(action_map.is_valid_type<float>());
    STATIC_REQUIRE_FALSE(action_map.is_valid_signature<char, int, char, float, char>());
  }
}

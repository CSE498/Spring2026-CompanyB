#include "core/Step.hpp"

#include "catch2/catch.hpp"

TEST_CASE("InfoStep basic functionality", "[InfoStep]") {
  using namespace steps;
  InfoHandler test_handler(
      [](int i) -> std::expected<bool, StepErr> { return i == 0; });
  // InfoHandler test_handler(std::function<std::expected<bool, StepErr>(int)>(
  //     [](int i) -> std::expected<bool, StepErr> { return i == 0; }));
  SECTION("default unexpected correctly returned for the invalid types") {}
}

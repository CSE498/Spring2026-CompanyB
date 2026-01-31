#include <catch2/catch_test_macros.hpp>

#include "tools/io_utils.hpp"

TEST_CASE("LoadStream reads lines from stream", "[io_utils]") {
  std::istringstream input("x\ny\nz");
  auto result = cse498::LoadStream(input);

  REQUIRE(result.size() == 3);
  CHECK(result[0] == "x");
  CHECK(result[1] == "y");
  CHECK(result[2] == "z");
}

TEST_CASE("LoadStream handles empty stream", "[io_utils]") {
  std::istringstream input("");
  auto result = cse498::LoadStream(input);

  REQUIRE(result.empty());
}

TEST_CASE("LoadStream handles single line without newline", "[io_utils]") {
  std::istringstream input("one line");
  auto result = cse498::LoadStream(input);

  REQUIRE(result.size() == 1);
  CHECK(result[0] == "one line");
}

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <expected>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include "core.hpp"

namespace cse498 {
namespace matchers {
template <typename T>
struct OptNotNull : Catch::Matchers::MatcherGenericBase {
  bool match(std::optional<T> const &other) const { return other.has_value(); }

  std::string describe() const override { return "Has value (is not empty)"; }
};

struct ExpNotErr : Catch::Matchers::MatcherGenericBase {
  template <typename T, typename E>
  bool match(std::expected<T, E> const &other) const {
    return other.has_value();
  }

  std::string describe() const override { return "Has value (is not err)"; }
};

template <typename State>
struct VariantState : Catch::Matchers::MatcherGenericBase {
  template <typename... Types>
  bool match(std::variant<Types...> const &other) const {
    return std::holds_alternative<State>(other);
  }

  // Gotta be a better way, like piping matcher results or something
  template <typename... Types>
  bool match(std::optional<std::variant<Types...>> const &other) const {
    return std::holds_alternative<State>(other.value());
  }

  template <typename... Types, typename E>
  bool match(std::expected<std::variant<Types...>, E> const &other) const {
    return std::holds_alternative<State>(other.value());
  }

  std::string describe() const override {
    return std::format("Variant holds type '{}'", typeid(State).name());
  }
};

};  // namespace matchers
};  // namespace cse498

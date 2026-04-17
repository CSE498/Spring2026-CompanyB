#pragma once

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <concepts>
#include <expected>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

#include "core.hpp"

namespace cse498 {

namespace matchers {
enum class CheckExp {
  NO,
  SUCCESS,
  ERROR,
};

enum class CheckOpt {
  NO,
  YES,
};
template <typename T> struct OptNotNull : Catch::Matchers::MatcherGenericBase {
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

struct ExpIsErr : Catch::Matchers::MatcherGenericBase {
  template <typename T, typename E>
  bool match(std::expected<T, E> const &other) const {
    return !other.has_value();
  }

  std::string describe() const override {
    return "Has error (does not have value)";
  }
};

template <typename State, CheckExp DoCheckExp = CheckExp::NO>
struct VariantState : Catch::Matchers::MatcherGenericBase {
  template <typename... Types>
  bool match(std::variant<Types...> const &other) const {
    return std::holds_alternative<State>(other);
  }

  template <typename... Types>
  bool match(std::optional<std::variant<Types...>> const &other) const {
    return std::holds_alternative<State>(other.value());
  }

  template <typename... Types, typename E>
    requires(DoCheckExp == CheckExp::SUCCESS)
  bool match(std::expected<std::variant<Types...>, E> const &other) const {
    return std::holds_alternative<State>(other.value());
  }

  template <typename... Types, typename S>
    requires(DoCheckExp == CheckExp::ERROR)
  bool match(std::expected<S, std::variant<Types...>> const &other) const {
    return std::holds_alternative<State>(other.error());
  }

  template <typename S, typename E>
    requires(DoCheckExp == CheckExp::NO)
  bool match(std::expected<S, E> const &) const {
    return false;
  }

  std::string describe() const override {
    std::string_view exp_permission;
    if constexpr (DoCheckExp == CheckExp::NO) {
      exp_permission = "not checking within expected";
    } else if constexpr (DoCheckExp == CheckExp::SUCCESS) {
      exp_permission = "checking within expecteds' success state";
    } else if constexpr (DoCheckExp == CheckExp::ERROR) {
      exp_permission = "checking within expecteds' error state";
    }

    return std::format("Variant ({}) holds type '{}'", exp_permission,
                       typeid(State).name());
  }
};

template <typename T>
concept VariantLike = requires(T t) { std::variant_size_v<T>; };

template <typename State, CheckExp DoCheckExp = CheckExp::NO>
  requires std::equality_comparable<State>
struct VariantHas : Catch::Matchers::MatcherGenericBase {
  State m_Cmp; // Value to compare to

  VariantHas(State cmp) : m_Cmp(cmp) {};

  template <VariantLike V> bool match(V const &other) const {
    return (std::get<State>(other) == m_Cmp);
  }

  template <VariantLike V> bool match(std::optional<V> const &other) const {
    return (std::get<State>(other.value()) == m_Cmp);
  }

  template <VariantLike V, typename E>
    requires(DoCheckExp == CheckExp::SUCCESS)
  bool match(std::expected<V, E> const &other) const {
    return (std::get<State>(other.value()) == m_Cmp);
  }

  template <VariantLike V, typename S>
    requires(DoCheckExp == CheckExp::ERROR)
  bool match(std::expected<S, V> const &other) const {
    return (std::get<State>(other.error()) == m_Cmp);
  }

  template <typename S, typename E>
    requires(DoCheckExp == CheckExp::NO)
  bool match(std::expected<S, E> const &) const {
    return false;
  }

  std::string describe() const override {
    std::string_view exp_permission;

    if constexpr (DoCheckExp == CheckExp::NO) {
      exp_permission = "not checking within expected";
    } else if constexpr (DoCheckExp == CheckExp::SUCCESS) {
      exp_permission = "checking within expecteds' success state";
    } else if constexpr (DoCheckExp == CheckExp::ERROR) {
      exp_permission = "checking within expecteds' error state";
    }

    std::string value{""};
    if constexpr (Concepts::Printable<State>) {
      value = std::format("(value == {})", m_Cmp);
    }

    return std::format("Variant ({}) holds correct current value {}",
                       exp_permission, value);
  }
};

}; // namespace matchers
}; // namespace cse498

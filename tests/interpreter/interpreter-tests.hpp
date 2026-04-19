#pragma once
#include "Interpreter/errors.hpp"
#include "core.hpp"
#include <catch2/catch_test_macros.hpp>
#include <expected>
#include <format>
#include <sstream>

// https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md#top

namespace Catch {
template <typename T> struct StringMaker<std::expected<T, cse498::InterpErr>> {
  static std::string convert(std::expected<T, cse498::InterpErr> const &v) {
    std::string val_str{"[V:Unprintable]"};
    if constexpr (Concepts::Printable<T>) {
      val_str = (std::stringstream() << "[" << v.value_or("") << "]").to_str();
    }
    return (v.has_value() ? val_str : std::format("[{}]", v.error().ToStr()));
  }
};
}; // namespace Catch

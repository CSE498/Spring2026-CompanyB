#pragma once

#include <type_traits>
namespace Concepts {
  template <typename T, typename ...Ts>
  concept IsOneOf = (std::is_same_v<T, Ts> || ...);
}

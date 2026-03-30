#pragma once

#include <functional>

// TODO : Update for possible
namespace FunInfo {
// Adapted from https://godbolt.org/z/of7r563P8 and
// https://github.com/devosoft/Empirical/blob/fab89b252a8da6d2c6fdf628654c2342e1eb014b/include/emp/meta/FunInfo.hpp

template <typename T, size_t Idx = 0>
struct nth_arg {
  using type = std::tuple_element_t<Idx, T>;
};

template <>
struct nth_arg<std::tuple<>> {
  using type = void;
};

template <typename T>
struct memfun_type {
  using type = void;
};

template <typename Ret, typename Class, typename... Args>
struct memfun_type<Ret (Class::*)(Args...) const> {
  using func = std::function<Ret(Args...)>;
  using ret = Ret;
  using params = std::tuple<Args...>;
  using first_param = std::tuple_element<0, params>;
};

template <typename F>
using FunInfo = memfun_type<decltype(&F::operator())>;

};  // namespace FunInfo

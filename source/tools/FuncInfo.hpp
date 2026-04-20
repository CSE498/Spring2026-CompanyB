#pragma once

#include <functional>
#include <tuple>

// Adapted from
// https://stackoverflow.com/questions/66961406/c-variant-visit-overloaded-function
// https://github.com/devosoft/Empirical/blob/fab89b252a8da6d2c6fdf628654c2342e1eb014b/include/emp/meta/FunInfo.hpp#LL72
// See a minimal example at https://godbolt.org/z/dvrMMvnar
namespace FuncInfo {
template <typename T>
struct memfun_type {
  using type = void;
};

template <typename Ret, typename Class, typename... Args>
struct memfun_type<Ret (Class::*)(Args...) const> {
  using func = std::function<Ret(Args...)>;
  using args = std::tuple<Args...>;
  using ret = Ret;
};

template <typename F>
using FuncInfo = memfun_type<decltype(&F::operator())>;
}  // namespace FuncInfo

#pragma once

#include <algorithm>
#include <concepts>
#include <expected>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace cse498 {

namespace TemplTools {
/**
 * @brief Valid only if `T` is of the same type of any type in `...Ts`.
 *
 * @tparam T Type to check against permissible types
 * @tparam Ts Permissible types
 */
template <typename T, typename... Ts>
concept IsOneOf = (std::same_as<T, Ts> || ...);

/** @brief Empty type to represent the end of a sequence of types. */
struct NullType {};

/**
 * @brief Returns `true` if all types in `Head` and `Tail...` are unique.
 *
 * @tparam Head First element in the sequence of types
 * @tparam ...Tail Remaining elements in the sequence of types
 * @return bool
 */
template <typename Head, typename... Tail>
constexpr bool all_unique() {
  // On final element, must be unique
  if constexpr (std::is_same<std::tuple<Tail...>, std::tuple<NullType>>()) {
    return true;
  } else if constexpr (IsOneOf<Head, Tail...>) {
    return false;
  } else {
    // Current Head is unique to Tail... so recurse
    return all_unique<Tail...>();
  }
}

/** @brief Wraps `all_unique()` to provide a corresponding concept requirement.
 */
template <typename... Ts>
concept UniqueTypes = all_unique<Ts..., NullType>();
};  // namespace TemplTools

enum class ActionMapErr {
  CALLABLE_NOT_FOUND,
  INVOCATION_ERR,
  TOO_FEW_ARGS,
  TOO_MANY_ARGS,
  INVALID_ARG_TYPE,
  INVALID_RET_TYPE,
  NAME_EXISTS,
};

/**
 * @brief A map matching input functions to a string name, allowing
 * invocation by name & arguments.
 *
 * Given a set of permissible types, stores & maps strings
 * to callable objects (std::function). Permits any function signature
 * which takes in and returns only types present in the set of
 * permissible types.
 *
 * @tparam Types All permissible types. Types must be copyable and unique.
 */
template <TemplTools::UniqueTypes... Types>
  requires(std::is_copy_constructible_v<Types> && ...)
class ActionMap {
  /** @brief Alias to std::variant, permits all types specified in `Types`. */
  using TypeVariant = decltype(std::variant<Types...>());

  /**
   * @brief Dataclass to store registered function entries.
   *
   * Contains:
   * - The original function wrapped in a lambda
   * - Index of variant alternative for return type in TypeVariant
   * - std::vector of indices of variant alternatives for parameter types
   *   in TypeVariant
   *
   * Default and copy ctors are deleted restricting usage
   * to moves, minimizing copies.
   */
  struct FuncEntry {
    std::function<TypeVariant(std::vector<TypeVariant> &&)> wrapped_func;
    size_t ret_t_idx;
    std::vector<size_t> arg_t_idxs;

    FuncEntry() = delete;
    FuncEntry(
        std::function<TypeVariant(std::vector<TypeVariant> &&)> wrapped_func,
        size_t ret_t_idx, std::vector<size_t> &&arg_t_idxs)
        : wrapped_func(std::move(wrapped_func)),
          ret_t_idx(ret_t_idx),
          arg_t_idxs(std::move(arg_t_idxs)){};

    // Delete copy ctors
    FuncEntry(const FuncEntry &) = delete;
    FuncEntry &operator=(const FuncEntry &) = delete;

    // Permit moving
    FuncEntry &operator=(FuncEntry &&) = default;
    FuncEntry(FuncEntry &&) = default;
  };

  std::unordered_map<std::string, FuncEntry> funcs{};

  // Adapted from
  // https://stackoverflow.com/questions/28410697/c-convert-vector-to-tuple See
  // arg_tuple doc comment
  template <typename V, typename AsTuple, size_t... Is>
  static AsTuple gen_arg_tuple(std::vector<V> &&args,
                               std::index_sequence<Is...>) {
    return std::make_tuple(
        std::get<typename std::tuple_element<Is, AsTuple>::type>(args[Is])...);
  }

  /**
   * @brief Construct std::tuple out of a std::vector of TypeVariants.
   *
   * Adapts technique at
   * https://stackoverflow.com/questions/28410697/c-convert-vector-to-tuple to
   * convert a std::vector of variants into a std::tuple so that we can utilize
   * std::apply() later. Uses the information known at the time of callable
   * registration to instantiate this template.
   *
   * @tparam L Length of resulting tuple
   * @tparam V Variant type for some ActionMap object
   * @tparam Ts Sequence of types representing the resulting tuple
   * @param args std::vector<V> of arguments wrapped in V
   * @return std::tuple<Ts...>
   */
  template <size_t L, typename V, typename... Ts>
  static std::tuple<Ts...> arg_tuple(std::vector<V> &&args) {
    return gen_arg_tuple<V, std::tuple<Ts...>>(std::move(args),
                                               std::make_index_sequence<L>{});
  }

  /**
   * @brief Returns the index of the variant alternative for a given type.
   *
   * @tparam TargetType Type for which to retrieve index within TypeVariant.
   * @tparam idx Current index for recursions to track. Should not be set when
   * called.
   * @return size_t
   */
  template <TemplTools::IsOneOf<Types...> TargetType, size_t idx = 0>
  constexpr size_t variant_index() const {
    if constexpr (std::is_same_v<TargetType, std::variant_alternative_t<
                                                 idx, TypeVariant>>) {
      return idx;
    } else {
      return variant_index<TargetType, idx + 1>();
    }
  }

  /**
   * @brief Returns std::vector of indices of variant alternatives within
   * TypeVariant.
   *
   * @tparam Params... Types to include within the resulting std::vector
   * @return std::vector<size_t>
   */
  template <TemplTools::IsOneOf<Types...>... Params>
  std::vector<size_t> param_sig_vec() const {
    return std::vector<size_t>{variant_index<Params>()...};
  }

 public:
  // Compile time helpers
  /**
   * @brief Determine whether given type is valid for this ActionMap.
   *
   * @tparam Type to validate
   * @return bool
   */
  template <typename T>
  constexpr bool is_valid_type() const noexcept {
    return TemplTools::IsOneOf<T, Types...>;
  }

  /**
   * @brief Determine whether given function signature is valid for
   * this ActionMap.
   *
   * @tparam Ret Function's return type
   * @tparam Args... Function's argument types
   * @return bool
   */
  template <typename Ret, typename... Args>
  constexpr bool is_valid_signature() const noexcept {
    return (TemplTools::IsOneOf<Ret, Types...> &&
            (TemplTools::IsOneOf<Args, Types...> && ...));
  }

  // Methods
  /** @brief Retrieve current number of callables in map. */
  size_t size() const noexcept { return funcs.size(); }

  /**
   * @brief Determine if provided name is present in map.
   *
   * @param name Name to check for within map
   * @return bool
   */
  [[nodiscard]] bool exists(const std::string &name) const noexcept {
    return funcs.contains(name);
  }

  /**
   * @brief Remove all entries in the map.
   */
  void clear() noexcept { funcs.clear(); }

  /** @brief Determine if the map is empty. */
  [[nodiscard]] bool empty() const noexcept { return funcs.empty(); }

  /**
   * @brief Remove callable from map, if present.
   *
   * Returns std::expected:
   * - std::string giving provided name if successful
   * - ActionMapErr describing the error encountered
   *
   * @param name Name of entry to remove from map.
   * @return std::expected<std::string, ActionMapErr>
   */
  [[nodiscard]] std::expected<std::string, ActionMapErr> deregister_callable(
      const std::string &name) {
    auto it = funcs.find(name);

    if (it == funcs.end())
      return std::unexpected(ActionMapErr::CALLABLE_NOT_FOUND);

    funcs.erase(it);
    return name;
  }

  /**
   * @brief Invoke function if present and return result.
   *
   * Returns std::expected:
   * - Ret giving the result of the function invocation
   * - ActionMapErr describing the error encountered otherwise
   *
   * Because the functions currently present in the map cannot be known at
   * compilation time, the return type cannot be deduced, requiring `Ret` to be
   * specified.
   *
   * @tparam Ret Return type of the function to invoke
   * @param name Name of entry to invoke
   * @param args... Arguments to pass to the invoked function.
   * @return std::expected<Ret, ActionMapErr>
   */
  template <TemplTools::IsOneOf<Types...> Ret,
            TemplTools::IsOneOf<Types...>... Args>
  [[nodiscard]] std::expected<Ret, ActionMapErr> invoke(const std::string &name,
                                                        Args... args) const {
    const auto it = funcs.find(name);

    if (it == funcs.cend())
      return std::unexpected(ActionMapErr::CALLABLE_NOT_FOUND);

    const FuncEntry &target_func = it->second;

    // Lift all supplied args into the TypeVariant and collect into a vec
    std::vector<TypeVariant> arg_list{args...};
    std::vector<size_t> arg_types = param_sig_vec<Args...>();

    // Verify function signature is as expected
    if (!(arg_list.size() == target_func.arg_t_idxs.size()))
      return (arg_list.size() < target_func.arg_t_idxs.size())
                 ? std::unexpected(ActionMapErr::TOO_FEW_ARGS)
                 : std::unexpected(ActionMapErr::TOO_MANY_ARGS);

    if (!(variant_index<Ret>() == target_func.ret_t_idx))
      return std::unexpected(ActionMapErr::INVALID_RET_TYPE);

    if (!(std::equal(arg_types.cbegin(), arg_types.cend(),
                     target_func.arg_t_idxs.cbegin())))
      return std::unexpected(ActionMapErr::INVALID_ARG_TYPE);

    TypeVariant output{
        std::invoke(target_func.wrapped_func, std::move(arg_list))};

    return std::get<Ret>(std::move(output));
  }

  /**
   * @brief Register function into map if possible.
   *
   * Moves given function into a lambda which takes in a `std::vector`
   * of arguments wrapped in `TypeVariant`, and inserts this lambda
   * into map. Leaves given function in unspecified state after registration.
   *
   * Leverages the fact that the full function signature is known at
   * the time that this method is called to supply the argument tuple
   * generation and to store signature types.
   *
   * Returns std::expected:
   * - void if successful
   * - ActionMapErr describing the error encountered
   *
   * @param name The name for which the function should map to
   * @param func The `std::function` object to insert into the table.
   * @return std::expected<void, ActionMapErr>
   */
  template <TemplTools::IsOneOf<Types...> Ret,
            TemplTools::IsOneOf<Types...>... Args>
  [[nodiscard]] std::expected<void, ActionMapErr> register_callable(
      const std::string &name, std::function<Ret(Args...)> &&func) {
    if (exists(name)) return std::unexpected(ActionMapErr::NAME_EXISTS);

    // Create function object w/ a lambda wrapping the original function
    std::function<TypeVariant(std::vector<TypeVariant> &&)> wrapped_func =
        [func = std::move(func)](std::vector<TypeVariant> &&args) {
          // Convert given vector of variants to a tuple of args matching
          // ...Args
          std::tuple<Args...> arg_tuple_vars =
              arg_tuple<sizeof...(Args), TypeVariant, Args...>(std::move(args));

          return std::apply(func, arg_tuple_vars);
        };

    funcs.try_emplace(name, std::move(wrapped_func), variant_index<Ret>(),
                      std::move(param_sig_vec<Args...>()));

    return {};
  }
};
};  // namespace cse498

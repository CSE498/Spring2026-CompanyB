#pragma once

#include <stdlib.h>

#include <type_traits>
#include <variant>

namespace Concepts {
/** @brief Require that the type `T` is equivalent to any of the types `...Ts`
 */
template <typename T, typename... Ts>
concept IsOneOf = (std::is_same_v<T, Ts> || ...);

/** @brief Helper compile-time call to check if all types in a list of types are
 * unique. */
template <typename Head, typename... Tail> constexpr bool all_unique() {
  // On final element, must be unique
  if constexpr (std::is_void_v<std::tuple<Tail...>>()) {
    return true;
  } else if constexpr (IsOneOf<Head, Tail...>) {
    return false;
  } else {
    // Current Head is unique to Tail... so recurse
    return all_unique<Tail...>();
  }
}

/** @brief Wraps `all_unique()` to provide a corresponding concept requirement.
 * Requires that all types in `...Ts` are unique.
 */
template <typename... Ts>
concept UniqueTypes = all_unique<Ts..., std::void_t>();

} // namespace Concepts

namespace StaticUtil {
/**
 * @brief Returns the index of the variant alternative for a given type in a
 * given variant.
 *
 * Requires that all types in variant are unique, otherwise a 1-1 mapping
 * between type and index cannot be formed.
 *
 * @tparam Variant The variant type to check
 * @tparam TargetType Type for which to retrieve index within TypeVariant.
 * @tparam idx Current index for recursions to track. Should not be set when
 * called.
 * @return size_t
 */

template <typename... VariantTypes, std::variant<VariantTypes...> Variant,
          typename TargetType, size_t idx = 0>
  requires Concepts::UniqueTypes<VariantTypes...>
constexpr size_t variant_index() {
  if constexpr (std::is_same_v<TargetType, std::variant_alternative_t<
                                               idx, decltype(Variant)>>) {
    return idx;
  } else {
    return variant_index<Variant, TargetType, idx + 1>();
  }
}
}; // namespace StaticUtil

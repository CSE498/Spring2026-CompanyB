#pragma once

#include <stdlib.h>

#include <type_traits>
#include <variant>

#include "AgentData.hpp"

namespace Concepts {
template <typename T, typename... Ts>
concept IsOneOf = (std::is_same_v<T, Ts> || ...);

template <typename Head, typename... Tail>
constexpr bool all_unique() {
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
 */
template <typename... Ts>
concept UniqueTypes = all_unique<Ts..., std::void_t>();

/**
 * @brief Determines a valid dataclass for the agents and world
 * @note REPLACE THE INT WITH THE LIST OF DATACLASSES FOR BOTH WORLDS i.e
 * IsOneOf<T, TrafficData, DiseaseData>
 * @note Feel free to add other constraints on what a Dataclass should have
 * after discussing with both world groups
 */
template <typename T>
concept IsDataClass = IsOneOf<T, cse498::TrafficData, cse498::DiseaseData>;

}  // namespace Concepts

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
};  // namespace StaticUtil

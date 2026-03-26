#pragma once

#include "core.hpp"
// #include <catch2/catch.hpp>
#include <expected>
#include <variant>
#include <vector>
#include <optional>
#include <string>

namespace cse498 {

/* Gonna redefine things that I know are already
built, this will be updated once this is more than a POC */
namespace mock {
struct Location {
  int x;
  int y;
};
}; // namespace mock

namespace steps {

using InfoType = std::variant<int, double, bool>;

  /* TODO
    */
template <Concepts::IsOneOf<int, double, bool> T, typename Err,
          typename TypeMismatchErr>
struct InfoHandler {
  using Ret = std::expected<bool, Err>;

  std::function<Ret(T)> valid_t_handler;

  InfoHandler(std::function<Ret(T)> handler) : valid_t_handler(handler) {};

  template <Concepts::IsOneOf<int, double, bool> I> Ret operator()(I i) {
    if constexpr(std::is_same_v<I, T>) {
      return std::invoke(valid_t_handler, i);
    } else {
      return std::unexpected(TypeMismatchErr());
    }
  }
};

struct StepErr {
  enum class Kind { EXAMPLE };

  Kind kind;
  std::string msg;
};

struct MovementStep {
  mock::Location loc;
};
struct InfoStep {
  enum class Aspect {
    OCCUPANCY_RAW,  // How many in area?
    OCCUPANCY_FRAC, // How much of area is occupied?
    LOC_AVAIL,      // Is specific spot available?
  };

  Aspect aspect;
  InfoType type;
};
struct ConditionalStep {
  std::optional<size_t> t_body;
  std::optional<size_t> f_body;
  // TODO -- How to store condition?
};
struct ReconStep {
  // TODO (probably gonna scrap)
};

template <typename T>
concept StepKind =
    Concepts::IsOneOf<T, MovementStep, InfoStep, ConditionalStep, ReconStep>;

using Step = std::variant<MovementStep, InfoStep, ConditionalStep, ReconStep>;

struct StepContainer {
  std::vector<Step> steps;

  // Probably shouldn't be void, later fix
  // This way, "creating" a step sequence should only really involve interacting
  // with the stepcontainer
  template <StepKind S, typename... Args> void add_step(Args &&...a) {
    this->steps.push_back(
        Step{std::in_place_type<S>, std::forward<Args>(a)...});
  }
};

}; // namespace steps

}; // namespace cse498
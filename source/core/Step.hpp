#pragma once

#include <cassert>
#include <expected>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "../tools/FuncInfo.hpp"
#include "WorldPosition.hpp"
#include "core.hpp"

namespace cse498 {
namespace steps {

using InfoType = std::variant<int, double, bool>;

template <typename T>
concept IsInfoType = Concepts::IsOneOf<T, int, double, bool>;

struct StepErr {
  enum class Kind {
    WRONG_TYPE,
    HANDLER_NOT_SET,
    STEPS_EXHAUSTED,
    NOT_INFORMED,
    MISSING_BRANCH,
  };

  Kind kind;
  std::string msg;

  StepErr() = delete;
  StepErr(Kind in_kind) : kind(in_kind), msg("") {};
  StepErr(Kind in_kind, std::string const &in_msg)
      : kind(in_kind), msg(in_msg) {};
};

template <IsInfoType I>
using InfoFunc = std::function<std::expected<bool, StepErr>(I)>;

template <IsInfoType... Is> using InfoFuncTuple = std::tuple<InfoFunc<Is>...>;

// Parameterize the info handler so that we can extremely easily add more types
// later
template <typename... Ts> struct _InfoHandler {
  // Generate the default handler function
  template <IsInfoType T> static InfoFunc<T> defaulted_handler() {
    return [](T) {
      return std::unexpected(
          StepErr(StepErr::Kind::WRONG_TYPE,
                  "InfoStep passed invalidly typed InfoType into InfoHandler"));
    };
  }

  // Pick and return the passed in func if types match, otherwise generate the
  // default
  template <IsInfoType Target, IsInfoType Current>
  static InfoFunc<Current> pick_handler(InfoFunc<Target> f) {
    if constexpr (std::is_same_v<Target, Current>) {
      return f;
    } else {
      return defaulted_handler<Current>();
    }
  }

  // Holds the defaulted handlers which will get one function overwritten
  InfoFuncTuple<Ts...> funcs;

  // Allows passing in a lambda
  template <typename F>
  _InfoHandler(F f)
      : funcs({pick_handler<typename std::tuple_element<
                                0, typename FuncInfo::FuncInfo<F>::args>::type,
                            Ts>(f)...}) {}

  template <typename S> std::expected<bool, StepErr> operator()(S s) {
    return std::invoke(std::get<InfoFunc<S>>(funcs), s);
  }
};

// Now set our desired InfoHandler type and let the compiler handle the rest
using InfoHandler = _InfoHandler<int, double, bool>;

// Forward decl.
struct StepContainer;

struct MovementStep {
  WorldPosition loc;

  bool operator==(MovementStep const& other) { return other.loc == loc; }
};

struct InfoStep {
  enum class Aspect {
    OCCUPANCY_RAW,  // How many in area?
    OCCUPANCY_FRAC, // How much of area is occupied?
    LOC_AVAIL,      // Is specific spot available?
  };

  Aspect aspect;
  WorldPosition target;

  InfoStep() = delete;
  InfoStep(Aspect in_aspect, WorldPosition in_pos)
      : aspect(in_aspect), target(in_pos) {};

  bool operator==(InfoStep const &other) const {
    return (other.aspect == aspect) && (other.target == target);
  }
};

struct ConditionalStep {
  InfoHandler condition;

  bool operator==([[maybe_unused]] ConditionalStep const &other) const {
    // TODO: Decide how to compare functors
    return true;
  }
};

struct ReconStep {
  // TODO (probably gonna scrap)
  bool operator==([[maybe_unused]] ReconStep const &other) const {
    return false;
  }
};

template <typename T>
concept StepKind =
    Concepts::IsOneOf<T, MovementStep, InfoStep, ConditionalStep, ReconStep>;

using Step = std::variant<MovementStep, InfoStep, ConditionalStep, ReconStep>;

// Future optimization -- can shove this tree structure into a vec and save on
// cache misses

template <typename T>
concept BranchLike = (std::is_same_v<T, StepContainer> || StepKind<T>);

struct StepContainer {
  struct Node {
    std::optional<Step> step;
    std::unique_ptr<Node> next;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    Node() : step({}), next(nullptr), left(nullptr), right(nullptr) {}

    template <StepKind S>
    Node(S &&s)
        : step(std::move(s)), next(nullptr), left(nullptr), right(nullptr) {}
  };

  // Root is an "empty" node so that "last" can always be bound
  std::unique_ptr<Node> root = std::make_unique<Node>();
  // This is *non owning* and a shared_ptr doesn't make sense
  Node* last = root.get();

  // When iterating, the "indexing" ptr needs no mutation
  Node const* cur_node = root.get();
  // Need one node prior to handle infostep stuff
  Node const* prev_node = nullptr;

  std::stack<Node const*> next_stack;

  // Filled in by .inform()
  std::optional<InfoType> world_info = {};

  [[nodiscard]] bool empty() const {
    return (root == nullptr || root->next == nullptr);
  }

  [[nodiscard]] bool exhausted() const {
    return (next_stack.empty() && (cur_node == nullptr));
  }

  template <IsInfoType I> void inform(I const &info) {
    world_info = InfoType{std::in_place_type<I>, info};
  }

  template <StepKind S> void add_step(S &&s) {
    assert(last != nullptr);
    last->next = std::make_unique<Node>(std::move(s));
    last = last->next.get();
    assert(last->next == nullptr);
    assert(last != nullptr);
  }

  // TODO -- Template these add_step overloads so that we can also add a single
  // step as a branch w/o needing a full stepcontainer

  template <BranchLike T>
  void add_step(InfoStep&& i, ConditionalStep&& s, T&& t_body) {
    assert(last != nullptr);

    // Insert infostep node first
    add_step(std::move(i));

    // Insert ConditionalStep node
    add_step(std::move(s));

    if constexpr (std::is_same_v<T, StepContainer>) {
      // Move t_body's root to last's left child, then unset its last (don't
      // change our last)
      last->left = std::move(t_body.root);
      t_body.last = nullptr;
    } else {
      // T is just a step, so put it on last's left
      last->left = std::make_unique<Node>();
      last->left->next = std::make_unique<Node>(std::move(t_body));
    }

    assert(last != nullptr);
  }

  template <BranchLike T, BranchLike F>
  void add_step(InfoStep&& i, ConditionalStep&& s, T&& t_body, F&& f_body) {
    assert(last != nullptr);

    // Do everything the single-branch add_step does, then just additionally add
    // in f_body
    add_step(std::move(i), std::move(s), std::move(t_body));

    if constexpr (std::is_same_v<F, StepContainer>) {
      // Move f_body's root to last's right child, then unset its last (don't
      // change our last)
      last->right = std::move(f_body.root);
      f_body.last = nullptr;
    } else {
      // F is just a step, so put it on last's right
      last->right = std::make_unique<Node>();
      last->right->next = std::make_unique<Node>(std::move(f_body));
    }

    assert(last != nullptr);
  }

  [[nodiscard]] std::expected<Step, StepErr> get_next() {
    // TODO - This method is way over inclusive, should refactor down & out
    // later
    if (cur_node == nullptr) {
      // If next_stack isnt empty we have at min one return point to go back to
      if (!next_stack.empty()) {
        // We don't track the node prior to the return point because a
        // conditional following a conditional directly is a failure mode
        // properly checked for, so we'll just set prev_node null and let
        // conditional handling take care of it
        prev_node = nullptr;
        cur_node = next_stack.top();
        next_stack.pop();
        return get_next();
      } else
        return std::unexpected(
            StepErr(StepErr::Kind::STEPS_EXHAUSTED, "No remaining steps"));
    }

    // Skip empty step(s)
    if (!cur_node->step.has_value()) {
      prev_node = cur_node;
      cur_node = cur_node->next.get();
      return get_next();
    }

    // Future optimization -- can save on all this checking+casting by moving
    // step-dependent work out to a visitor and just passing the current step
    // into std::visit

    // Handle current node being conditional
    if (std::holds_alternative<ConditionalStep>(cur_node->step.value())) {
      // Our world_info object must have information
      if (!world_info.has_value())
        return std::unexpected(
            StepErr(StepErr::Kind::NOT_INFORMED,
                    "StepContainer was not informed for ConditionalStep"));

      ConditionalStep cur_step =
          std::get<ConditionalStep>(cur_node->step.value());

      // Guaranteed fine to do given the prior checks
      // Visit the given infohandler with the given world information
      std::expected<bool, StepErr> cond_result =
          std::visit(cur_step.condition, world_info.value());

      // Unset world_info now so we don't accidentally reuse it
      world_info = {};

      // Forward the error if encountered
      if (!cond_result.has_value())
        return std::unexpected(cond_result.error());

      if (cond_result.value()) {
        // Likely don't need this check, it *should* be impossible to have a
        // conditional step w/o a true branch
        if (cur_node->left == nullptr)
          return std::unexpected(StepErr(StepErr::Kind::MISSING_BRANCH,
                                         "ConditionalStep which evaluated true "
                                         "does not have a true branch"));

        // Push the return point onto the stack, then return the first step in
        // the true branch
        next_stack.push(cur_node->next.get());
        prev_node = cur_node;
        cur_node = cur_node->left.get();
        return get_next();
      } else {
        // If no false branch, we just continue to the next node
        if (cur_node->right == nullptr) {
          prev_node = cur_node;
          cur_node = cur_node->next.get();
          return get_next();
        }

        // Push return point onto stack, then ret first step on false branch
        next_stack.push(cur_node->next.get());
        prev_node = cur_node;
        cur_node = cur_node->right.get();
        return get_next();
      }
    }

    // The rest of the nodes don't require extra work
    Step ret_step = cur_node->step.value();
    prev_node = cur_node;
    cur_node = cur_node->next.get();
    return ret_step;
  }
};

}; // namespace steps
}; // namespace cse498

#include "core/Step.hpp"
#include "WorldPosition.hpp"
#include "Worlds/MazeWorld.hpp"
#include "test-tools/matchers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <sstream>
#include <string>
#include <variant>

using namespace cse498::steps;

TEST_CASE("StepContainer construction functionality", "[StepContainer]") {
  using cse498::WorldPosition;

  StepContainer steps{};

  REQUIRE(steps.empty());
  REQUIRE_FALSE(steps.root->step.has_value());

  SECTION("Single movement container") {
    steps.add_step(MovementStep{cse498::WorldPosition{0.5, 0.5}});

    REQUIRE_FALSE(steps.empty());
    REQUIRE(steps.last != nullptr);
    REQUIRE(steps.last->step.has_value());
    REQUIRE(std::holds_alternative<MovementStep>(steps.last->step.value()));
  }

  SECTION("Conditional movement") {
    // Go to {1.5, 1.5} if the spot is free, otherwise go to {1.5, -1.5}

    StepContainer t_branch{};
    StepContainer f_branch{};

    WorldPosition target{1.5, 1.5};
    WorldPosition fallback{1.5, -1.5};

    MovementStep target_move{target};
    MovementStep fallback_move{fallback};
    InfoStep can_move_info{InfoStep::Aspect::LOC_AVAIL, target};
    ConditionalStep can_move_cond{[](bool b) { return b; }};

    t_branch.add_step(std::move(target_move));
    f_branch.add_step(std::move(fallback_move));

    steps.add_step(std::move(can_move_info), std::move(can_move_cond),
                   std::move(t_branch), std::move(f_branch));

    REQUIRE_FALSE(steps.empty());

    // Make sure t_branch and f_branch were moved from
    REQUIRE(t_branch.empty());
    REQUIRE(t_branch.root == nullptr);
    REQUIRE(t_branch.last == nullptr);
    REQUIRE(f_branch.empty());
    REQUIRE(f_branch.root == nullptr);
    REQUIRE(f_branch.last == nullptr);

    // Check the steps are as expected
    REQUIRE_FALSE(steps.root == nullptr);
    REQUIRE_FALSE(steps.root->next == nullptr);
    REQUIRE(steps.root->next->step.has_value());

    // Future improvement -- this mess could be moved out to a custom Catch2
    // matcher

    StepContainer::Node const *checking_node = steps.root->next.get();

    // Should see InfoStep
    REQUIRE(checking_node->step.has_value());
    REQUIRE_THAT(checking_node->step,
                 cse498::matchers::OptNotNull<Step>() &&
                     cse498::matchers::VariantState<InfoStep>());

    InfoStep info_step = std::get<InfoStep>(checking_node->step.value());
    REQUIRE(info_step.target == cse498::WorldPosition{1.5, 1.5});
    REQUIRE(info_step.aspect == InfoStep::Aspect::LOC_AVAIL);

    REQUIRE_FALSE(checking_node->next == nullptr);

    checking_node = checking_node->next.get();

    // Should see ConditionalStep
    REQUIRE_THAT(checking_node->step,
                 cse498::matchers::OptNotNull<Step>() &&
                     cse498::matchers::VariantState<ConditionalStep>());

    // Note - leave testing InfoHandler correctness to the InfoHandler tests

    REQUIRE_FALSE(checking_node->left == nullptr);
    REQUIRE_FALSE(checking_node->left->next == nullptr);
    REQUIRE_FALSE(checking_node->right == nullptr);
    REQUIRE_FALSE(checking_node->right->next == nullptr);

    // Check left branch correctness
    REQUIRE_THAT(checking_node->left->next->step,
                 cse498::matchers::OptNotNull<Step>() &&
                     cse498::matchers::VariantState<MovementStep>());

    REQUIRE(
        std::get<MovementStep>(checking_node->left->next->step.value()).loc ==
        target);
    REQUIRE(checking_node->left->next->next == nullptr);

    // Check right branch correctness
    REQUIRE_THAT(checking_node->right->next->step,
                 cse498::matchers::OptNotNull<Step>() &&
                     cse498::matchers::VariantState<MovementStep>());

    REQUIRE(
        std::get<MovementStep>(checking_node->right->next->step.value()).loc ==
        fallback);
    REQUIRE(checking_node->right->next->next == nullptr);

    // No more steps after conditional
    REQUIRE(checking_node->next == nullptr);
  }
};

TEST_CASE("InfoStep and InfoHandler basic functionality", "[InfoStep]") {
  using namespace cse498::steps;
  InfoHandler test_handler(
      [](int i) -> std::expected<bool, StepErr> { return i == 0; });
  SECTION("default unexpected correctly returned for the invalid types") {}
};

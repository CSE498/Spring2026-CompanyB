#include "core/Step.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <sstream>
#include <string>
#include <variant>

#include "WorldPosition.hpp"
#include "Worlds/MazeWorld.hpp"
#include "test-tools/matchers.hpp"

using namespace cse498::steps;
using cse498::WorldPosition;
using namespace cse498::matchers;

TEST_CASE("StepContainer basic functionality", "[StepContainer]") {
  StepContainer steps{};

  REQUIRE(steps.empty());
  REQUIRE_FALSE(steps.root->step.has_value());

  SECTION("single movement") {
    steps.add_step(MovementStep{WorldPosition{0.5, 0.5}});

    SECTION("construction") {
      REQUIRE_FALSE(steps.empty());
      REQUIRE(steps.last != nullptr);
      REQUIRE(steps.last->step.has_value());
      REQUIRE(std::holds_alternative<MovementStep>(steps.last->step.value()));
    }

    SECTION("evaluation") {
      // First and only step should be the movement step
      std::expected<Step, StepErr> cur_step = steps.get_next();
      REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<MovementStep>());
      REQUIRE(std::get<MovementStep>(cur_step.value()).loc ==
              WorldPosition{0.5, 0.5});

      // Should be exhausted now
      cur_step = steps.get_next();
      REQUIRE_FALSE(cur_step.has_value());
      REQUIRE(cur_step.error().kind == StepErr::Kind::STEPS_EXHAUSTED);
      REQUIRE(steps.exhausted());
    }
  }

  SECTION("simple conditional") {
    StepContainer t_branch{};
    StepContainer f_branch{};

    WorldPosition target{1.5, 1.5};
    WorldPosition fallback{1.5, -1.5};

    MovementStep target_move{target};
    MovementStep fallback_move{fallback};
    InfoStep can_move_info{InfoStep::Aspect::LOC_AVAIL, target};
    ConditionalStep can_move_cond{[](bool b) { return b; }};

    SECTION("construction by stepcontainer-consumption") {
      t_branch.add_step(std::move(target_move));
      f_branch.add_step(std::move(fallback_move));

      steps.add_step(std::move(can_move_info), std::move(can_move_cond),
                     std::move(t_branch), std::move(f_branch));
      SECTION("construction") {
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

        REQUIRE(std::get<MovementStep>(checking_node->left->next->step.value())
                    .loc == target);
        REQUIRE(checking_node->left->next->next == nullptr);

        // Check right branch correctness
        REQUIRE_THAT(checking_node->right->next->step,
                     cse498::matchers::OptNotNull<Step>() &&
                         cse498::matchers::VariantState<MovementStep>());

        REQUIRE(std::get<MovementStep>(checking_node->right->next->step.value())
                    .loc == fallback);
        REQUIRE(checking_node->right->next->next == nullptr);

        // No more steps after conditional
        REQUIRE(checking_node->next == nullptr);
      }

      SECTION("evaluation") {
        std::expected<Step, StepErr> cur_step = steps.get_next();
        REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<InfoStep>());
        REQUIRE(std::get<InfoStep>(cur_step.value()) ==
                InfoStep{InfoStep::Aspect::LOC_AVAIL, target});

        SECTION("True branch") {
          steps.inform(true);
          cur_step = steps.get_next();
          REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<MovementStep>());
          REQUIRE(std::get<MovementStep>(cur_step.value()).loc == target);
        }
        SECTION("False branch") {
          steps.inform(false);
          cur_step = steps.get_next();
          REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<MovementStep>());
          REQUIRE(std::get<MovementStep>(cur_step.value()).loc == fallback);
        }

        cur_step = steps.get_next();
        // Should now be exhausted
        REQUIRE_FALSE(cur_step.has_value());
        REQUIRE(cur_step.error().kind ==
                cse498::steps::StepErr::Kind::STEPS_EXHAUSTED);
        REQUIRE(steps.exhausted());
      }
    }

    SECTION("construction by direct step") {
      steps.add_step(std::move(can_move_info), std::move(can_move_cond),
                     std::move(target_move), std::move(fallback_move));
      SECTION("construction") {
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

        REQUIRE(std::get<MovementStep>(checking_node->left->next->step.value())
                    .loc == target);
        REQUIRE(checking_node->left->next->next == nullptr);

        // Check right branch correctness
        REQUIRE_THAT(checking_node->right->next->step,
                     cse498::matchers::OptNotNull<Step>() &&
                         cse498::matchers::VariantState<MovementStep>());

        REQUIRE(std::get<MovementStep>(checking_node->right->next->step.value())
                    .loc == fallback);
        REQUIRE(checking_node->right->next->next == nullptr);

        // No more steps after conditional
        REQUIRE(checking_node->next == nullptr);
      }

      SECTION("evaluation") {
        std::expected<Step, StepErr> cur_step = steps.get_next();
        REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<InfoStep>());
        REQUIRE(std::get<InfoStep>(cur_step.value()) ==
                InfoStep{InfoStep::Aspect::LOC_AVAIL, target});

        SECTION("True branch") {
          steps.inform(true);
          cur_step = steps.get_next();
          REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<MovementStep>());
          REQUIRE(std::get<MovementStep>(cur_step.value()).loc == target);
        }
        SECTION("False branch") {
          steps.inform(false);
          cur_step = steps.get_next();
          REQUIRE_THAT(cur_step, ExpNotErr() && VariantState<MovementStep>());
          REQUIRE(std::get<MovementStep>(cur_step.value()).loc == fallback);
        }

        cur_step = steps.get_next();
        // Should now be exhausted
        REQUIRE_FALSE(cur_step.has_value());
        REQUIRE(cur_step.error().kind ==
                cse498::steps::StepErr::Kind::STEPS_EXHAUSTED);
        REQUIRE(steps.exhausted());
      }
    }
  }
}

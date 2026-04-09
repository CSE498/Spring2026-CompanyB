#pragma once

#include <concepts>
#include <random>

#include "../core/Step.hpp"
#include "../core/AgentBase.hpp"
#include "../tools/StateGridPosition.hpp"
#include "../core/WorldBase.hpp"
#include "../core/AgentData.hpp"

namespace cse498 {

template <typename T>
concept IsSwarm_Data = Concepts::IsOneOf<T, TrafficData, InfectionData>;

template <IsSwarm_Data SwarmData>
class SwarmingAgent : public AgentBase<SwarmData> {
 private:
  std::mt19937 rng{std::random_device{}()};

  // Returns a random neighboring position (up/down/left/right)
  WorldPosition get_random_neighbor(WorldPosition & pos) {
    std::uniform_int_distribution<int> dist(1, 4);
    switch (dist(rng)) {
      case 1:
        return pos.Up();  // up
      case 2:
        return pos.Down();  // down
      case 3:
        return pos.Left();  // left
      default:
        return pos.Right();  // right
    }
  }

 public:
  SwarmingAgent(SwarmData data) : AgentBase<SwarmData>(data) {}

  [[nodiscard]] StepContainer GetTurn() override {
    if constexpr (std::is_same_v<SwarmData, TrafficData>) {
      return TrafficGetTurn();
    } else if constexpr (std::is_same_v<SwarmData, InfectionData>) {
      return InfectionGetTurn();
    }
  }

  [[nodiscard]] StepContainer TrafficGetTurn() {
    StepContainer container{};

    if(!this->m_Data.is_active) {
      // If the agent is not active, it should do nothing (remain still)
      return container; // empty container means no steps, i.e. remain still
    }

    // I think we need some sort of conditional that says if we're at an
    // intersection do this, else continue straight kinda thing (not sure if the
    // world has this information tbh)

    // No target set — make a random turn and skip the world query entirely
    if (!this->m_Data.destination.has_value()) {
      WorldPosition random_pos = get_random_neighbor(this->m_Data.pos); //(3,1)
      cse498::steps::MovementStep random_move{random_pos};
      container.add_step(std::move(random_move));
      return container;
    }

    // Ask the world if the target position is available
    cse498::steps::InfoStep query{
        cse498::steps::InfoStep::Aspect::LOC_AVAIL,
        this->m_Data.destination.value()  // safe, we checked above
    };

    // Read the world's answer and branch: true = available, false = not
    cse498::steps::ConditionalStep has_target{cse498::steps::InfoHandler(
        [](bool is_available) -> std::expected<bool, cse498::steps::StepErr> {
          return is_available;
        })};



    
    // True branch: target is open, move toward it (checks if we swarm away or
    // to the location)
    cse498::steps::StepContainer true_branch{};
    if (this->m_Data.swarm_away) {
      // as of now it just moves randomly (need to implement logic)
      cse498::steps::MovementStep true_move{
          get_random_neighbor(this->m_Data.pos)};
      true_branch.add_step(std::move(true_move));
    } else {
      // as of now it sets move to point to the destination, not sure if this is
      // the correct logic or not
      cse498::steps::MovementStep true_move{this->m_Data.destination.value()};
      true_branch.add_step(std::move(true_move));
    }

    // False branch: target is blocked, move randomly (this likely will never be
    // called I dont think (unless its a "wall" ig))
    cse498::steps::StepContainer false_branch{};
    cse498::steps::MovementStep false_move{
        get_random_neighbor(this->m_Data.pos)};
    false_branch.add_step(std::move(false_move));

    // Wire it all together: query → condition → true/false branch
    container.add_step(std::move(query), std::move(has_target),
                       std::move(true_branch), std::move(false_branch));

    return container;
  }

  [[nodiscard]] StepContainer InfectionGetTurn() {
    // For now, just return an empty container (i.e. do nothing)
    return StepContainer{};
  }

  // Following 3 functions are helper functions to set aspects of the agent's state

  // Helper function to set the agent's destination
  void SetGoal(WorldPosition goal) {
    auto state = this->GetState();
    state.destination = goal;
    this->SetState(state);
  }

  // Helper function to set whether the agent is swarming away or towards the goal
  void SetSwarmAway(bool swarm_away) {
    auto state = this->GetState();
    state.swarm_away = swarm_away;
    this->SetState(state);
  }

  // Helper function to set whether the agent is active or not
  void SetActive(bool active) {
    auto state = this->GetState();
    state.is_active = active;
    this->SetState(state);
  }
};

}  // namespace cse498
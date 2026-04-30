#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../core/AgentData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"
#include "../core/StepWorldBase.hpp"
#include "../core/core.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {

/**
 * @brief Concept for state types supported by SwarmingAgent.
 *
 * SwarmingAgent currently supports traffic and infectious-disease data because
 * those state objects provide the fields its movement logic reads and updates.
 */

/**
 * @brief Agent that moves toward a destination while avoiding recently visited
 * cells.
 *
 * This agent implements a simple swarming behavior where it prefers to move
 * toward a target destination but also keeps track of recently visited cells to
 * avoid getting stuck in loops or congested areas. The agent scores neighboring
 * cells based on their distance to the target and how frequently they have been
 * visited, and then chooses the best option with some random tie-breaking.
 */
template <Concepts::IsDataClass SwarmData>
class SwarmingAgent : public StepAgentBase<SwarmData> {
  const double visit_penalty =
      1.75;  // Tunable parameter to adjust how strongly the agent avoids
             // revisiting cells
 private:
  /// Random generator used to choose among candidate neighboring cells.
  std::mt19937 rng{std::random_device{}()};

  /// Recent positions used to avoid short movement loops.
  std::deque<WorldPosition> recent_positions;

  /// Set mirror of recent_positions for O(1) lookup in in_recent.
  std::unordered_set<WorldPosition> recent_positions_set;

  /// Tracks how many times each position has been visited
  /// Higher counts will discourage revisiting the same cell
  std::unordered_map<WorldPosition, std::size_t> visit_counts;

  /// Number of recent cells retained for loop-avoidance.
  static constexpr std::size_t history_size = 6;

  /// Small random value added to movement scores to prevent ties and encourage
  /// exploration.
  static constexpr double tie_breaker = 0.001;

  /**
   * @brief Record a visit to a cell by incrementing its count and adding it to
   * recent_positions.
   * This method should be called at the end of each turn to update the agent's
   * knowledge of its movement history. It updates visit_counts to reflect the
   * new visit and adds the position to recent_positions, which is used to avoid
   * short-term loops. recent_positions is maintained as a fixed-size queue that
   * only keeps the most recent 6 positions, so it effectively tracks the last 6
   * cells the agent has been in. This allows the agent to prefer moves that
   * lead to less recently visited cells, encouraging exploration and reducing
   * the chance of getting stuck in loops.
   * @param pos The position that the agent has just moved to and should be
   * recorded as visited.
   */
  void record_visit(WorldPosition const& pos) {
    visit_counts[pos]++;

    recent_positions.push_back(pos);
    recent_positions_set.insert(pos);
    while (recent_positions.size() > history_size) {
      WorldPosition const evicted = recent_positions.front();
      recent_positions.pop_front();
      if (std::count(recent_positions.begin(), recent_positions.end(),
                     evicted) == 0) {
        recent_positions_set.erase(evicted);
      }
    }
  }

  /**
   * @brief Get the visit count for a cell from the visit_counts map.
   * @param pos The position of the cell to query.
   * @return The number of times the agent has visited the cell at the given
   * position. If the cell has not been visited before, this returns 0. This
   * method is used in the movement scoring function to penalize cells that have
   * been visited frequently, encouraging the agent to explore less-visited
   * areas of the world and avoid congestion.
   */
  [[nodiscard]] std::size_t get_visit_count(WorldPosition const& pos) const {
    auto it = visit_counts.find(pos);
    return (it == visit_counts.end()) ? 0 : it->second;
  }

  /**
   * @brief Score a neighboring cell for movement desirability based on distance
   * to target, visit history, and recent movement outcomes.
   * @param candidate The neighboring cell to score.
   * @param target The target position.
   * @return The score for the candidate cell.
   * Lower scores indicate more desirable movement options.
   */
  [[nodiscard]] double neighbor_score(WorldPosition const& candidate,
                                      WorldPosition const& target) const {
    auto manhattan = [&](WorldPosition const& p) {
      return std::abs(target.X() - p.X()) + std::abs(target.Y() - p.Y());
    };

    // Base goal seeking
    double score = static_cast<double>(manhattan(candidate));

    // Penalize revisiting overused cells
    score += visit_penalty * static_cast<double>(get_visit_count(candidate));

    return score;
  }

  /**
   * @brief Get the four cardinal neighboring positions of a given position.
   * @param pos The position to get neighbors for.
   * @return An array of the four neighboring positions in the order: up, down,
   * left, right.
   */
  static std::array<WorldPosition, 4> get_neighbors(WorldPosition const& pos) {
    return {pos.Up(), pos.Down(), pos.Left(), pos.Right()};
  }

  /**
   * @brief Choose neighboring positions to move to, preferring those that are
   * closer to the target and less recently visited.
   * @param pos The current position of the agent.
   * @param target The target destination the agent is trying to reach.
   * @return An array of the four neighboring positions sorted by desirability,
   * with the most desirable option first. The agent will attempt to move to the
   * first position, and if that move is blocked or unavailable, it can fall
   * back to the next options in order.
   */
  std::array<WorldPosition, 4> choose_move_options(
      WorldPosition const& pos, WorldPosition const& target) {
    auto neighbors = get_neighbors(pos);

    struct Candidate {
      WorldPosition pos;
      double score;
      double tie_breaker;
    };

    std::vector<Candidate> scored;
    scored.reserve(4);

    std::uniform_real_distribution<double> jitter(0.0, tie_breaker);

    for (auto const& n : neighbors) {
      scored.push_back(Candidate{n, neighbor_score(n, target), jitter(rng)});
    }

    std::sort(scored.begin(), scored.end(),
              [](Candidate const& a, Candidate const& b) {
                return a.score + a.tie_breaker < b.score + b.tie_breaker;
              });

    return {scored[0].pos, scored[1].pos, scored[2].pos, scored[3].pos};
  }

  /**
   * @brief Check whether a position is in the recent movement history.
   * @param p Position to search for.
   * @return true if p appears in recent_positions.
   */
  [[nodiscard]] bool in_recent(WorldPosition const& p) const {
    return recent_positions_set.count(p) > 0;
  }

  /**
   * @brief Choose a random cardinal neighbor, preferring non-recent cells.
   * @param pos Current position.
   * @return Neighboring position selected from up, down, left, or right.
   */
  WorldPosition get_random_neighbor(WorldPosition const& pos) {
    std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(),
                                           pos.Right()};

    std::vector<WorldPosition> candidates{};

    // check if all possible neighbors are in
    // recent positions
    for (auto const& n : neighbors) {
      if (!in_recent(n)) {
        candidates.push_back(n);
      }
    }

    // if all in recent then any option works
    if (candidates.empty()) {
      candidates.assign(neighbors.begin(), neighbors.end());
    }

    // pick random movement of avaliable
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng)];
  }

 public:
  /**
   * @brief Construct a swarming agent with initial state and stable ID.
   * @param data Initial state object.
   * @param id Stable ID assigned by the world.
   */
  SwarmingAgent(SwarmData data, size_t id, LogLevel loglevel = LogLevel::Normal,
                uint64_t tick = 0)
      : StepAgentBase<SwarmData>(data, id, loglevel, tick) {}

  /**
   * @brief Dispatch to the movement policy for the active data type.
   * @return StepContainer describing the agent's requested turn.
   */
  [[nodiscard]] StepContainer GetTurn() override {
    if constexpr (std::is_same_v<SwarmData, TrafficData>) {
      return TrafficGetTurn();
    } else if constexpr (std::is_same_v<SwarmData, DiseaseData>) {
      return InfectionGetTurn();
    }
  }

  /**
   * @brief Build a traffic-world movement turn.
   * The agent attempts to move toward its destination while avoiding recently
   * visited cells and recently failed movement attempts. If no destination is
   * set, it moves randomly with a bias toward unvisited neighbors.
   * @return StepContainer with one or two MovementSteps, depending on whether
   * the backup move is needed. The first step is always the primary move. The
   * second step is a backup move that is only executed if the primary move is
   * blocked or unavailable.
   */
  [[nodiscard]] StepContainer TrafficGetTurn()
    requires(std::is_same_v<SwarmData, TrafficData>)
  {
    StepContainer container{};

    if (!this->mData.is_active) {
      return container;
    }

    WorldPosition const& pos = this->mData.position;

    // First, learn from what happened last turn
    record_visit(pos);

    if (!this->mData.destination.has_value()) {
      WorldPosition random_pos = get_random_neighbor(pos);

      container.add_step(cse498::steps::MovementStep{random_pos});
      return container;
    }

    WorldPosition const& dest = this->mData.destination.value();

    if (pos == dest) {
      return container;
    }

    auto options = choose_move_options(pos, dest);

    container.add_step(cse498::steps::MovementStep{options[0]});
    container.add_step(cse498::steps::MovementStep{options[1]});
    container.add_step(cse498::steps::MovementStep{options[2]});
    container.add_step(cse498::steps::MovementStep{options[3]});

    return container;
  }

  /**
   * @brief Build an infectious-world movement turn.
   *
   * Infection state changes are handled by InfectiousWorld. The agent only
   * chooses a neighboring movement target for this tick.
   *
   * @return StepContainer containing one random MovementStep.
   */
  [[nodiscard]] StepContainer InfectionGetTurn()
    requires(std::is_same_v<SwarmData, DiseaseData>)
  {
    // Behavior in each state:
    // SUSCEPTIBLE:
    // Move randomly
    // Transition to INFECTED when near infected agents (handled by world logic)
    // INFECTED:
    // Move randomly
    // Infect susceptible agents on contact/proximity (handled by world logic)
    // RECOVERED:
    // Move randomly
    // No infection interaction behavior

    StepContainer container{};
    WorldPosition random_pos = get_random_neighbor(this->mData.position);
    cse498::steps::MovementStep random_move{random_pos};
    container.add_step(std::move(random_move));
    return container;
  }

  /**
   * @brief Set the destination field when the data type supports it.
   * @param goal Target world position.
   */
  void SetGoal(WorldPosition goal) override {
    if constexpr (requires { this->mData.destination; }) {
      auto state = this->GetState();
      state.destination = goal;
      this->SetStateNoLog(state);
    }
  }

  /**
   * @brief Set traffic swarming direction when supported by TrafficData.
   * @param swarm_away true to flee from the goal, false to approach it.
   */
  void SetSwarmAway(bool swarm_away)
    requires(std::is_same_v<SwarmData, TrafficData>)
  {
    auto state = this->GetState();
    state.swarm_away = swarm_away;
    this->SetStateNoLog(state);
  }

  /**
   * @brief Enable or disable a traffic agent.
   * @param active true if the agent should emit movement steps.
   */
  void SetActive(bool active)
    requires(std::is_same_v<SwarmData, TrafficData>)
  {
    auto state = this->GetState();
    state.is_active = active;
    this->SetStateNoLog(state);
  }
};

// clang-format on
}  // End of namespace cse498

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <deque>
#include <optional>
#include <random>

#include "../core/AgentData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"
#include "../core/StepWorldBase.hpp"
#include "../tools/StateGridPosition.hpp"

namespace cse498 {

/**
 * @brief Concept for state types supported by SwarmingAgent.
 *
 * SwarmingAgent currently supports traffic and infectious-disease data because
 * those state objects provide the fields its movement logic reads and updates.
 */
template <typename T>
concept IsSwarmData = Concepts::IsOneOf<T, TrafficData, DiseaseData>;

/**
 * @brief Step-based agent that greedily moves toward goals or wanders locally.
 *
 * For TrafficData, the agent approaches its optional destination while avoiding
 * immediate backtracking through a short position history. For DiseaseData, it
 * emits random neighboring movement steps and leaves infection state
 * transitions to InfectiousWorld.
 *
 * @tparam SwarmData Either TrafficData or DiseaseData.
 */
template <IsSwarmData SwarmData>
class SwarmingAgent : public StepAgentBase<SwarmData> {
 private:
  /// Random generator used to choose among candidate neighboring cells.
  std::mt19937 rng{std::random_device{}()};

  /// Number of recent cells retained for loop-avoidance.
  static constexpr std::size_t kHistorySize = 6;

  /// Recent positions used to avoid short movement loops.
  std::deque<WorldPosition> recent_positions;

  /**
   * @brief Add a position to the rolling movement history.
   * @param pos Position to record.
   */
  void record_position(WorldPosition const& pos) {
    // skip duplicates so a stationary agent doesn't flush the history with
    // copies of the same cell
    if (!recent_positions.empty() && recent_positions.back() == pos) return;

    recent_positions.push_back(pos);

    while (recent_positions.size() > kHistorySize) {
      recent_positions.pop_front();
    }
  }

  /**
   * @brief Check whether a position is in the recent movement history.
   * @param p Position to search for.
   * @return true if p appears in recent_positions.
   */
  [[nodiscard]] bool in_recent(WorldPosition const& p) const {
    return std::find(recent_positions.begin(), recent_positions.end(), p) !=
           recent_positions.end();
  }

  /**
   * @brief Choose a random cardinal neighbor, preferring non-recent cells.
   * @param pos Current position.
   * @return Neighboring position selected from up, down, left, or right.
   */
  WorldPosition get_random_neighbor(WorldPosition& pos) {
    std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(),
                                           pos.Right()};

    std::vector<WorldPosition> candidates{};

    for (auto const& n : neighbors) {  // check if all possible neighbors are in
                                       // recent positions
      if (!in_recent(n)) {
        candidates.push_back(n);
      }
    }

    if (candidates.empty()) {  // if all in recent then any option works
      candidates.assign(neighbors.begin(), neighbors.end());
    }

    std::uniform_int_distribution<size_t> dist(
        0, candidates.size() - 1);  // pick random movement of avaliable
    return candidates[dist(rng)];
  }

  /**
   * @brief Pick the cardinal neighbor closest to a target.
   *
   * The search prefers cells that are not in recent_positions so the agent can
   * route around walls and dead ends instead of repeatedly choosing the same
   * blocked cell. If exclude is set, that cell is skipped so callers can choose
   * a distinct backup after picking a primary movement.
   *
   * @param pos Current position.
   * @param target Desired destination.
   * @param exclude Optional neighbor to ignore.
   * @return Best neighboring cell by Manhattan distance.
   */
  // Picks the neighbor with the smallest Manhattan distance to `target`,
  // preferring cells not in `recent_positions` so the agent naturally
  // routes around walls and dead ends rather than pounding the same cell.
  // If `exclude` is set, that cell is skipped (used to find a distinct
  // "second choice" after already picking a primary). If every neighbor is
  // in recent history, we fall back to the closest cell overall so the
  // agent keeps swarming instead of freezing.
  WorldPosition best_neighbor(WorldPosition const& pos,
                              WorldPosition const& target,
                              std::optional<WorldPosition> exclude) {
    std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(),
                                           pos.Right()};

    auto manhattan = [&](WorldPosition const& p) {
      return std::abs(target.X() - p.X()) + std::abs(target.Y() - p.Y());
    };

    WorldPosition const* best_fresh = nullptr;
    double best_fresh_d = 0.0;
    WorldPosition const* best_any = nullptr;
    double best_any_d = 0.0;

    for (auto const& n : neighbors) {
      if (exclude.has_value() && n == *exclude) continue;
      double d = manhattan(n);
      if (best_any == nullptr || d < best_any_d) {
        best_any = &n;
        best_any_d = d;
      }
      if (!in_recent(n) && (best_fresh == nullptr || d < best_fresh_d)) {
        best_fresh = &n;
        best_fresh_d = d;
      }
    }

    if (best_fresh != nullptr) return *best_fresh;
    if (best_any != nullptr) return *best_any;
    return pos;  // only reached if `exclude` somehow matches all neighbors
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
   * @brief Build a traffic movement turn.
   *
   * Inactive agents emit no steps. Agents without a destination wander
   * randomly. Agents with a destination request an availability check for a
   * primary movement and provide a backup movement if the primary is blocked.
   *
   * @return StepContainer for the traffic world.
   */
  [[nodiscard]] StepContainer TrafficGetTurn()
    requires(std::is_same_v<SwarmData, TrafficData>)
  {
    StepContainer container{};

    if (!this->mData.is_active) {
      return container;  // empty container means no steps remain still
    }

    // Remember where we are this turn so future detours can avoid looping
    // back onto cells we've just visited.
    record_position(this->mData.position);

    // make a random turn and skip the world query entirely
    if (!this->mData.destination.has_value()) {
      WorldPosition random_pos = get_random_neighbor(this->mData.position);
      cse498::steps::MovementStep random_move{random_pos};
      container.add_step(std::move(random_move));
      return container;
    }

    // wander randomly instead of approaching (maybe not have a swarm away) (I
    // REMOVED SWARM AWAY)

    // if we are already at the destination stay there
    WorldPosition const& pos = this->mData.position;
    WorldPosition const& dest = this->mData.destination.value();
    if (pos == dest) {
      return container;  // empty to stay in place
    }

    // Pick the best neighbor toward the destination preferring cells we
    // haven't been to recently. `backup` is the next best choice, used if
    // the world tells us `primary` is a wall
    WorldPosition primary = best_neighbor(pos, dest, std::nullopt);
    WorldPosition backup = best_neighbor(pos, dest, primary);

    // Eagerly mark both as attempted in our history iff both turn out to
    // be walls and the agent stays put and next turn we'll try different
    // cells instead of repicking these two forever
    record_position(primary);
    record_position(backup);

    cse498::steps::InfoStep query{cse498::steps::InfoStep::Aspect::LOC_AVAIL,
                                  primary};

    cse498::steps::ConditionalStep is_open{cse498::steps::InfoHandler(
        [](bool is_available) -> std::expected<bool, cse498::steps::StepErr> {
          return is_available;
        })};

    container.add_step(std::move(query), std::move(is_open),
                       cse498::steps::MovementStep{primary},
                       cse498::steps::MovementStep{backup});
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

}  // namespace cse498

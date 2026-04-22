#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>
#include <random>
#include <unordered_map>

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

  /// Recent positions used to avoid short movement loops.
  std::deque<WorldPosition> recent_positions;

  /// Represents a grid position used as a key in hash maps
  /// Stores X and Y coordinates extracted from WorldPosition
  struct CellKey {
    double x{};
    double y{};

    /// Equality comparison required for unordered_map keys
    bool operator==(CellKey const &other) const {
      return x == other.x && y == other.y;
    }
  };

  /// Hash function for CellKey so it can be used in unordered_map
  /// Combines hashes of x and y coordinates
  struct CellKeyHash {
    std::size_t operator()(CellKey const &k) const {
      std::size_t h1 = std::hash<double>{}(k.x);
      std::size_t h2 = std::hash<double>{}(k.y);
      return h1 ^ (h2 << 1); // combine hashes
    }
  };

  /// Represents a directed edge between two positions (from → to)
  /// Used to track movement attempts between cells
  struct EdgeKey {
    CellKey from{};
    CellKey to{};

    /// Equality comparison for edge keys
    bool operator==(EdgeKey const &other) const {
      return from == other.from && to == other.to;
    }
  };

  /// Hash function for EdgeKey so it can be used in unordered_map
  /// Combines hashes of the "from" and "to" positions
  struct EdgeKeyHash {
    std::size_t operator()(EdgeKey const &e) const {
      std::size_t h1 = CellKeyHash{}(e.from);
      std::size_t h2 = CellKeyHash{}(e.to);
      return h1 ^ (h2 << 1);
    }
  };

  /// Converts a WorldPosition into a CellKey for use in maps
  /// Extracts X and Y coordinates
  static CellKey to_key(WorldPosition const &p) {
    return CellKey{p.X(), p.Y()};
  }

  /// Converts a movement (from → to) into an EdgeKey
  /// Used for tracking penalties on attempted paths
  static EdgeKey to_edge(WorldPosition const &from, WorldPosition const &to) {
    return EdgeKey{to_key(from), to_key(to)};
  }

  /// Tracks how many times each position has been visited
  /// Higher counts will discourage revisiting the same cell
  std::unordered_map<CellKey, std::size_t, CellKeyHash> visit_counts;

  /// Tracks penalties for moving along specific edges (from → to)
  /// Higher penalties indicate that movement attempts failed recently
  std::unordered_map<EdgeKey, double, EdgeKeyHash> edge_penalties;

  /// Stores the origin position from the previous turn
  /// Used to determine if movement succeeded or failed
  std::optional<WorldPosition> pending_origin;

  /// Stores the primary movement attempted last turn
  /// Used to detect if primary succeeded or needs penalizing
  std::optional<WorldPosition> pending_primary;

  /// Stores the backup movement attempted last turn
  /// Used to detect fallback behavior and adjust penalties
  std::optional<WorldPosition> pending_backup;

  /**
   * @brief Record a visit to a cell in the visit_counts map.
   * This is called whenever the agent successfully occupies a cell at the end
   * of a turn. It increments the visit count for that cell, which is used to
   * discourage the agent from repeatedly visiting the same cell in a short
   * period of time. The visit count is stored in an unordered_map keyed by the
   * cell's coordinates, and the value is the number of times the agent has
   * visited that cell. This helps the agent learn to avoid congested areas and
   * break out of loops by making frequently visited cells less attractive for
   * movement.
   */
  void record_visit(WorldPosition const &pos) { visit_counts[to_key(pos)]++; }

  /**
   * @brief Get the visit count for a cell from the visit_counts map.
   * @param pos The position of the cell to query.
   * @return The number of times the agent has visited the cell at the given
   * position. If the cell has not been visited before, this returns 0. This
   * method is used in the movement scoring function to penalize cells that have
   * been visited frequently, encouraging the agent to explore less-visited
   * areas of the world and avoid congestion.
   */
  [[nodiscard]] std::size_t get_visit_count(WorldPosition const &pos) const {
    auto it = visit_counts.find(to_key(pos));
    return (it == visit_counts.end()) ? 0 : it->second;
  }

  /**
   * @brief Add a penalty to an edge in the edge_penalties map.
   * @param from The starting position of the edge.
   * @param to The ending position of the edge.
   * @param amount The penalty amount to add.
   * This function increments the penalty for the given edge by the specified
   * amount. The edge is represented as a pair of cell positions, and the
   * penalty is stored in an unordered_map keyed by the edge's coordinates.
   */
  void add_edge_penalty(WorldPosition const &from, WorldPosition const &to,
                        double amount) {
    edge_penalties[to_edge(from, to)] += amount;
  }

  /**
   * @brief Get the penalty for an edge from the edge_penalties map.
   * @param from The starting position of the edge.
   * @param to The ending position of the edge.
   * @return The penalty value for the given edge. If the edge has no recorded
   * penalty, this returns 0. This method is used in the movement scoring
   * function to penalize recently failed movement attempts along specific
   * edges, making those moves less attractive for a short period of time.
   */
  [[nodiscard]] double get_edge_penalty(WorldPosition const &from,
                                        WorldPosition const &to) const {
    auto it = edge_penalties.find(to_edge(from, to));
    return (it == edge_penalties.end()) ? 0.0 : it->second;
  }

  /**
   * @brief Decay the penalties for all edges in the edge_penalties map.
   * @param factor The decay factor (default is 0.85).
   * This function reduces the penalty for all edges by the specified factor.
   * Edges with penalties below a certain threshold are removed from the map.
   */
  void decay_edge_penalties(double factor = 0.85) {
    for (auto it = edge_penalties.begin(); it != edge_penalties.end();) {
      it->second *= factor;
      if (it->second < 0.25) {
        it = edge_penalties.erase(it);
      } else {
        ++it;
      }
    }
  }

  /**
   * @brief Update internal state based on the outcome of the previous turn's
   * movement attempt. This should be called at the start of each turn before
   * choosing new movement steps. The agent learns from the success or failure
   * of its previous primary and backup movement targets, updating visit counts
   * and edge penalties to discourage recently failed moves and short loops.
   * @param current_pos The agent's current position, which reflects the outcome
   * of the previous turn's movement attempt. The function compares current_pos
   * to the pending primary and backup targets set in the last turn. If the
   * primary target succeeded, it simply records the visit. If the backup
   * succeeded, it records the visit and adds a penalty to the primary target.
   * If neither succeeded and the agent stayed in place, it adds penalties to
   * both targets. In all cases, it records the visit to the current position
   * and then clears the pending move state for the next turn.
   */
  void update_from_previous_turn(WorldPosition const &current_pos) {
    if (!pending_origin.has_value()) {
      record_visit(current_pos);
      return;
    }

    WorldPosition const &origin = *pending_origin;

    if (pending_primary.has_value() && current_pos == *pending_primary) {
      // Primary succeeded
      record_visit(current_pos);
    } else if (pending_backup.has_value() && current_pos == *pending_backup) {
      // Backup succeeded
      record_visit(current_pos);

      // Primary was likely blocked or unavailable this turn
      if (pending_primary.has_value()) {
        add_edge_penalty(origin, *pending_primary, 3.0);
      }
    } else if (current_pos == origin) {
      // Agent did not move; likely blocked or congested
      if (pending_primary.has_value()) {
        add_edge_penalty(origin, *pending_primary, 4.0);
      }
      if (pending_backup.has_value()) {
        add_edge_penalty(origin, *pending_backup, 2.0);
      }

      // Also count staying here; this helps break loops around dead ends
      record_visit(current_pos);
    } else {
      // Unexpected move outcome, but still record the visit
      record_visit(current_pos);
    }

    pending_origin.reset();
    pending_primary.reset();
    pending_backup.reset();
  }

  /**
   * @brief Score a neighboring cell for movement desirability based on distance
   * to target, visit history, and recent movement outcomes.
   * @param current The agent's current position.
   * @param candidate The neighboring cell to score.
   * @param target The target position.
   * @return The score for the candidate cell.
   * Lower scores indicate more desirable movement options.
   */
  [[nodiscard]] double neighbor_score(WorldPosition const &current,
                                      WorldPosition const &candidate,
                                      WorldPosition const &target) const {
    auto manhattan = [&](WorldPosition const &p) {
      return std::abs(target.X() - p.X()) + std::abs(target.Y() - p.Y());
    };

    double score = 0.0;

    // Base goal seeking
    score += static_cast<double>(manhattan(candidate));

    // Penalize revisiting overused cells
    score += 1.75 * static_cast<double>(get_visit_count(candidate));

    // Penalize recently failed attempts from this exact origin
    score += 2.5 * get_edge_penalty(current, candidate);

    // Strongly discourage short loops, but do not forbid them
    if (in_recent(candidate)) {
      score += 6.0;
    }

    return score;
  }

  std::array<WorldPosition, 4> get_neighbors(WorldPosition const &pos) const {
    return {pos.Up(), pos.Down(), pos.Left(), pos.Right()};
  }

  std::pair<WorldPosition, WorldPosition>
  choose_primary_backup(WorldPosition const &pos, WorldPosition const &target) {
    auto neighbors = get_neighbors(pos);

    struct Candidate {
      WorldPosition pos;
      double score;
      double tie_breaker;
    };

    std::vector<Candidate> scored;
    scored.reserve(4);

    std::uniform_real_distribution<double> jitter(0.0, 0.001);

    for (auto const &n : neighbors) {
      scored.push_back(
          Candidate{n, neighbor_score(pos, n, target), jitter(rng)});
    }

    std::sort(scored.begin(), scored.end(),
              [](Candidate const &a, Candidate const &b) {
                double as = a.score + a.tie_breaker;
                double bs = b.score + b.tie_breaker;
                return as < bs;
              });

    return {scored[0].pos, scored[1].pos};
  }

  /**
   * @brief Check whether a position is in the recent movement history.
   * @param p Position to search for.
   * @return true if p appears in recent_positions.
   */
  [[nodiscard]] bool in_recent(WorldPosition const &p) const {
    return std::find(recent_positions.begin(), recent_positions.end(), p) !=
           recent_positions.end();
  }

  /**
   * @brief Choose a random cardinal neighbor, preferring non-recent cells.
   * @param pos Current position.
   * @return Neighboring position selected from up, down, left, or right.
   */
  WorldPosition get_random_neighbor(WorldPosition const &pos) {
    std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(),
                                           pos.Right()};

    std::vector<WorldPosition> candidates{};

    for (auto const &n : neighbors) { // check if all possible neighbors are in
                                      // recent positions
      if (!in_recent(n)) {
        candidates.push_back(n);
      }
    }

    if (candidates.empty()) { // if all in recent then any option works
      candidates.assign(neighbors.begin(), neighbors.end());
    }

    std::uniform_int_distribution<size_t> dist(
        0, candidates.size() - 1); // pick random movement of avaliable
    return candidates[dist(rng)];
  }

public:
  /**
   * @brief Construct a swarming agent with initial state and stable ID.
   * @param data Initial state object.
   * @param id Stable ID assigned by the world.
   */
  SwarmingAgent(SwarmData data, size_t id)
      : StepAgentBase<SwarmData>(data, id) {}

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

    WorldPosition const &pos = this->mData.position;

    // First, learn from what happened last turn
    update_from_previous_turn(pos);

    // Slowly forget old congestion / temporary blockages
    decay_edge_penalties();

    if (!this->mData.destination.has_value()) {
      WorldPosition random_pos = get_random_neighbor(pos);

      pending_origin = pos;
      pending_primary = random_pos;
      pending_backup.reset();

      container.add_step(cse498::steps::MovementStep{random_pos});
      return container;
    }

    WorldPosition const &dest = this->mData.destination.value();

    if (pos == dest) {
      return container;
    }

    auto [primary, backup] = choose_primary_backup(pos, dest);

    pending_origin = pos;
    pending_primary = primary;
    pending_backup = backup;

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
      this->SetState(state);
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
    this->SetState(state);
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
    this->SetState(state);
  }
};

} // namespace cse498

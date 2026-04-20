/**
 * StepPacingAgent.hpp
 * @brief A pacing agent that uses the step-based world system.
 *
 * Walks back and forth along a row or column, emitting a single MovementStep
 * each turn. Direction reversal is detected by comparing the current position
 * against the last submitted target — if they differ the move was blocked.
 **/

#pragma once

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../core/AgentData.hpp"
#include "../core/Step.hpp"
#include "../core/StepAgentBase.hpp"

namespace cse498 {

class StepPacingAgent : public StepAgentBase<DiseaseData> {
 protected:
  bool vertical = true;  ///< true = moves down/up; false = right/left
  bool reverse = false;  ///< true = moving back (up or left)

  /// Position we requested last turn; used to detect a blocked move.
  std::optional<WorldPosition> last_target{};

  // -------------------------------------------------------------------------
  // BFS support
  // -------------------------------------------------------------------------

  /// Returns true when the given cell can be stepped onto.
  std::function<bool(WorldPosition)> m_passable;

  /// Cached BFS result and cursor.
  std::vector<WorldPosition> m_bfs_path;
  size_t m_bfs_idx{0};

  /// Destination the current path was computed for.
  WorldPosition m_bfs_target{};

  /// The cell we last asked to move to (for stuck detection).
  WorldPosition m_bfs_last_step{};

  /// True while we are navigating via BFS (cleared when pacing resumes).
  bool m_bfs_active{false};

  /// Compact a WorldPosition into a single integer for use as a map key.
  [[nodiscard]] static size_t encode(WorldPosition p) noexcept {
    return (static_cast<size_t>(p.CellX()) << 16) |
           static_cast<size_t>(p.CellY());
  }

  /// BFS from @p start to @p goal using m_passable as the walkability test.
  [[nodiscard]] std::vector<WorldPosition> ComputeBFS(
      WorldPosition start, WorldPosition goal) const {
    if (start == goal || !m_passable) return {};

    std::queue<WorldPosition> frontier;
    std::unordered_map<size_t, WorldPosition> came_from;

    frontier.push(start);
    came_from[encode(start)] = start;

    while (!frontier.empty()) {
      WorldPosition cur = frontier.front();
      frontier.pop();

      if (cur == goal) {
        // Reconstruct the path from goal back to start.
        std::vector<WorldPosition> path;
        WorldPosition pos = goal;
        while (!(pos == start)) {
          path.push_back(pos);
          pos = came_from.at(encode(pos));
        }
        std::reverse(path.begin(), path.end());
        return path;
      }

      for (auto next : {cur.Up(), cur.Down(), cur.Left(), cur.Right()}) {
        size_t k = encode(next);
        if (!came_from.count(k) && m_passable(next)) {
          came_from[k] = cur;
          frontier.push(next);
        }
      }
    }
    return {};  // No path found
  }

 public:
  StepPacingAgent(DiseaseData data, size_t id)
      : StepAgentBase<DiseaseData>(data, id) {}
  ~StepPacingAgent() override = default;

  StepPacingAgent& SetHorizontal() {
    vertical = false;
    return *this;
  }
  StepPacingAgent& SetVertical() {
    vertical = true;
    return *this;
  }

  /// Supply the walkability predicate used by the BFS.  Must be called once
  /// after the agent is added to the world (before the simulation starts).
  StepPacingAgent& SetPassable(std::function<bool(WorldPosition)> fn) {
    m_passable = std::move(fn);
    return *this;
  }

  /// SetGoal is unused for a pacing agent but required by the interface.
  void SetGoal(WorldPosition /*position*/) override {}

  [[nodiscard]] steps::StepContainer GetTurn() override {
    using namespace cse498::steps;
    WorldPosition pos = mData.position;

    // -------------------------------------------------------------------------
    // INFECTED or RECOVERING with a destination → BFS navigation
    // -------------------------------------------------------------------------
    if (mData.destination.has_value() &&
        (mData.health == HealthState::INFECTED ||
         mData.health == HealthState::RECOVERED)) {
      WorldPosition dest = mData.destination.value();

      // Conditions that require a fresh BFS computation:
      //   1. First time in BFS mode or destination changed.
      //   2. Path has been fully consumed.
      //   3. Our last submitted step was blocked (agent didn't advance).
      bool dest_changed = !m_bfs_active || !(m_bfs_target == dest);
      bool path_exhausted = (m_bfs_idx >= m_bfs_path.size());
      bool step_blocked =
          m_bfs_active && (m_bfs_idx > 0) && !(pos == m_bfs_last_step);

      if (dest_changed || path_exhausted || step_blocked) {
        m_bfs_path = ComputeBFS(pos, dest);
        m_bfs_idx = 0;
        m_bfs_target = dest;
        m_bfs_active = true;
      }

      if (m_bfs_idx < m_bfs_path.size()) {
        WorldPosition next = m_bfs_path[m_bfs_idx++];
        m_bfs_last_step = next;
        StepContainer steps;
        steps.add_step(MovementStep{next});
        return steps;
      }

      // Arrived at destination (or no path exists) — stay put.
      return StepContainer{};
    }

    m_bfs_active = false;  // reset so BFS re-initialises next infection

    // If our last move was blocked (position unchanged), flip direction.
    if (last_target.has_value() && pos != last_target.value()) {
      reverse = !reverse;
    }

    WorldPosition target = vertical ? (reverse ? pos.Up() : pos.Down())
                                    : (reverse ? pos.Left() : pos.Right());

    last_target = target;

    StepContainer steps;
    steps.add_step(MovementStep{target});
    return steps;
  }
};

}  // namespace cse498

/**
 * SimWorldBase.hpp
 * @brief Common base for simulation worlds (infectious, traffic)
 * @note Status: PROPOSAL
 **/

#pragma once

#include <algorithm>  // for std::max and std::min
#include <cassert>
#include <vector>

#include "../core/WorldBase.hpp"

namespace cse498 {

class SimWorldBase : public WorldBase {
 protected:
  enum ActionType {
    REMAIN_STILL = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
  };

  size_t floor_id;  ///< Easy access to floor CellType ID.
  size_t wall_id;   ///< Easy access to wall CellType ID.

  size_t tick_count = 0;  // Use time from Data Analytics group

  /// Provide the agent with movement actions.
  void ConfigAgent(AgentBase& agent) override {
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
  }

  /// Try to move an agent in the given direction. Returns true on success.
  bool MoveAgent(AgentBase& agent, size_t action_id) {
    WorldPosition cur_pos = agent.GetLocation().AsWorldPosition();
    WorldPosition new_pos;
    switch (action_id) {
      case REMAIN_STILL:
        new_pos = cur_pos;
        break;
      case MOVE_UP:
        new_pos = cur_pos.Up();
        break;
      case MOVE_DOWN:
        new_pos = cur_pos.Down();
        break;
      case MOVE_LEFT:
        new_pos = cur_pos.Left();
        break;
      case MOVE_RIGHT:
        new_pos = cur_pos.Right();
        break;
      default:
        return false;
    }

    if (!main_grid.IsValid(new_pos)) return false;
    if (main_grid[new_pos] == wall_id) return false;

    agent.SetLocation(new_pos);
    return true;
  }

 public:
  SimWorldBase() {
    floor_id = main_grid.AddCellType("floor", "Open walkable cell.", '.');
    wall_id = main_grid.AddCellType("wall", "Impassable wall.", '#');
  }
  ~SimWorldBase() override = default;

  [[nodiscard]] size_t GetTickCount() const { return tick_count; }
  [[nodiscard]] size_t GetFloorID() const { return floor_id; }
  [[nodiscard]] size_t GetWallID() const {
    return wall_id;
  }  // Maybe use the time class from the data Analytics group?

  /// Return IDs of all agents at a given grid position.
  [[nodiscard]] std::vector<size_t> GetAgentsAt(WorldPosition pos) const {
    std::vector<size_t> result;
    for (const auto& ptr : agent_set) {
      if (!ptr->GetLocation().IsPosition()) continue;
      WorldPosition ap = ptr->GetLocation().AsWorldPosition();
      if (ap.CellX() == pos.CellX() && ap.CellY() == pos.CellY()) {
        result.push_back(ptr->GetID());
      }
    }
    return result;
  }

  /// Return IDs of all agents within manhattan distance of a position.
  [[nodiscard]] std::vector<size_t> GetAgentsNear(WorldPosition center,
                                                  size_t radius) const {
    std::vector<size_t> result;
    for (const auto& ptr : agent_set) {
      if (!ptr->GetLocation().IsPosition()) continue;
      WorldPosition ap = ptr->GetLocation().AsWorldPosition();
      size_t dx = std::max(ap.CellX(), center.CellX()) -
                  std::min(ap.CellX(), center.CellX());
      size_t dy = std::max(ap.CellY(), center.CellY()) -
                  std::min(ap.CellY(), center.CellY());
      if (dx + dy <= radius) {
        result.push_back(ptr->GetID());
      }
    }
    return result;
  }
};

}  // End of namespace cse498

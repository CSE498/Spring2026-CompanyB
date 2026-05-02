/**
 * SimWorldBase.hpp
 * @brief Common base for grid-based simulation worlds (infectious, traffic).
 * @note Extends StepWorldBase<DataClass>. DataClass must have a
 *       `WorldPosition position` member (e.g., DiseaseData).
 **/

#pragma once

#include <algorithm>  // for std::max and std::min
#include <cassert>
#include <vector>

#include "../core/StepWorldBase.hpp"

namespace cse498 {

template <Concepts::IsDataClass DataClass>
class SimWorldBase : public StepWorldBase<DataClass> {
  using Base = StepWorldBase<DataClass>;

 protected:
  using AgentPtr = std::shared_ptr<StepAgentBase<DataClass>>;
  using Base::agent_set;
  using Base::main_grid;

  size_t floor_id{};  ///< Easy access to floor CellType ID.
  size_t wall_id{};   ///< Easy access to wall CellType ID.
  size_t tick_count{0};

 public:
  SimWorldBase() {
    floor_id = main_grid.AddCellType("floor", "Open walkable cell.", '.');
    wall_id = main_grid.AddCellType("wall", "Impassable wall.", '#');
  }
  ~SimWorldBase() override = default;

  // -- Accessors --

  [[nodiscard]] size_t GetTickCount() const { return tick_count; }
  [[nodiscard]] const auto& GetAgents() const { return agent_set; }
  [[nodiscard]] size_t GetFloorID() const { return floor_id; }
  [[nodiscard]] size_t GetWallID() const { return wall_id; }
  [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }

  [[nodiscard]] WorldGrid& GetGrid() { return main_grid; }
  [[nodiscard]] const WorldGrid& GetGrid() const { return main_grid; }

  /// Get the current DataClass state of agent at index \p id.
  [[nodiscard]] DataClass GetAgentState(size_t id) const {
    assert(id < agent_set.size() && "GetAgentState: Agent id out of range");
    return agent_set[id]->GetState();
  }

  /// Const view of all agents
  [[nodiscard]] const std::vector<std::shared_ptr<StepAgentBase<DataClass>>>&
  GetAgents() const {
    return agent_set;
  }

  // -- Spatial queries (require DataClass to have a WorldPosition `position`)

  /// Return indices of all agents located in cell \p pos.
  [[nodiscard]] std::vector<size_t> GetAgentsAt(WorldPosition pos) const {
    std::vector<size_t> result;
    for (size_t i = 0; i < agent_set.size(); ++i) {
      WorldPosition ap = agent_set[i]->GetState().position;
      if (ap.CellX() == pos.CellX() && ap.CellY() == pos.CellY())
        result.push_back(i);
    }
    return result;
  }

  /// Return indices of all agents within Manhattan \p radius of \p center.
  [[nodiscard]] std::vector<size_t> GetAgentsNear(WorldPosition center,
                                                  size_t radius) const {
    std::vector<size_t> result;
    for (size_t i = 0; i < agent_set.size(); ++i) {
      WorldPosition ap = agent_set[i]->GetState().position;
      size_t dx = std::max(ap.CellX(), center.CellX()) -
                  std::min(ap.CellX(), center.CellX());
      size_t dy = std::max(ap.CellY(), center.CellY()) -
                  std::min(ap.CellY(), center.CellY());
      if (dx + dy <= radius) result.push_back(i);
    }
    return result;
  }
};

}  // End of namespace cse498

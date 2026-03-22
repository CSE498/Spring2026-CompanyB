#pragma once
// N.B. most of this was cannibalized from MazeWorld to start with
#include "../core/WorldBase.hpp"

namespace cse498 {

  class TrafficWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t road_id; ///< Easy access to road CellType ID.
    size_t grass_id;  ///< Easy access to grass CellType ID.

    // Reworked by Claude — two cell types for traffic light phases,
    // with symbols showing which direction traffic may flow.
    size_t traffic_light_vertical_id;   ///< Traffic light allowing vertical movement ('|')
    size_t traffic_light_horizontal_id; ///< Traffic light allowing horizontal movement ('-')

    std::vector<WorldPosition> traffic_light_positions; ///< Positions of all traffic lights.

    enum TrafficLightPhase : int { ALLOW_VERTICAL = 0, ALLOW_HORIZONTAL = 1 };
    TrafficLightPhase traffic_light_phase{};
    int traffic_light_clock = 0;
    const int traffic_light_period = 3;

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("stay", REMAIN_STILL);
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    TrafficWorld() {
      road_id = main_grid.AddCellType("road", "Road to drive in", ' ');
      grass_id  = main_grid.AddCellType("grass",  "Grass that cars can't go on.", '#');
      // Written by Claude — register both traffic light phases with directional symbols
      traffic_light_vertical_id = main_grid.AddCellType(
        "traffic_light_vertical",
        "Traffic light allowing vertical (up/down) movement.", '|');
      traffic_light_horizontal_id = main_grid.AddCellType(
        "traffic_light_horizontal",
        "Traffic light allowing horizontal (left/right) movement.", '-');

      // Initial phase is ALLOW_VERTICAL, so traffic lights start as '|'
      main_grid.Load(std::vector<std::string>{"#################",
                                              "########        #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "#       |       #",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "#        ########",
                                              "#################"} );

      // Written by Claude — record all traffic light positions for phase swapping
      for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
        for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
          WorldPosition pos(x, y);
          if (main_grid[pos] == traffic_light_vertical_id) {
            traffic_light_positions.push_back(pos);
          }
        }
      }
    }
    ~TrafficWorld() = default;

    /// Allow the agents to move around the maze.
    int DoAction(AgentBase & agent, size_t action_id) override {
      // Determine where the agent is trying to move.
      WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
      WorldPosition new_position;
      switch (action_id) {
      case REMAIN_STILL: new_position = cur_position; break;
      case MOVE_UP:      new_position = cur_position.Up(); break;
      case MOVE_DOWN:    new_position = cur_position.Down(); break;
      case MOVE_LEFT:    new_position = cur_position.Left(); break;
      case MOVE_RIGHT:   new_position = cur_position.Right(); break;
      }

      if (!main_grid.IsValid(new_position)) { return false; }
      if (main_grid[new_position] == grass_id) { return false; }
      // Reworked by Claude — check against the phase-specific traffic light cell types
      const size_t dest_type = main_grid[new_position];
      if (dest_type == traffic_light_vertical_id) {
        // Only vertical movement allowed through this light
        if (action_id == MOVE_LEFT || action_id == MOVE_RIGHT) return false;
      } else if (dest_type == traffic_light_horizontal_id) {
        // Only horizontal movement allowed through this light
        if (action_id == MOVE_UP || action_id == MOVE_DOWN) return false;
      }

      // Set the agent to its new postion.
      agent.SetLocation(new_position);

      return true;
    }

    // Reworked by Claude — swap traffic light cell types to update both
    // movement rules and display symbol in one step
    void UpdateWorld() override {
      if (++traffic_light_clock == traffic_light_period) {
        traffic_light_clock = 0;
        traffic_light_phase = traffic_light_phase == TrafficLightPhase::ALLOW_HORIZONTAL ?
                                                     TrafficLightPhase::ALLOW_VERTICAL :
                                                     TrafficLightPhase::ALLOW_HORIZONTAL;
        // Swap the cell type at every traffic light position to match the new phase
        const size_t new_type = (traffic_light_phase == ALLOW_VERTICAL)
                                  ? traffic_light_vertical_id
                                  : traffic_light_horizontal_id;
        for (const auto& pos : traffic_light_positions) {
          main_grid[pos] = new_type;
        }
      }
    }

  };
// clang-format on
}  // End of namespace cse498

#pragma once
// N.B. most of this was cannibalized from MazeWorld to start with
#include "../core/WorldBase.hpp"

namespace cse498 {

  class TrafficWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t road_id; ///< Easy access to road CellType ID.
    size_t grass_id;  ///< Easy access to grass CellType ID.

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    TrafficWorld() {
      road_id = main_grid.AddCellType("road", "Road to drive in", ' ');
      grass_id  = main_grid.AddCellType("grass",  "Grass that cars can't go on.", '#');

      main_grid.Load(std::vector<std::string>{"#################",
                                              "########        #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "######## ###### #",
                                              "#               #",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "# ###### ########",
                                              "#        ########",
                                              "#################"} );
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

      // Set the agent to its new postion.
      agent.SetLocation(new_position);

      return true;
    }

  };
// clang-format on
}  // End of namespace cse498

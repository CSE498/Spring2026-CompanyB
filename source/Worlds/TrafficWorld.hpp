#pragma once
#include <algorithm>
#include <map>

#include "../core/WorldBase.hpp"
#include "../tools/WeightedSet.hpp"

namespace cse498 {

class TrafficWorld : public WorldBase {
 protected:
  enum ActionType {
    REMAIN_STILL = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
  };

  size_t road_id;   ///< ID of road cells, which agents can move on.
  size_t grass_id;  ///< ID of grass cells, which agents can't move on..

  // Reworked by Claude — two cell types for traffic light phases,
  // with symbols showing which direction traffic may flow.
  size_t traffic_light_vertical_id;    ///< Traffic light allowing vertical
                                       ///< movement ('|')
  size_t traffic_light_horizontal_id;  ///< Traffic light allowing horizontal
                                       ///< movement ('-')

  size_t spawn_id;        ///< ID of cells that spawn agents.
  size_t destination_id;  ///< ID of cells which are destinations that agents
                          ///< try to reach.

  std::vector<WorldPosition>
      traffic_light_positions;  ///< Positions of all traffic lights.
  std::vector<WorldPosition>
      spawner_positions;  ///< Positions of all spawner tiles.
  WeightedSet<WorldPosition>
      destination_positions;  // Weighted set to randomly assign destinations
  std::map<WorldPosition, std::string>
      destination_colours;  ///< Per-destination ANSI colour codes.

  /// @brief Indicates whether traffic lights in the world allow agents to pass
  /// through intersections vertically or horizontally.
  enum class TrafficLightPhase : int {
    ALLOW_VERTICAL = 0,
    ALLOW_HORIZONTAL = 1
  };
  /// @brief The TrafficLightPhase (allow vertical or horizontal movement) held
  /// by all traffic lights in the world.
  TrafficLightPhase traffic_light_phase{};
  /// @brief The number of turns that pass before all traffic lights flip to the
  /// opposite of what they currently are.
  static constexpr int traffic_light_period = 3;
  /// @brief Counts up each turn and gets reset when it reaches
  /// traffic_light_period.
  int traffic_light_clock = 0;

  /// @brief The number of turns that pass before a new agent is spawned from
  /// each spawner with a random destination.
  static constexpr int spawn_period = 20;

  /// @brief Counts up each turn and gets reset when it reaches spawn_period.
  int spawn_clock = 0;

  /// @brief The number of currently-active agents that have been spawned by
  /// spawner tiles. Incremented whenever a spawner spawns something,
  /// decremented whenever an agent reaches its destination and despawns.
  int num_spawned_agents = 0;
  /// @brief Cap on the number of active spawned agents that can exist at a
  /// time, to prevent the world from getting too chaotic.
  static constexpr int max_spawned_agents = 15;

  /// Provide the agent with movement actions.
  void ConfigAgent(AgentBase &agent) override {
    agent.AddAction("stay", REMAIN_STILL);
    agent.AddAction("up", MOVE_UP);
    agent.AddAction("down", MOVE_DOWN);
    agent.AddAction("left", MOVE_LEFT);
    agent.AddAction("right", MOVE_RIGHT);
  }

 public:
  TrafficWorld() {
    road_id = main_grid.AddCellType("road", "Road to drive in", '.');
    grass_id =
        main_grid.AddCellType("grass", "Grass that cars can't go on.", '#');
    // Written by Claude — register both traffic light phases with directional
    // symbols
    traffic_light_vertical_id = main_grid.AddCellType(
        "traffic_light_vertical",
        "Traffic light allowing vertical (up/down) movement.", '|');
    traffic_light_horizontal_id = main_grid.AddCellType(
        "traffic_light_horizontal",
        "Traffic light allowing horizontal (left/right) movement.", '-');

    spawn_id =
        main_grid.AddCellType("spawn", "Spawner for driving agents", 'S');

    destination_id = main_grid.AddCellType(
        "destination", "Destination for driving agents", 'D');

    // City-block grid: perimeter road + 2 inner horizontal + 2 inner vertical
    // roads forming 12 intersections.
    // '.' = road  '#' = grass/block  'S' = spawn  'D' = destination
    // '|' = traffic light (vertical phase, allows up/down)
    // '-' = traffic light (horizontal phase, allows left/right)
    // All lights start as '|' so they are found by the vertical-scan below.

    // clang-format off
    main_grid.Load(std::vector<std::string>{
        "###################################",
        "#..S.....|...............|........#",
        "#.#######.###############.#######.#",
        "#.#######.###############.#######D#",
        "#.#######.###############.#######.#",
        "#D#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#|.......|...............|.......|#",
        "#.#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#|.......|...............|.......|#",
        "#.#######.###############.#######.#",
        "#D#######.###############.#######.#",
        "#.#######.###############.#######.#",
        "#........|...............|........#",
        "###################################"});

    // clang-format on

    // Initially written by Claude, then modified — record all traffic light and
    // spawner positions
    for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
      for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
        WorldPosition pos(x, y);
        if (main_grid[pos] == traffic_light_vertical_id) {
          traffic_light_positions.push_back(pos);
        } else if (main_grid[pos] == spawn_id) {
          spawner_positions.push_back(pos);
        }
      }
    }
    const std::vector<std::string> colour_palette = {
        "\033[91m",  // bright red
        "\033[92m",  // bright green
        "\033[93m",  // bright yellow
        "\033[94m",  // bright blue
        "\033[95m",  // bright magenta
    };
    size_t colour_idx = 0;
    for (size_t y = 0; y < main_grid.GetHeight(); ++y) {
      for (size_t x = 0; x < main_grid.GetWidth(); ++x) {
        WorldPosition pos(x, y);
        if (main_grid[pos] == destination_id) {
          destination_positions.Insert(pos, 1.0);  // equal weight for now
          destination_colours.emplace(
              pos, colour_palette[colour_idx % colour_palette.size()]);
          ++colour_idx;
        }
      }
    }
  }

  /// @brief Return the ANSI colour code pre-assigned to a destination tile,
  ///        or an empty string if the position is not a destination.
  [[nodiscard]] const std::string &GetDestinationColour(
      const WorldPosition &pos) const {
    auto it = destination_colours.find(pos);
    if (it != destination_colours.end()) {
      return it->second;
    }
    static const std::string empty{};
    return empty;
  }
  ~TrafficWorld() = default;

  /// Allow the agents to move around the maze.
  int DoAction(AgentBase &agent, size_t action_id) override {
    // Determine where the agent is trying to move.
    WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
    assert(main_grid.IsValid(cur_position));

    WorldPosition new_position;
    switch (action_id) {
      case REMAIN_STILL:
        new_position = cur_position;
        break;
      case MOVE_UP:
        new_position = cur_position.Up();
        break;
      case MOVE_DOWN:
        new_position = cur_position.Down();
        break;
      case MOVE_LEFT:
        new_position = cur_position.Left();
        break;
      case MOVE_RIGHT:
        new_position = cur_position.Right();
        break;
      default:
        new_position = cur_position;
    }

    if (!main_grid.IsValid(new_position)) {
      return 0;
    }

    if (main_grid[new_position] == grass_id) {
      return 0;
    }
    // The following code is for collision detection between agents, which works
    // in a somewhat bizarre-looking way to simulate 2-lane roads with agents
    // moving in both directions. If an agent is stopped and an agent coming up
    // from behind it runs into it, like this: >  >| then we want to make the
    // moving agent stop behind the stopped agent: >>| Similarly if the agent is
    // trying to move into an agent moving perpendicular to it, like: >^ or >v
    // then that move shouldn't be allowed either.
    // But if two agents move into each other while moving in opposite
    // directions, like this: >< we should let them move past each other, ending
    // up like this: <>, then this: <  >
    auto new_pos_dir = DirectionOfDrivingAgentAt(new_position);
    auto cur_pos_dir = DirectionOfDrivingAgentAt(cur_position);

    if (cur_position != new_position && cur_pos_dir.has_value() &&
        new_pos_dir.has_value()) {
      // Directions correspond to ints as follows: N = 0, E = 1, S = 2, W = 3
      // So taking a direction and adding 1 mod 4 corresponds to turning right,
      // and adding 2 mod 4 corresponds to turning around.

      // (number) & 3 means bitwise and of the number with 00...011
      // which grabs the last 2 bits, which is the same as reducing mod 4.
      auto opposite_of_current = static_cast<Direction>(
          (static_cast<int>(cur_pos_dir.value()) + 2) & 3);
      if (new_pos_dir != opposite_of_current) {
        return 0;
      }
    }
    // Reworked by Claude — check against the phase-specific traffic light cell
    // types
    const size_t dest_type = main_grid[new_position];
    if (dest_type == traffic_light_vertical_id) {
      // Only vertical movement allowed through this light
      if (action_id == MOVE_LEFT || action_id == MOVE_RIGHT) return 0;
    } else if (dest_type == traffic_light_horizontal_id) {
      // Only horizontal movement allowed through this light
      if (action_id == MOVE_UP || action_id == MOVE_DOWN) return 0;
    }

    // Set the agent to its new postion.
    agent.SetLocation(new_position);
    return 1;
  }
  // This code and everything related to it is bad and I'm sorry for writing it.
  // In the future we'll definitely need better ways for agents to interact with
  // each other. Pushing it now because it works, at least.

  /// @brief Returns the direction of the DrivingAgent at the given position,
  /// if there is a DrivingAgent with that position and it hasn't been
  /// despawned. Otherwise, returns nullopt.
  std::optional<Direction> DirectionOfDrivingAgentAt(WorldPosition pos) const {
    auto it = std::find_if(
        agent_set.begin(), agent_set.end(), [&pos](agent_ptr_t const &agent) {
          return agent->GetLocation().AsWorldPosition() == pos;
        });
    if (it == agent_set.end()) {
      return std::nullopt;
    }
    if (auto agentAt = dynamic_cast<DrivingAgent *>(it->get())) {
      // effectively this turns off collision detection for agents that have
      // reached their destination
      if (agentAt->GetReachedDestination()) {
        return std::nullopt;
      }
      return std::make_optional(agentAt->GetDirection());
    }
    return std::nullopt;
  }

  bool AgentExistsAt(WorldPosition pos) const {
    auto it = std::find_if(
        agent_set.begin(), agent_set.end(), [&pos](agent_ptr_t const &agent) {
          return agent->GetLocation().AsWorldPosition() == pos;
        });
    return it != agent_set.end();
  }

  void UpdateWorld() override {
    UpdateTrafficLights();
    UpdateSpawners();
    HandleDestinations();
  }

  void RunAgents() override {
    for (const auto &agent_ptr : agent_set) {
      auto *driver = dynamic_cast<DrivingAgent *>(agent_ptr.get());
      if (driver && driver->GetReachedDestination()) continue;
      size_t action_id = agent_ptr->SelectAction(main_grid);
      int result = DoAction(*agent_ptr, action_id);
      agent_ptr->SetActionResult(result);
    }
  }

 private:
  // Reworked by Claude — swap traffic light cell types to update both
  // movement rules and display symbol in one step
  void UpdateTrafficLights() {
    if (++traffic_light_clock >= traffic_light_period) {
      traffic_light_clock = 0;
      traffic_light_phase =
          traffic_light_phase == TrafficLightPhase::ALLOW_HORIZONTAL
              ? TrafficLightPhase::ALLOW_VERTICAL
              : TrafficLightPhase::ALLOW_HORIZONTAL;
      // Swap the cell type at every traffic light position to match the new
      // phase
      const size_t new_type =
          (traffic_light_phase == TrafficLightPhase::ALLOW_VERTICAL)
              ? traffic_light_vertical_id
              : traffic_light_horizontal_id;
      for (const auto &pos : traffic_light_positions) {
        main_grid[pos] = new_type;
      }
    }
  }
  /// @brief Update the spawn clock by 1 tick. If it's time to spawn more
  /// agents, go to each spawner without an agent currently on top of it and
  /// spawn a new DrivingAgent with a randomly chosen destination.
  void UpdateSpawners() {
    if (++spawn_clock >= spawn_period) {
      spawn_clock = 0;
      for (const WorldPosition &pos : spawner_positions) {
        // Don't spawn on top of an existing agent
        if (main_grid[pos] == spawn_id && !AgentExistsAt(pos) &&
            num_spawned_agents < max_spawned_agents) {
          auto dest = destination_positions.GetRandomElement();
          if (dest.has_value()) {
            WorldPosition dest_pos = dest.value();
            auto &agent = AddAgent<DrivingAgent>("Car");
            agent.SetLocation(pos);
            agent.SetGridDestination(dest_pos.CellX(), dest_pos.CellY());
            agent.SetColour(GetDestinationColour(dest_pos));

            ++num_spawned_agents;
          }
        }
      }
    }
  }
  /// @brief Despawn any agents that have reached their destination.
  void HandleDestinations() {
    for (auto &agent_ptr : agent_set) {
      auto *driver = dynamic_cast<DrivingAgent *>(agent_ptr.get());
      if (!driver || driver->GetReachedDestination()) continue;

      WorldPosition pos = agent_ptr->GetLocation().AsWorldPosition();
      WorldPosition dest = driver->GetDestination().AsWorldPosition();

      if (pos.CellX() == dest.CellX() && pos.CellY() == dest.CellY()) {
        driver->SetReachedDestination(true);
        driver->SetSymbol('D');
        --num_spawned_agents;
        assert(num_spawned_agents >= 0);
      }
    }
  }
};
}  // End of namespace cse498

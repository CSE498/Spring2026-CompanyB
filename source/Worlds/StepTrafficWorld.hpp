#pragma once

#include <map>
#include <vector>

#include "../core/StepWorldBase.hpp"
#include "../tools/WeightedSet.hpp"
#include "TrafficData.hpp"

namespace cse498 {

class TrafficWorld : public StepWorldBase<TrafficData> {
  using Agent = StepAgentBase<TrafficData>;
  using AgentPtr = std::shared_ptr<Agent>;

  struct WorldErr {};

  // This is "zoomed in" to the definition in the introduction -- this is still defined within SimpleWorld
struct StepVisitor {
  // We do want to represent failure, but don't need to represent
  // any output, so we'll define & alias our return as such:
  using VisitRet = std::expected<void, WorldErr>;
  
  // We'll want to modify the agent and the container, so we'll hold on to
  // some references for them. You'll have to do the same for any other
  // non-variant external context desired.
  StepAgentBase<TrafficData> &agent;
  StepContainer &container;
  StepWorldBase &world;
  
  // Now we'll need to have an operator() overload for each Step type.
  VisitRet operator()(steps::MovementStep step) {
    // The simplest step -- the agent just wants to move to a space.
    if (world.GetGrid().IsValid(step.loc)) {
        // also need to check: step isn't onto grass; 
        // if trying to move onto a traffic light, it's in a phase which
        // allows this (need to know the agent's position and the new position for this);
        // whether a collision will happen.

        // should have some helper functions for this.
        
        // if all that is true, update the agent's location.
    }
    return {};
  }
  
  VisitRet operator()(steps::InfoStep step) {
    // When we get an info step, the agent would like to do something
    // conditionally (the container handles the branching internally).
    // It will provide us with the "aspect" about the world, and the
    // location it wants the info about. In return, we'll "inform" the
    // container with what has been requested.
    switch (step.aspect) {
      using Aspect = cse498::steps::InfoStep::Aspect;
    case Aspect::LOC_AVAIL: {
      // Just check whether the location is in-bounds and not a grass tile.
      // Don't bother checking anything about traffic lights and collisions
      container.inform(false);
      break;
    }
    case Aspect::OCCUPANCY_FRAC: {
      // TEMP (but I don't expect this branch to be used much anyway)
      container.inform(false);
      break;
    }
    case Aspect::OCCUPANCY_RAW: {
      // TEMP (but I don't expect this branch to be used much anyway)
      container.inform(false);
      break;
    }
    };
    return {};
  }

  // Note for someone who may want to refactor in the future to not have to fill
  // fill in overloads for steps we shouldn't reach (like ConditionalStep), we
  // could either (a) extend the InfoHandler approach for a defaulted visitor or
  // apply CRTP and have it build in these overloads.
  
  
  VisitRet operator()([[maybe_unused]] steps::ConditionalStep step) {
    // The compiler needs this, but this should never be reached. A
    // better solution is forthcoming, for now just leave empty.
    return {};
  }
  
  VisitRet operator()([[maybe_unused]] steps::ReconStep step) {
    // The compiler needs this, but this should (for now) never be
    // reached. A better solution is forthcoming, for now just leave empty.
    return {};
  }
};

 protected:
  size_t road_id{};   ///< ID of road cells, which agents can move on.
  size_t grass_id{};  ///< ID of grass cells, which agents can't move on..

  // Reworked by Claude — two cell types for traffic light phases,
  // with symbols showing which direction traffic may flow.
  size_t traffic_light_vertical_id{};    ///< Traffic light allowing vertical
                                         ///< movement ('|')
  size_t traffic_light_horizontal_id{};  ///< Traffic light allowing horizontal
                                         ///< movement ('-')

  size_t spawn_id{};        ///< ID of cells that spawn agents.
  size_t destination_id{};  ///< ID of cells which are destinations that agents
                            ///< try to reach.
  std::vector<WorldPosition>
      traffic_light_positions{};  ///< Positions of all traffic lights.
  std::vector<WorldPosition>
      spawner_positions{};  ///< Positions of all spawner tiles.
  WeightedSet<WorldPosition>
      destination_positions{};  // Weighted set to randomly assign destinations
  std::map<WorldPosition, std::string>
      destination_colours{};  ///< Per-destination ANSI colour codes.

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

  /// @brief Queue of IDs of despawned agents available for recycling.
  /// Written by Claude.
  std::queue<size_t> despawned_agent_ids{};

  // Pulled by Claude out of the preexisting constructor — registers cell types
  // and scans the loaded grid for traffic lights, spawners, and destinations.
 private:
  void RegisterCellTypes() {
    road_id = main_grid.AddCellType("road", "Road to drive in", '.');
    grass_id =
        main_grid.AddCellType("grass", "Grass that cars can't go on.", '#');
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
  }

  /// @brief Scan the grid after loading to find traffic lights, spawners,
  /// and destinations.
  void ScanGrid() {
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

 public:
  /// @brief Construct a TrafficWorld from a vector of strings representing
  /// the grid layout.
  /// Written by Claude.
  explicit TrafficWorld(const std::vector<std::string> &grid_lines) {
    RegisterCellTypes();
    main_grid.Load(grid_lines);
    ScanGrid();
  }

  /// @brief Construct a TrafficWorld by reading a grid layout from a file.
  /// Written by Claude.
  explicit TrafficWorld(const std::string &filepath) {
    RegisterCellTypes();
    std::ifstream file(filepath);
    assert(file.is_open() && "TrafficWorld: could not open grid file");
    main_grid.Load(file);
    ScanGrid();
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

  TrafficData DoAction(AgentPtr agent) override {
    StepContainer steps = agent->GetTurn();
    // do the sort of stuff in the "do_turns" example in steps.org
    // to run the steps in that container

    // move the agent if you end up with a movement step. Stop after you've
    // made a move (maybe?)

    // return the agent's TrafficData state but with the direction changed
    // based on what the move was. E.g. if the agent moved right/east then it
    // should now be pointing east.
  }

  void UpdateWorld() override {
    // this should be basically the same as the one in the old TrafficWorld
    // mainly we just need to update the helper functions that uses

    // handle traffic lights, spawners, destinations
  }
};
};  // namespace cse498
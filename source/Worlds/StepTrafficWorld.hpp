#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <print>
#include <ranges>
#include <thread>
#include <vector>

#include "../core/StepWorldBase.hpp"
#include "../tools/WeightedSet.hpp"
#include "TrafficData.hpp"

namespace cse498 {

class StepTrafficWorld : public StepWorldBase<TrafficData> {
  using Agent = StepAgentBase<TrafficData>;
  using AgentPtr = std::shared_ptr<Agent>;

  struct WorldErr {};

  // This is "zoomed in" to the definition in the introduction -- this is still
  // defined within SimpleWorld
  struct StepVisitor {
    // We do want to represent failure, but don't need to represent
    // any output, so we'll define & alias our return as such:
    using VisitRet = std::expected<void, WorldErr>;

    // We'll want to modify the agent and the container, so we'll hold on to
    // some references for them. You'll have to do the same for any other
    // non-variant external context desired.
    StepAgentBase<TrafficData> &agent;
    StepContainer &container;
    StepTrafficWorld &world;

    size_t grass_id{};

    // Now we'll need to have an operator() overload for each Step type.
    VisitRet operator()(steps::MovementStep step) {
      std::println("Desired pos: {}, {}", step.loc.CellX(), step.loc.CellY());
      // The simplest step -- the agent just wants to move to a space.
      if (!world.IsValid(step.loc) || world.IsGrass(step.loc)) {
        return {};

        // if trying to move onto a traffic light, it's in a phase which
        // allows this (need to know the agent's position and the new position
        // for this); whether a collision will happen.

        // should have some helper functions for this.

        // if all that is true, update the agent's location.
      }
      if (world.CanCollideWithAgentAt(agent, step.loc)) {
        return {};
      }
      auto curr_state = agent.GetState();
      curr_state.position = step.loc;
      agent.SetState(curr_state);

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
          container.inform(world.IsValid(step.target) &&
                           !world.IsGrass(step.target));
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

    // Note for someone who may want to refactor in the future to not have to
    // fill fill in overloads for steps we shouldn't reach (like
    // ConditionalStep), we could either (a) extend the InfoHandler approach for
    // a defaulted visitor or apply CRTP and have it build in these overloads.

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
  explicit StepTrafficWorld(const std::vector<std::string> &grid_lines) {
    RegisterCellTypes();
    main_grid.Load(grid_lines);
    ScanGrid();
  }

  /// @brief Construct a TrafficWorld by reading a grid layout from a file.
  /// Written by Claude.
  explicit StepTrafficWorld(const std::string &filepath) {
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
  ~StepTrafficWorld() = default;

  bool IsValid(const WorldPosition &pos) const {
    return main_grid.IsValid(pos);
  }

  bool IsGrass(const WorldPosition &pos) const {
    return main_grid.IsValid(pos) && main_grid[pos] == grass_id;
  }

  Direction GetOppositeDirection(const Direction dir) const {
    // (number) & 3 means bitwise and of the number with 00...011
    // which grabs the last 2 bits, which is the same as reducing mod 4.
    return static_cast<Direction>((static_cast<int>(dir) + 2) & 3);
  }

  bool CanCollideWithAgentAt(const Agent &agent,
                             const WorldPosition &pos) const {
    Direction opposite = GetOppositeDirection(agent.GetState().direction);
    auto is_agent_at_position = [&](const AgentPtr &ptr) {
      return ptr->GetState().is_active && ptr->GetState().position == pos;
    };
    auto has_opposite_direction = [&](const AgentPtr &ptr) {
      return ptr->GetState().direction == opposite;
    };
    return !std::ranges::empty(agent_set |
                               std::views::filter(is_agent_at_position) |
                               std::views::filter(has_opposite_direction));
  }

  TrafficData DoAction(AgentPtr agent) override {
    StepContainer steps = agent->GetTurn();
    StepVisitor visitor{*agent, steps, *this, grass_id};
    // do the sort of stuff in the "do_turns" example in steps.org
    // to run the steps in that container
    WorldPosition prev_position = agent->GetState().position;
    while (!steps.exhausted()) {
      std::expected<Step, StepErr> step = steps.get_next();
      if (!step.has_value()) {
        // TEMP
        break;
      }
      StepVisitor::VisitRet step_result = std::visit(visitor, step.value());

      if (!step_result.has_value()) {
        // Handle error
      }
    }
    // The position will be updated by the StepVisitor
    // so all we have left to update is the direction and is_active
    TrafficData new_state = agent->GetState();
    WorldPosition new_position = new_state.position;

    if (new_position == new_state.destination) {
      new_state.is_active = false;
    }

    // TODO: if position changed: update direction

    return new_state;
  }

  void UpdateWorld() override {
    // this should be basically the same as the one in the old TrafficWorld
    // mainly we just need to update the helper functions that uses

    // handle traffic lights, spawners, destinations
  }

  // =====================================================================
  //  Terminal display — written by Claude.
  //  Adapted from AutoInterface to work with the StepWorldBase/StepAgentBase
  //  API.  AutoInterface was an AgentBase that rendered each frame from
  //  inside SelectAction(); since StepWorldBase has no interface-agent
  //  concept, we integrate the display directly into the world's run loop.
  // =====================================================================
 private:
  using Clock = std::chrono::steady_clock;

  /// Time between displayed frames (configurable via SetFrameDelay).
  std::chrono::milliseconds frame_delay{200};

  /// When the last frame was drawn (used for pacing).
  Clock::time_point last_frame_time{};

  /// Maps a direction enum to the character that represents an agent
  /// facing that way, matching the symbols the old DrivingAgent used.
  static constexpr char DirectionSymbol(Direction dir) {
    switch (dir) {
      case Direction::North:
        return '^';
      case Direction::East:
        return '>';
      case Direction::South:
        return 'v';
      case Direction::West:
        return '<';
    }
    return '?';
  }

  /// Render the current grid state plus all active agents to the
  /// terminal, colouring destination tiles and agents just like
  /// AutoInterface did.
  void DrawGrid() {
    const size_t W = main_grid.GetWidth();
    const size_t H = main_grid.GetHeight();

    // Build a character grid from the underlying cell symbols.
    std::vector<std::string> symbol_grid(H);
    std::vector<std::vector<std::string>> colour_grid(
        H, std::vector<std::string>(W));

    for (size_t y = 0; y < H; ++y) {
      symbol_grid[y].resize(W);
      for (size_t x = 0; x < W; ++x) {
        symbol_grid[y][x] = main_grid.GetSymbol(WorldPosition{x, y});
      }
    }

    // Colour destination tiles using the pre-assigned palette.
    for (size_t y = 0; y < H; ++y) {
      for (size_t x = 0; x < W; ++x) {
        const std::string &col = GetDestinationColour(WorldPosition{x, y});
        if (!col.empty()) colour_grid[y][x] = col;
      }
    }

    // Stamp active agents onto the grid.
    for (const auto &agent_ptr : agent_set) {
      const TrafficData &state = agent_ptr->GetState();
      if (!state.is_active) continue;

      const size_t ax = state.position.CellX();
      const size_t ay = state.position.CellY();
      if (ax >= W || ay >= H) continue;

      symbol_grid[ay][ax] = DirectionSymbol(state.direction);
      if (!state.colour.empty()) {
        colour_grid[ay][ax] = state.colour;
      }
    }

    // Clear screen with ANSI escape, then print the grid with a box
    // around it — same presentation as AutoInterface.
    std::cout << "\033[2J\033[H";
    std::cout << '+' << std::string(W, '-') << "+\n";
    for (size_t y = 0; y < H; ++y) {
      std::cout << '|';
      for (size_t x = 0; x < W; ++x) {
        const std::string &col = colour_grid[y][x];
        if (!col.empty()) {
          std::cout << col << symbol_grid[y][x] << "\033[0m";
        } else {
          std::cout << symbol_grid[y][x];
        }
      }
      std::cout << "|\n";
    }
    std::cout << '+' << std::string(W, '-') << "+\n";
    std::cout.flush();
  }

 public:
  /// Set the delay between displayed frames (default 200 ms).
  StepTrafficWorld &SetFrameDelay(std::chrono::milliseconds delay) {
    frame_delay = delay;
    return *this;
  }

  /// Run the simulation with terminal display.
  /// This replaces the old pattern of adding an AutoInterface agent to
  /// the world: it calls RunAgents() and UpdateWorld() in a loop and
  /// renders the grid to the terminal between ticks, sleeping as
  /// needed to maintain the configured frame rate.
  void RunWithDisplay() {
    run_over = false;
    last_frame_time = Clock::now();

    while (!run_over) {
      // Pace the loop so each frame takes at least frame_delay.
      auto now = Clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - last_frame_time);
      if (elapsed < frame_delay) {
        std::this_thread::sleep_for(frame_delay - elapsed);
      }
      last_frame_time = Clock::now();

      DrawGrid();
      RunAgents();
      UpdateWorld();
    }
  }
};
};  // namespace cse498
#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <ranges>
#include <thread>
#include <vector>

#include "../core/AgentData.hpp"
#include "../core/StepWorldBase.hpp"
#include "../tools/WeightedSet.hpp"

namespace cse498 {

template <typename SpawnedAgent>
class StepTrafficWorld : public StepWorldBase<TrafficData> {
  using Agent = StepAgentBase<TrafficData>;
  using AgentPtr = std::shared_ptr<Agent>;
  // May want to change this later with e.g an enum of error codes, but since it
  // isn't being used too heavily this is probably fine for now
  struct WorldErr {
    std::string message = "";
  };

  struct StepVisitor {
    // We do want to represent failure, but don't need to represent
    // any output, so we'll define & alias our return as such:
    using VisitRet = std::expected<void, WorldErr>;

    StepAgentBase<TrafficData> &agent;
    StepContainer &container;
    StepTrafficWorld &world;

    // NOTE: all this "typename StepVisitor::VisitRet" stuff (instead of just
    // "VisitRet") is for some reason required when adding "template <typename
    // SpawnedAgent>".
    typename StepVisitor::VisitRet operator()(steps::MovementStep step) {
      auto can_move = world.CanMakeMoveAt(agent, step.loc);
      if (!can_move.has_value()) {
        return std::unexpected<WorldErr>(can_move.error());
      }
      if (!can_move.value()) return {};

      WorldPosition old_pos = agent.GetState().position;
      auto curr_state = agent.GetState();
      curr_state.position = step.loc;
      curr_state.direction = world.GetNewDirection(old_pos, step.loc).value();
      agent.SetState(curr_state, LogLevel::Normal, world.GetTickCount());

      return {};
    }

    typename StepVisitor::VisitRet operator()(steps::InfoStep step) {
      switch (step.aspect) {
        using Aspect = cse498::steps::InfoStep::Aspect;
        case Aspect::LOC_AVAIL: {
          container.inform(
              world.CanMakeMoveAt(agent, step.target).value_or(false));
          break;
        }
        // we consider a space to have a capacity of 2, since normally it should
        // contain at most 2 agents (moving in opposite directions)
        case Aspect::OCCUPANCY_FRAC: {
          container.inform(static_cast<double>(std::ranges::count_if(
                               world.agent_set,
                               [=](AgentPtr const &ptr) {
                                 return ptr->GetState().position == step.target;
                               })) /
                           2.0);
          break;
        }
        case Aspect::OCCUPANCY_RAW: {
          container.inform(static_cast<int>(
              std::ranges::count_if(world.agent_set, [=](AgentPtr const &ptr) {
                return ptr->GetState().position == step.target;
              })));
          break;
        }
      };
      return {};
    }

    typename StepVisitor::VisitRet operator()(
        [[maybe_unused]] steps::ConditionalStep step) {
      // The compiler needs this, but this should never be reached. A
      // better solution is forthcoming, for now just leave empty.
      return {};
    }

    typename StepVisitor::VisitRet operator()(
        [[maybe_unused]] steps::ReconStep step) {
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

  // ID of cells that spawn agents.
  size_t spawn_fast_id{};  // < Id for Fast spawners that uses the fast clock
  size_t
      spawn_normal_id{};  // < Id for Normal spawners that uses the normal clock
  size_t spawn_slow_id{};  // < Id for Slow spawners that uses the Slow clock

  // Containers for the 3 different types of spawners
  std::vector<WorldPosition>
      fast_spawner_positions{};  // < fast Spawner location container
  std::vector<WorldPosition>
      normal_spawner_positions{};  // < normal Spawner location container
  std::vector<WorldPosition>
      slow_spawner_positions{};  // < slow Spawner location container

  size_t destination_id{};  ///< ID of cells which are destinations that agents
                            ///< try to reach.
  std::vector<WorldPosition>
      traffic_light_positions{};  ///< Positions of all traffic lights.
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
  /// each spawner with a random destination. This is for the Fast speed
  static constexpr int fast_spawn_period = 10;
  /// @brief The number of turns that pass before a new agent is spawned from
  /// each spawner with a random destination. This is for the Normal speed
  static constexpr int normal_spawn_period = 20;
  /// @brief The number of turns that pass before a new agent is spawned from
  /// each spawner with a random destination. This is for the Slow speed
  static constexpr int slow_spawn_period = 30;

  /// @brief Counts up each turn and gets reset when it reaches spawn_period.
  /// Fast
  int fast_spawn_clock = 0;
  /// @brief Counts up each turn and gets reset when it reaches spawn_period.
  /// Normal
  int normal_spawn_clock = 0;
  /// @brief Counts up each turn and gets reset when it reaches spawn_period.
  /// Slow
  int slow_spawn_clock = 0;

  // Cyan shades for spawner tile display
  /// @brief Bright Cyan for the fast spawner
  inline static const std::string fast_spawn_colour = "\033[96m";
  /// @brief Reg Cyan for the normal spawner
  inline static const std::string normal_spawn_colour = "\033[36m";
  /// @brief Dim Cyan for the slow spawner
  inline static const std::string slow_spawn_colour = "\033[2;36m";

  /// @brief The number of currently-active agents that have been spawned by
  /// spawner tiles. Incremented whenever a spawner spawns something,
  /// decremented whenever an agent reaches its destination and despawns.
  int num_spawned_agents = 0;
  /// @brief Cap on the number of active spawned agents that can exist at a
  /// time, to prevent the world from getting too chaotic.
  static constexpr int max_spawned_agents = 50;

  /// @brief Queue of IDs of despawned agents available for recycling.
  /// Written by Claude.
  std::queue<size_t> despawned_agent_ids{};

 private:
  // Pulled by Claude out of the preexisting constructor — registers cell types
  // and scans the loaded grid for traffic lights, spawners, and destinations.
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

    spawn_fast_id = main_grid.AddCellType("spawn_fast", "Fast spawner", 'F');
    spawn_normal_id =
        main_grid.AddCellType("spawn_normal", "Normal spawner", 'N');
    spawn_slow_id = main_grid.AddCellType("spawn_slow", "Slow spawner", 'S');
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
        } else if (main_grid[pos] == spawn_fast_id) {
          fast_spawner_positions.push_back(pos);
        } else if (main_grid[pos] == spawn_normal_id) {
          normal_spawner_positions.push_back(pos);
        } else if (main_grid[pos] == spawn_slow_id) {
          slow_spawner_positions.push_back(pos);
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
  /// @brief Return the direction that an agent should face after moving from
  /// "pos" to "new_pos", or an error if the move is invalid (only moves 1
  /// square up/down/left/right are valid).
  /// @param pos Agent's current position
  /// @param new_pos Position the agent is attempting to move to
  /// @return Direction from pos to new_pos if move is valid, error otherwise
  [[nodiscard]] std::expected<Direction, WorldErr> GetNewDirection(
      WorldPosition pos, WorldPosition new_pos) const {
    size_t old_x = pos.CellX();
    size_t old_y = pos.CellY();
    size_t new_x = new_pos.CellX();
    size_t new_y = new_pos.CellY();
    Direction new_dir{};
    if (new_x == old_x) {
      if (new_y == old_y - 1) {
        new_dir = Direction::North;
      } else if (new_y == old_y + 1) {
        new_dir = Direction::South;
      } else {
        return std::unexpected<WorldErr>("invalid move");
      }
    } else if (new_y == old_y) {
      if (new_x == old_x - 1) {
        new_dir = Direction::West;
      } else if (new_x == old_x + 1) {
        new_dir = Direction::East;
      } else {
        return std::unexpected<WorldErr>("invalid move");
      }
    } else {
      return std::unexpected<WorldErr>("invalid move");
    }
    return new_dir;
  }
  /// @brief Returns a bool signaling whether the agent can move from its
  /// current position to the new position--or, if the move is invalid, an
  /// error.
  /// @note The rules here are a bit complicated so I'll summarize them in
  /// prose. An agent can move from A to B if:
  ///
  /// Either A = B, or B is directly up/down/left/right from A. No diagonal
  /// moves are allowed and no agent is allowed to move more than 2 steps at at
  /// time.
  ///
  /// B is a valid location (i.e. not out-of-bounds) and not an impassable
  /// "grass" tile.
  ///
  /// The agent isn't trying to move backward (i.e. the opposite of the
  /// direction it's facing), though this is allowed if the agent is at a dead
  /// end (grass tiles on all but one side) and has to turn around.
  ///
  /// The agent isn't trying to move horizontally/vertically into a traffic
  /// light that's currently blocking horizontal/vertical moves.
  ///
  /// The agent(s) on B must all be facing opposite to the direction that the
  /// agent would be in if it completed the move. (This is to simulate 2-lane
  /// roads, where agents can pass each other if they're moving in opposite
  /// directions, make turns at intersections as long as the street they're
  /// trying to turn onto only has cars in the opposite-direction lane, and so
  /// on.)
  [[nodiscard]] std::expected<bool, WorldErr> CanMakeMoveAt(
      const Agent &agent, const WorldPosition &new_pos) const {
    if (!IsValid(new_pos) || IsGrass(new_pos)) {
      return false;
    }

    WorldPosition pos = agent.GetState().position;
    std::expected<Direction, WorldErr> new_dir_ret =
        GetNewDirection(pos, new_pos);
    if (!new_dir_ret.has_value()) {
      return std::unexpected<WorldErr>(new_dir_ret.error());
    }
    Direction new_dir = new_dir_ret.value();

    if (new_dir == GetOppositeDirection(agent.GetState().direction) &&
        !IsDeadEnd(pos)) {
      return false;
    }

    if (HorizontalBlockedAt(new_pos) &&
        (new_dir == Direction::East || new_dir == Direction::West)) {
      return false;
    } else if (VerticalBlockedAt(new_pos) &&
               (new_dir == Direction::North || new_dir == Direction::South)) {
      return false;
    }

    if (CanCollideWithAgentAt(new_dir, new_pos)) {
      return false;
    }
    return true;
  }

  [[nodiscard]] bool IsValid(const WorldPosition &pos) const {
    return main_grid.IsValid(pos);
  }

  [[nodiscard]] bool IsGrass(const WorldPosition &pos) const {
    return main_grid.IsValid(pos) && main_grid[pos] == grass_id;
  }
  /// @brief Returns whether the given position has a traffic-light tile which
  /// is currently blocking horizontal traffic.
  [[nodiscard]] bool HorizontalBlockedAt(const WorldPosition &pos) const {
    return main_grid.IsValid(pos) &&
           main_grid[pos] == traffic_light_vertical_id;
  }
  /// @brief Returns whether the given position has a traffic-light tile which
  /// is currently blocking vertical traffic.
  [[nodiscard]] bool VerticalBlockedAt(const WorldPosition &pos) const {
    return main_grid.IsValid(pos) &&
           main_grid[pos] == traffic_light_horizontal_id;
  }
  /// @brief Returns whether the given position is surrounded by grass on all
  /// but one side.
  [[nodiscard]] bool IsDeadEnd(const WorldPosition &pos) const {
    int grass_count = 0;
    if (IsGrass(pos.Up())) grass_count++;
    if (IsGrass(pos.Down())) grass_count++;
    if (IsGrass(pos.Left())) grass_count++;
    if (IsGrass(pos.Right())) grass_count++;

    return grass_count == 3;
  }

  [[nodiscard]] Direction GetOppositeDirection(const Direction dir) const {
    // (number) & 3 means bitwise and of the number with 00...011
    // which grabs the last 2 bits, which is the same as reducing mod 4.
    return static_cast<Direction>((static_cast<int>(dir) + 2) & 3);
  }

  [[nodiscard]] bool CanCollideWithAgentAt(const Direction agent_direction,
                                           const WorldPosition &pos) const {
    Direction opposite = GetOppositeDirection(agent_direction);
    auto is_agent_at_position = [&](const AgentPtr &ptr) {
      return ptr->GetState().is_active && ptr->GetState().position == pos;
    };
    auto not_opposite_direction = [&](const AgentPtr &ptr) {
      return ptr->GetState().direction != opposite;
    };
    return !std::ranges::empty(agent_set |
                               std::views::filter(is_agent_at_position) |
                               std::views::filter(not_opposite_direction));
  }

  [[nodiscard]] TrafficData DoAction(AgentPtr agent) override {
    if (!agent->GetState().is_active) {
      return agent->GetState();
    }
    StepContainer steps = agent->GetTurn();
    StepVisitor visitor{*agent, steps, *this};
    while (!steps.exhausted()) {
      std::expected<Step, StepErr> step = steps.get_next();
      if (!step.has_value()) {
        // TODO: better error handling
        break;
      }
      typename StepVisitor::VisitRet step_result =
          std::visit(visitor, step.value());

      if (!step_result.has_value()) {
        // TODO: better error handling
      }
    }
    // The position will be updated by the StepVisitor
    // so all we have left to update is the direction and is_active
    TrafficData new_state = agent->GetState();
    WorldPosition new_position = new_state.position;

    if (new_state.destination.has_value() &&
        new_position == *new_state.destination) {
      new_state.is_active = false;
      despawned_agent_ids.push(agent->GetId());
      --num_spawned_agents;
    }

    new_state.symbol = DirectionSymbol(new_state.direction);

    return new_state;
  }

  void UpdateWorld() override {
    UpdateTrafficLights();
    UpdateSpawners();
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

  [[nodiscard]] bool AgentExistsAt(WorldPosition pos) const {
    auto agent_at = [&](const AgentPtr &ptr) {
      return ptr->GetState().position == pos;
    };
    return std::ranges::find_if(agent_set, agent_at) != agent_set.end();
  }

  /// Pulls out common logic for the spawners and simplifies the update spwaners
  /// go to each spawner without an agent currently on top of it and
  /// spawn a new DrivingAgent with a randomly chosen destination.
  void SpawnFromPositions(const std::vector<WorldPosition> &positions) {
    for (const WorldPosition &pos : positions) {
      if (!AgentExistsAt(pos) && num_spawned_agents < max_spawned_agents) {
        auto dest = destination_positions.GetRandomElement();
        if (dest.has_value()) {
          WorldPosition dest_pos = dest.value();

          if (!despawned_agent_ids.empty()) {
            RecycleDespawnedAgent(pos, dest_pos);
          } else {
            TrafficData state = {dest_pos, pos, Direction::East,
                                 true,     '>', GetDestinationColour(dest_pos)};
            AddAgent<SpawnedAgent>(state);
          }
          ++num_spawned_agents;
        }
      }
    }
  }
  /// @brief Update the spawn clock by 1 tick. If it's time to spawn more
  /// agents, Uses the 3 new Timers and idas and containser for the 3 speeds
  /// of spawners
  void UpdateSpawners() {
    if (++fast_spawn_clock >= fast_spawn_period) {
      fast_spawn_clock = 0;
      SpawnFromPositions(fast_spawner_positions);
    }
    if (++normal_spawn_clock >= normal_spawn_period) {
      normal_spawn_clock = 0;
      SpawnFromPositions(normal_spawner_positions);
    }
    if (++slow_spawn_clock >= slow_spawn_period) {
      slow_spawn_clock = 0;
      SpawnFromPositions(slow_spawner_positions);
    }
  }

  // Modified from code written by Claude.
  // Pulls, from despawned_agent_ids, the ID of an agent that previously
  // despawned and is now inactive, then respawns that agent at the given
  // spawner with the given destination.
  void RecycleDespawnedAgent(const WorldPosition &spawner_pos,
                             const WorldPosition &dest_pos) {
    assert(!despawned_agent_ids.empty());
    size_t reuse_id = despawned_agent_ids.front();
    despawned_agent_ids.pop();
    auto driver = agent_set.at(reuse_id);
    // Note for future use. Currently a non-DrivingAgent id should never make it
    // into despawned_agent_ids since those are the only agents that support
    // spawning, despawning, and destinations. In the future, with multiple
    // agent types, we'll find a way to relax this.
    TrafficData state = driver->GetState();
    state.position = spawner_pos;
    state.destination = dest_pos;
    state.direction = Direction::East;
    state.is_active = true;
    state.symbol = '>';
    state.colour = GetDestinationColour(dest_pos);
    driver->SetState(state, LogLevel::Normal, GetTickCount());
  }

  // =====================================================================
  //  Terminal display — written by Claude.
  //  Adapted from AutoInterface to work with the StepWorldBase/StepAgentBase
  //  API.  AutoInterface was an AgentBase that rendered each frame from
  //  inside SelectAction(); since StepWorldBase has no interface-agent
  //  concept, we integrate the display directly into the world's run loop.
  // =====================================================================

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

    // Colour spawner tiles with cyan shades, all displayed as 'S'
    auto colourSpawners = [&](const std::vector<WorldPosition> &positions,
                              const std::string &colour) {
      for (const auto &pos : positions) {
        size_t x = pos.CellX(), y = pos.CellY();
        symbol_grid[y][x] = 'S';
        colour_grid[y][x] = colour;
      }
    };

    colourSpawners(fast_spawner_positions, fast_spawn_colour);
    colourSpawners(normal_spawner_positions, normal_spawn_colour);
    colourSpawners(slow_spawner_positions, slow_spawn_colour);

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
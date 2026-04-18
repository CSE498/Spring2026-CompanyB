/**
 * Group13_main.cpp
 * @brief Demo of InfectiousWorld using the step-based agent system.
 *
 * compile (from demo/ directory):
 *   g++ -std=c++23 -DDEMO_GROUP13 -I../source \
 *       Group13_main.cpp -o infectious_demo \
 *       ../source/tools/Point.cpp ../source/tools/Box.cpp
 * run:
 *   ./infectious_demo
 **/

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../source/Agents/ScriptedAgent.hpp"
#include "../source/Agents/SwarmingAgent.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"

using namespace cse498;

// ---------------------------------------------------------------------------
// Demo constants
// ---------------------------------------------------------------------------
namespace demo {
constexpr int    kFrameDelayMs  = 400;
constexpr size_t kSimTicks      = 160;
constexpr size_t kNumPacers     = 8;    // agent IDs 0..7 are pacers
constexpr size_t kGridW         = 44;
constexpr size_t kGridH         = 18;
constexpr size_t kSpawnInterval = 15;   // spawn a new walker every N ticks
constexpr size_t kMaxAgents     = 50;   // cap so the grid doesn't overflow

// Entry point for new spawns — top-left door into the left room
constexpr size_t kSpawnX = 2;
constexpr size_t kSpawnY = 2;

// Quarantine zone: bottom-right room (grid coords)
constexpr double kQX1 = 30.0, kQY1 = 10.0;
constexpr double kQX2 = 42.0, kQY2 = 16.0;
}  // namespace demo

// ---------------------------------------------------------------------------
// ANSI colours
// ---------------------------------------------------------------------------
namespace ansi {
constexpr const char* RESET   = "\033[0m";
constexpr const char* BOLD    = "\033[1m";
constexpr const char* GREEN   = "\033[32m";
constexpr const char* RED     = "\033[31m";
constexpr const char* BLUE    = "\033[34m";
constexpr const char* CYAN    = "\033[36m";
constexpr const char* YELLOW  = "\033[33m";
constexpr const char* GREY    = "\033[90m";
// backgrounds
constexpr const char* BG_YELLOW = "\033[43m";
constexpr const char* BG_DARK   = "\033[100m";
}  // namespace ansi

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void DrawWorld(const InfectiousWorld& world) {
  const WorldGrid& grid   = world.GetGrid();
  size_t W = grid.GetWidth(), H = grid.GetHeight();

  // Build display layers
  struct Cell { char sym; const char* fg; const char* bg; };
  std::vector<std::vector<Cell>> display(H,
      std::vector<Cell>(W, {'.', ansi::GREY, ""}));

  // 1. Background tiles
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      char s = grid.GetSymbol(WorldPosition{x, y});
      if (s == '#') {
        display[y][x] = {'#', ansi::GREY, ""};
      } else if (world.IsInQuarantine(WorldPosition{x, y})) {
        display[y][x] = {'~', ansi::YELLOW, ansi::BG_YELLOW};
      } else {
        display[y][x] = {'.', ansi::GREY, ""};
      }
    }
  }

  
  //    Colour encodes health: green=susceptible, red=infected, blue=recovered.
  for (size_t id = 0; id < world.GetNumAgents(); ++id) {
    DiseaseData st  = world.GetAgentState(id);
    WorldPosition p = st.position;
    bool is_pacer   = (id < demo::kNumPacers);

    const char* col = ansi::GREEN;
    if (st.health == HealthState::INFECTED)  col = ansi::RED;
    if (st.health == HealthState::RECOVERED) col = ansi::BLUE;

    char sym = is_pacer ? 'P' : 'W';
    display[p.CellY()][p.CellX()] = {sym, col, ""};
  }

  // 3. Render
  std::cout << "\033[2J\033[H";
  std::cout << ansi::BOLD
            << "=== Infectious Disease Simulation (Group 13) ===\n"
            << ansi::RESET;
  std::cout << "  Tick: " << std::setw(4) << world.GetTickCount()
            << "   |   "
            << ansi::GREEN << "Susceptible: "
            << std::setw(3) << world.GetSusceptibleCount() << ansi::RESET
            << "   "
            << ansi::RED   << "Infected: "
            << std::setw(3) << world.GetInfectedCount()    << ansi::RESET
            << "   "
            << ansi::BLUE  << "Recovered: "
            << std::setw(3) << world.GetRecoveredCount()   << ansi::RESET
            << "\n\n";

  std::cout << "  +" << std::string(W, '-') << "+\n";
  for (size_t y = 0; y < H; ++y) {
    std::cout << "  |";
    for (size_t x = 0; x < W; ++x) {
      const auto& c = display[y][x];
      std::cout << c.bg << c.fg << c.sym << ansi::RESET;
    }
    std::cout << "|\n";
  }
  std::cout << "  +" << std::string(W, '-') << "+\n\n";

  // Legend
  std::cout
    << "  Agent types:  "
    << ansi::GREEN << "P" << ansi::RESET << " Pacer (Scripted)   "
    << ansi::CYAN  << "W" << ansi::RESET << " Walker (Swarming)\n"
    << "  Health:       "
    << ansi::GREEN << "green"  << ansi::RESET << "=susceptible  "
    << ansi::RED   << "red"    << ansi::RESET << "=infected  "
    << ansi::BLUE  << "blue"   << ansi::RESET << "=recovered\n"
    << "  Background:   "
    << ansi::GREY  << "#" << ansi::RESET << " wall   "
    << ansi::YELLOW << ansi::BG_YELLOW << "~" << ansi::RESET
    << ansi::YELLOW << " quarantine zone (movement blocked)" << ansi::RESET
    << "\n"
    << "  Spawning:     "
    << "new " << ansi::CYAN << "W" << ansi::RESET
    << " agents enter every " << demo::kSpawnInterval
    << " ticks at (" << demo::kSpawnX << "," << demo::kSpawnY << ")   "
    << "total agents: " << world.GetNumAgents()
    << " / " << demo::kMaxAgents << "\n";
}

// ---------------------------------------------------------------------------
// Demo setup & loop
// ---------------------------------------------------------------------------
int RunInfectiousDemo() {
  InfectiousWorld world(demo::kGridW, demo::kGridH);
  WorldGrid& grid = world.GetGrid();

  // --- Border walls ---
  for (size_t y = 0; y < demo::kGridH; ++y) {
    grid[0,               y] = world.GetWallID();
    grid[demo::kGridW - 1,y] = world.GetWallID();
  }
  for (size_t x = 0; x < demo::kGridW; ++x) {
    grid[x, 0               ] = world.GetWallID();
    grid[x, demo::kGridH - 1] = world.GetWallID();
  }

  // --- Internal divider: two main rooms with a gap passage ---
  for (size_t y = 1; y < 12; ++y)
    grid[22, y] = world.GetWallID();
  // passage at y=12..13 (gap)

  // --- Quarantine room: walled off in bottom-right with one entry ---
  for (size_t x = 29; x < demo::kGridW - 1; ++x)
    grid[x, 10] = world.GetWallID();
  for (size_t y = 10; y < demo::kGridH - 1; ++y)
    grid[29, y] = world.GetWallID();
  // door at (30, 10) left open (skip that cell)
  grid[30, 10] = world.GetFloorID();

  // --- Quarantine zone (movement restricted by world logic) ---
  world.AddQuarantineZone(
      Box::FromCorners(Point(demo::kQX1, demo::kQY1),
                       Point(demo::kQX2, demo::kQY2)));

  // --- Disease parameters ---
  world.SetTransmissionRate(0.35);
  world.SetInfectionRadius(2.0);
  world.SetInfectionDuration(14);
  world.SetImmunityDuration(25);

  // -----------------------------------------------------------------------
  // Spawn agents — demonstrates point 2: spawn & execute agent logic
  // -----------------------------------------------------------------------

  // Pacers: IDs 0..7, structured back-and-forth movement
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{ 3,  3}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{ 7,  5}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{12,  7}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{18,  4}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{ 4, 13}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{10, 15}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{25,  3}});
  world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{WorldPosition{27,  8}});

  // Random walkers: IDs 8..17, autonomous roaming
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{ 2,  8}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{ 5, 11}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{ 9,  4}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{14, 14}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{19,  9}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{20, 15}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{24,  6}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{25, 14}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{32, 11}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{38, 12}});

  // Patient zero — pacer in the left room
  world.InfectAgent(0);

  // --- Main simulation loop ---
  for (size_t tick = 0; tick < demo::kSimTicks; ++tick) {
    // Dynamic agent spawning: introduce a new infected random walker every N ticks
    if (tick > 0 && tick % demo::kSpawnInterval == 0 &&
        world.GetNumAgents() < demo::kMaxAgents) {
      world.AddAgent<SwarmingAgent<DiseaseData>>(
          DiseaseData{WorldPosition{demo::kSpawnX, demo::kSpawnY}});
      world.InfectAgent(world.GetNumAgents() - 1);  // spawned agents arrive infected
    }

    DrawWorld(world);
    world.RunAgents();      // each agent calls GetTurn(); world calls DoAction()
    world.UpdateWorld();    // spread infection, advance health timers
    std::this_thread::sleep_for(
        std::chrono::milliseconds(demo::kFrameDelayMs));
  }

  DrawWorld(world);
  std::cout << "\n" << ansi::BOLD << "=== Simulation Complete ===\n" << ansi::RESET;
  std::cout << "  Final — "
            << "Susceptible: " << world.GetSusceptibleCount()
            << "  Infected: "  << world.GetInfectedCount()
            << "  Recovered: " << world.GetRecoveredCount() << "\n";
  return 0;
}

#if defined(DEMO_GROUP13)
int main() { return RunInfectiousDemo(); }
#endif

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

#include "../source/Agents/dummyStepPacingAgent.hpp"
#include "../source/Agents/SwarmingAgent.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"

using namespace cse498;

// ---------------------------------------------------------------------------
// Demo constants
// ---------------------------------------------------------------------------
namespace demo {
constexpr int    kFrameDelayMs  = 200;
constexpr size_t kSimTicks      = 400;
constexpr size_t kNumPacers     = 6;    // IDs 0..5: building 
constexpr size_t kGridW         = 90;
constexpr size_t kGridH         = 33;
constexpr size_t kSpawnInterval = 12;   // new infected visitor every N ticks
constexpr size_t kMaxAgents     = 60;

// Red Cedar River rows
constexpr size_t kRiverY1 = 14;
constexpr size_t kRiverY2 = 16;

// Quarantine ward inside Olin Health Center 
constexpr double kQX1 = 77.0, kQY1 = 18.0;
constexpr double kQX2 = 89.0, kQY2 = 30.0;

// Three spawn entry points
constexpr size_t kSpawnSources = 3;
constexpr size_t kSpawnX[kSpawnSources] = {8, 44, 73};
constexpr size_t kSpawnY[kSpawnSources] = {12, 12, 12};
}  // namespace demo


// ANSI colours

namespace ansi {
constexpr const char* RESET     = "\033[0m";
constexpr const char* BOLD      = "\033[1m";
constexpr const char* WHITE     = "\033[97m";
constexpr const char* GREEN     = "\033[32m";
constexpr const char* RED       = "\033[31m";
constexpr const char* BLUE      = "\033[34m";
constexpr const char* CYAN      = "\033[36m";
constexpr const char* YELLOW    = "\033[33m";
constexpr const char* GREY      = "\033[90m";
constexpr const char* BG_BLUE   = "\033[44m";
constexpr const char* BG_YELLOW = "\033[43m";
}  // namespace ansi

// ----------------------------------
// Draw
// -----------------------------------
void DrawWorld(const InfectiousWorld& world) {
  const WorldGrid& grid = world.GetGrid();
  size_t W = grid.GetWidth(), H = grid.GetHeight();

  struct Cell { char sym; const char* fg; const char* bg; };
  std::vector<std::vector<Cell>> display(H,
      std::vector<Cell>(W, {'.', ansi::GREY, ""}));

  //  Background tiles
  for (size_t y = 0; y < H; ++y) {
    for (size_t x = 0; x < W; ++x) {
      char s = grid.GetSymbol(WorldPosition{x, y});
      if (s == '#' && y >= demo::kRiverY1 && y <= demo::kRiverY2) {
        // Red Cedar River 
        display[y][x] = {'~', ansi::BLUE, ansi::BG_BLUE};
      } else if (s == '#') {
        display[y][x] = {'#', ansi::GREY, ""};
      } else if (world.IsInQuarantine(WorldPosition{x, y})) {
        display[y][x] = {'~', ansi::YELLOW, ansi::BG_YELLOW};
      } else {
        display[y][x] = {'.', ansi::GREY, ""};
      }
    }
  }

  // Building names
  struct Label { size_t x, y; const char* text; };
  static constexpr Label kLabels[] = {

    { 3,  6, "WELLS HALL"},
    {18,  6, "MSU UNION"},
    {34,  6, "BERKEY"},
    {49,  6, "SHAW HALL"},
    {65,  6, "LIBRARY"},
    {81,  6, "NAT.SCI"},
  
    {20, 15, "RED CEDAR RIVER"},
   
    { 3, 23, "ENGINEERING"},
    {20, 22, "BRODY HALL"},
    {42, 24, "STADIUM"},
    {80, 23, "OLIN HEALTH"},
  };
  for (auto& lbl : kLabels) {
    for (size_t i = 0; lbl.text[i] != '\0' && lbl.x + i < W; ++i) {
      auto& cell = display[lbl.y][lbl.x + i];
      if (cell.sym == '.' || cell.sym == '~') {
        cell.sym = lbl.text[i];
        cell.fg  = ansi::CYAN;
        
      }
    }
  }

  // Agent overlay symbol 
  for (size_t id = 0; id < world.GetNumAgents(); ++id) {
    DiseaseData   st = world.GetAgentState(id);
    WorldPosition  p = st.position;
    bool is_resident = (id < demo::kNumPacers);

    const char* col = ansi::GREEN;
    if (st.health == HealthState::INFECTED)  col = ansi::RED;
    if (st.health == HealthState::RECOVERED) col = ansi::BLUE;

    char sym = is_resident ? 'P' : 'W';
    display[p.CellY()][p.CellX()] = {sym, col, ""};
  }

  // 3. Render
  std::cout << "\033[2J\033[H";
  std::cout << ansi::BOLD
            << "=== MSU Campus Infectious Disease Simulation (Group 13) ===\n"
            << ansi::RESET;
  std::cout << "  Tick: " << std::setw(4) << world.GetTickCount()
            << "   |   "
            << ansi::GREEN  << "Susceptible: " << std::setw(3)
            << world.GetSusceptibleCount() << ansi::RESET << "   "
            << ansi::RED    << "Infected: "    << std::setw(3)
            << world.GetInfectedCount()    << ansi::RESET << "   "
            << ansi::BLUE   << "Recovered: "   << std::setw(3)
            << world.GetRecoveredCount()   << ansi::RESET << "\n\n";

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
    << "  Agents:  "
    << ansi::GREEN  << "P" << ansi::RESET << " Scripted Agent "
    << ansi::CYAN   << "W" << ansi::RESET << " Swarming Agent\n"
    << "  Health:  "
    << ansi::GREEN  << "green"  << ansi::RESET << "=susceptible  "
    << ansi::RED    << "red"    << ansi::RESET << "=infected  "
    << ansi::BLUE   << "blue"   << ansi::RESET << "=recovered\n"
    << "  "
    << ansi::YELLOW << ansi::BG_YELLOW << "~" << ansi::RESET
    << ansi::YELLOW << " Olin Health (enter infected → recover → leave)" << ansi::RESET
    << "   agents: " << world.GetNumAgents()
    << " / " << demo::kMaxAgents << "\n";
}


// World layout helpers

/// Draw four walls of a rectangular building and cut door openings.
static void DrawBuilding(
    WorldGrid& grid, size_t wall_id, size_t floor_id,
    size_t x1, size_t y1, size_t x2, size_t y2,
    std::initializer_list<std::pair<size_t, size_t>> doors)
{
  for (size_t x = x1; x <= x2; ++x) {
    grid[x, y1] = wall_id;
    grid[x, y2] = wall_id;
  }
  for (size_t y = y1 + 1; y < y2; ++y) {
    grid[x1, y] = wall_id;
    grid[x2, y] = wall_id;
  }
  for (auto [dx, dy] : doors)
    grid[dx, dy] = floor_id;
}


// Campus setup 
int RunInfectiousDemo() {
  InfectiousWorld world(demo::kGridW, demo::kGridH);
  WorldGrid& grid  = world.GetGrid();
  size_t wall  = world.GetWallID();
  size_t floor = world.GetFloorID();

  // --- Outer border ---
  for (size_t y = 0; y < demo::kGridH; ++y) {
    grid[0,                y] = wall;
    grid[demo::kGridW - 1, y] = wall;
  }
  for (size_t x = 0; x < demo::kGridW; ++x) {
    grid[x, 0               ] = wall;
    grid[x, demo::kGridH - 1] = wall;
  }



  // Wells Hall 
  DrawBuilding(grid, wall, floor,  2,  2, 13, 11, {{7,11}, {13,6}});

  // MSU Union 
  DrawBuilding(grid, wall, floor, 17,  2, 29, 11, {{17,6}, {23,11}, {29,6}, {23,2}});

  // Berkey Hall
  DrawBuilding(grid, wall, floor, 33,  2, 42, 11, {{33,6}, {38,11}});

  // Shaw Hall 
  DrawBuilding(grid, wall, floor, 48,  2, 60, 11, {{48,6}, {54,11}});

  // Library  
  DrawBuilding(grid, wall, floor, 64,  2, 76, 11, {{64,6}, {70,11}, {76,6}});

  // Natural Science
  DrawBuilding(grid, wall, floor, 80,  2, 88, 11, {{80,6}, {84,11}});


  // RED CEDAR RIVER  
  for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
    for (size_t x = 1; x <= demo::kGridW - 2; ++x)
      grid[x, y] = wall;

  // Bridge 1 
  for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
    for (size_t x = 7; x <= 10; ++x)
      grid[x, y] = floor;

  // Bridge 2 
  for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
    for (size_t x = 43; x <= 46; ++x)
      grid[x, y] = floor;

  // Bridge 3 
  for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
    for (size_t x = 69; x <= 72; ++x)
      grid[x, y] = floor;



  //EB
  DrawBuilding(grid, wall, floor,  2, 19, 15, 28, {{9,19}, {15,24}, {9,28}});

  // Brody
  DrawBuilding(grid, wall, floor, 19, 19, 32, 27, {{26,19}, {19,23}});

  // Stadium
  DrawBuilding(grid, wall, floor, 36, 19, 57, 30, {{47,19}, {36,24}, {47,30}, {57,24}});



  // --- Quarantine zone = full Olin area ---
  world.AddQuarantineZone(
      Box::FromCorners(Point(demo::kQX1, demo::kQY1),
                       Point(demo::kQX2, demo::kQY2)));

  // --- Disease parameters ---
  world.SetTransmissionRate(0.50);
  world.SetInfectionRadius(2.5);
  world.SetTreatmentDuration(3);    // 3 ticks inside Olin = recovered
  world.SetImmunityDuration(50);    // 50 ticks immunity after recovery
  world.SetFallbackRecoveryTicks(100); // swarming agents auto-recover after 100 infection ticks

  // Clinic entrance: centre of Olin zone — infected agents pathfind here.
  world.SetClinicEntrance(WorldPosition{82, 24});

  world.SetRecoveryExit(WorldPosition{70, 17});

  // =========================================================================
  // AGENTS
  // =========================================================================

  // Passability predicate passed to every StepPacingAgent so BFS can
  // navigate around building walls toward Olin Health when infected.
  auto passable = [&world](WorldPosition p) {
    const auto& g = world.GetGrid();
    return g.IsValid(p) && g[p] != world.GetWallID();
  };

  // add one resident pacer and wire its BFS predicate in one call.
  auto add_pacer = [&](WorldPosition pos, bool horiz = false) -> StepPacingAgent& {
    auto& a = world.AddAgent<StepPacingAgent>(DiseaseData{pos});
    if (horiz) a.SetHorizontal();
    return a.SetPassable(passable);
  };

 
  add_pacer(WorldPosition{ 7,  6});           // Wells Hall
  add_pacer(WorldPosition{20,  7}, true);     // MSU Union 
  add_pacer(WorldPosition{37,  6});           // Berkey Hall
  add_pacer(WorldPosition{ 5, 24});           // Engineering Building
  add_pacer(WorldPosition{22, 23}, true);     // Brody Hall
  add_pacer(WorldPosition{52,  6});           // Shaw Hall dorm

  // Swarming agents


  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{14, 12}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{44, 12}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{68, 12}});


  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{16, 17}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{44, 17}});
  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{62, 17}});


  world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{47, 19}});  // Stadium

  // Patient zero — infected 
  world.InfectAgent(demo::kNumPacers);  


  // SIMULATION LOOP

  for (size_t tick = 0; tick < demo::kSimTicks; ++tick) {
    
    if (tick > 0 && tick % demo::kSpawnInterval == 0 &&
        world.GetNumAgents() < demo::kMaxAgents) {
      size_t src = (tick / demo::kSpawnInterval) % demo::kSpawnSources;
      world.AddAgent<SwarmingAgent<DiseaseData>>(
          DiseaseData{WorldPosition{demo::kSpawnX[src], demo::kSpawnY[src]}});
      world.InfectAgent(world.GetNumAgents() - 1);
    }

    DrawWorld(world);
    world.RunAgents();
    world.UpdateWorld();
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

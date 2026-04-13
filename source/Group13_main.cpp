/**
 * infectious_demo.cpp
 * @brief Demo of InfectiousWorld
 * @note Status: PROPOSAL
 * compile with:
 * g++ -std=c++23 -DCSE498_INFECTIOUS_DEMO_STANDALONE -I. \
 * Group13_main.cpp -o infectious_demo tools/Point.cpp tools/Box.cpp
 * run with:
 * ./infectious_demo
 **/

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "Agents/PacingAgent.hpp"
#include "Worlds/InfectiousWorld.hpp"

using namespace cse498;

namespace infectious_demo {
constexpr int kFrameDelayMs = 300;
constexpr std::size_t kSimulationTicks = 80;
}  // namespace infectious_demo

namespace ansi {
constexpr const char* CLEAR_SCREEN = "\033[2J\033[H";
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* RED = "\033[31m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* GREY = "\033[90m";
}  // namespace ansi

/// Draw the grid with agents color-coded by health state.
void DrawWorld(const InfectiousWorld& world) {
  const WorldGrid& grid = world.GetGrid();

  // Build a character grid from the world tiles.
  std::vector<std::string> display(grid.GetHeight());
  for (size_t y = 0; y < grid.GetHeight(); ++y) {
    display[y].resize(grid.GetWidth());
    for (size_t x = 0; x < grid.GetWidth(); ++x) {
      display[y][x] = grid.GetSymbol(WorldPosition{x, y});
    }
  }

  // Stamp agents onto the grid.
  for (size_t id = 0; id < world.GetNumAgents(); ++id) {
    const AgentBase& agent = world.GetAgent(id);
    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    char sym;
    switch (world.GetAgentHealth(id)) {
      case InfectiousWorld::HealthState::SUSCEPTIBLE:
        sym = 'S';
        break;
      case InfectiousWorld::HealthState::INFECTED:
        sym = 'I';
        break;
      case InfectiousWorld::HealthState::RECOVERED:
        sym = 'R';
        break;
    }
    display[pos.CellY()][pos.CellX()] = sym;
  }

  // Clear screen and print.
  std::cout << ansi::CLEAR_SCREEN;
  std::cout << "=== Infectious Disease Simulation ===\n";
  std::cout << "Tick: " << world.GetTickCount() << "\n\n";

  std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
  for (const auto& row : display) {
    std::cout << '|';
    for (char c : row) {
      switch (c) {
        case 'S':
          std::cout << ansi::GREEN << c << ansi::RESET;
          break;
        case 'I':
          std::cout << ansi::RED << c << ansi::RESET;
          break;
        case 'R':
          std::cout << ansi::BLUE << c << ansi::RESET;
          break;
        case '#':
          std::cout << ansi::GREY << c << ansi::RESET;
          break;
        default:
          std::cout << c;
      }
    }
    std::cout << "|\n";
  }
  std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";

  std::cout << "\n  " << ansi::GREEN << "S" << ansi::RESET << " = Susceptible  "
            << ansi::RED << "I" << ansi::RESET << " = Infected  " << ansi::BLUE
            << "R" << ansi::RESET << " = Recovered\n";
  std::cout << "  Susceptible: " << world.GetSusceptibleCount()
            << "  Infected: " << world.GetInfectedCount()
            << "  Recovered: " << world.GetRecoveredCount() << "\n";
}

/// Runs the infectious-world demo; safe to call from another translation unit.
int RunInfectiousDemo() {
  InfectiousWorld world(20, 10);

  WorldGrid& grid = world.GetGrid();
  // Add some walls
  for (size_t y = 0; y < 10; ++y) {
    grid[0, y] = world.GetWallID();
    grid[19, y] = world.GetWallID();
  }
  for (size_t x = 0; x < 20; ++x) {
    grid[x, 0] = world.GetWallID();
    grid[x, 9] = world.GetWallID();
  }
  // Internal wall
  // Internal wall
  for (size_t y = 1; y < 6; ++y) {
    grid[10, y] = world.GetWallID();
  }

  // Configure disease.
  world.SetTransmissionRate(0.4);
  world.SetInfectionRadius(1.5);
  world.SetInfectionDuration(8);
  world.SetImmunityDuration(15);

  // Add agents scattered across the grid.
  world.AddAgent<PacingAgent>("Agent-1").SetLocation(WorldPosition{3, 2});
  world.AddAgent<PacingAgent>("Agent-2").SetLocation(WorldPosition{5, 4});
  world.AddAgent<PacingAgent>("Agent-3").SetLocation(WorldPosition{7, 3});
  world.AddAgent<PacingAgent>("Agent-4").SetLocation(WorldPosition{4, 6});
  world.AddAgent<PacingAgent>("Agent-5").SetLocation(WorldPosition{8, 7});
  world.AddAgent<PacingAgent>("Agent-6").SetLocation(WorldPosition{12, 3});
  world.AddAgent<PacingAgent>("Agent-7").SetLocation(WorldPosition{15, 5});
  world.AddAgent<PacingAgent>("Agent-8").SetLocation(WorldPosition{14, 2});
  world.AddAgent<PacingAgent>("Agent-9").SetLocation(WorldPosition{16, 7});
  world.AddAgent<PacingAgent>("Agent-10").SetLocation(WorldPosition{13, 7});

  // Patient zero!
  world.InfectAgent(0);

  // Run simulation with visual output.
  for (size_t tick = 0; tick < infectious_demo::kSimulationTicks; ++tick) {
    DrawWorld(world);
    world.RunAgents();
    world.UpdateWorld();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(infectious_demo::kFrameDelayMs));
  }

  DrawWorld(world);
  std::cout << "\n=== Simulation Complete ===\n";

  return 0;
}

// Needed so as not to conflict with main in simple_main.cpp
#if defined(CSE498_INFECTIOUS_DEMO_STANDALONE)
int main() { return RunInfectiousDemo(); }
#endif

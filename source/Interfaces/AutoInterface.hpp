/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief An automated text interface that prints the world to the console
 * on a timed interval, with no player-controlled character.
 * @note Status: PROPOSAL
 * @note Written by Claude.
 **/

#pragma once

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../core/InterfaceBase.hpp"
#include "../core/WorldBase.hpp"

// clang-format off
namespace cse498 {

  class AutoInterface : public InterfaceBase {
  protected:
    using Clock = std::chrono::steady_clock;

    std::chrono::milliseconds frame_delay{200};  ///< Time between frames.
    Clock::time_point last_frame_time{};         ///< When we last drew.

    // -- Helper Functions --

    /// Draw the grid to the console, reusing TrashInterface's approach.
    void DrawGrid(const WorldGrid & grid,
                  const std::vector<size_t> & item_ids,
                  const std::vector<size_t> & agent_ids)
    {
      std::vector<std::string> symbol_grid(grid.GetHeight());

      // Load the world into the symbol_grid.
      for (size_t y = 0; y < grid.GetHeight(); ++y) {
        symbol_grid[y].resize(grid.GetWidth());
        for (size_t x = 0; x < grid.GetWidth(); ++x) {
          symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x, y});
        }
      }

      // Substitute in items.
      for (size_t id : item_ids) {
        const ItemBase & item = world.GetItem(id);
        WorldPosition pos = item.GetLocation().AsWorldPosition();
        symbol_grid[pos.CellY()][pos.CellX()] = '+';
      }

      // Substitute in agents (excluding ourselves, since we're not a "character").
      for (const auto & agent_id : agent_ids) {
        if (agent_id == GetID()) continue;
        const AgentBase & agent = world.GetAgent(agent_id);
        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        symbol_grid[pos.CellY()][pos.CellX()] = agent.GetSymbol();
      }

      // Clear screen with ANSI escape, then print the grid with a box around it.
      std::cout << "\033[2J\033[H";
      std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
      for (const auto & row : symbol_grid) {
        std::cout << '|';
        for (char cell : row) std::cout << cell;
        std::cout << "|\n";
      }
      std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
      std::cout.flush();
    }

  public:
    AutoInterface(size_t id, const std::string & name, const WorldBase & world)
      : InterfaceBase(id, name, world) { }
    ~AutoInterface() = default;

    /// Set the delay between frames.
    AutoInterface & SetFrameDelay(std::chrono::milliseconds delay) {
      frame_delay = delay;
      return *this;
    }

    // -- AgentBase overrides --

    bool Initialize() override {
      last_frame_time = Clock::now();
      return true;
    }

    size_t SelectAction(const WorldGrid & grid) override
    {
      // Sleep until it's time for the next frame.
      auto now = Clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_time);
      if (elapsed < frame_delay) {
        std::this_thread::sleep_for(frame_delay - elapsed);
      }
      last_frame_time = Clock::now();

      auto item_ids = world.GetKnownItems(*this);
      auto agent_ids = world.GetKnownAgents(*this);

      DrawGrid(grid, item_ids, agent_ids);

      // No action — this interface just observes.
      return 0;
    }
  };
// clang-format on
}  // End of namespace cse498

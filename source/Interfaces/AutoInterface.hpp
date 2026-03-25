/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief An automated text interface that prints the world to the console
 * on a timed interval, with no player-controlled character.
 * @note Status: TEMPORARY (will end up being discarded soon enough in favor of
 * the web and native GUIs). This is purely meant to help demo TrafficWorld and
 *won't necessarily work outside of that. This means you probably shouldn't
 *spend too much time reviewing it, as very little of it is likely to make it
 *into the end product.
 * @note Written by Claude. (Initially written as a modification of
 *TrashInterface that got rid of the controls and ran automatically; color stuff
 *was added later, also by Claude.)
 **/

#pragma once

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../Agents/DrivingAgent.hpp"
#include "../Worlds/TrafficWorld.hpp"
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
      const size_t W = grid.GetWidth();
      const size_t H = grid.GetHeight();

      std::vector<std::string> symbol_grid(H);
      // colour_grid stores the ANSI code for each cell, or "" for no colour.
      std::vector<std::vector<std::string>> colour_grid(
          H, std::vector<std::string>(W));

      // Load the world into the symbol_grid.
      for (size_t y = 0; y < H; ++y) {
        symbol_grid[y].resize(W);
        for (size_t x = 0; x < W; ++x) {
          symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x, y});
        }
      }

      // Colour destination tiles if the world is a TrafficWorld.
      const auto *traffic_world = dynamic_cast<const TrafficWorld *>(&world);
      if (traffic_world) {
        for (size_t y = 0; y < H; ++y) {
          for (size_t x = 0; x < W; ++x) {
            const std::string &col =
                traffic_world->GetDestinationColour(WorldPosition{x, y});
            if (!col.empty()) colour_grid[y][x] = col;
          }
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
        const auto *driver = dynamic_cast<const DrivingAgent *>(&agent);
        if (driver && !driver->GetColour().empty()) {
          colour_grid[pos.CellY()][pos.CellX()] = driver->GetColour();
        }
      }

      // Clear screen with ANSI escape, then print the grid with a box around it.
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

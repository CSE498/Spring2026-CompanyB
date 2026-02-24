/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A low-end text interface providing a bare-minimum level of interaction.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <iostream>
#include <vector>

#include "../core/InterfaceBase.hpp"
#include "../core/WorldBase.hpp"

namespace cse498 {

  class TrashInterface : public InterfaceBase {
  protected:
    bool wait_for_input = true;  ///< Should we pause and wait for the user to hit a key?

    // -- Helper Functions --
    void DrawGrid(const WorldGrid & grid,
                  const std::vector<size_t> & item_ids,
                  const std::vector<size_t> & agent_ids)
    {
      std::vector<std::string> symbol_grid(grid.GetHeight());

      // Load the world into the symbol_grid;
      for (size_t y=0; y < grid.GetHeight(); ++y) {
        symbol_grid[y].resize(grid.GetWidth());
        for (size_t x=0; x < grid.GetWidth(); ++x) {
          symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x,y});
        }
      }

      // Substitute in items.
      for (size_t id : item_ids) {
        const ItemBase & item = world.GetItem(id);
        WorldPosition pos = item.GetLocation().AsWorldPosition();
        symbol_grid[pos.CellY()][pos.CellX()] = '+';
      }

      // Substitute in agents.
      for (const auto & agent_id : agent_ids) {
        const AgentBase & agent = world.GetAgent(agent_id);
        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        symbol_grid[pos.CellY()][pos.CellX()] = agent.GetSymbol();
      }

      // Print out the symbol_grid with a box around it.
      std::cout << '+' << std::string(grid.GetWidth(),'-') << "+\n";
      for (const auto & row : symbol_grid) {
        std::cout << "|";
        for (char cell : row) std::cout << cell;
        std::cout << "|\n";
      }
      std::cout << '+' << std::string(grid.GetWidth(),'-') << "+\n";
      std::cout << "\nUse W, A, S, D to move or Q to quit.";
      std::cout << "\nYour move? ";
      std::cout.flush();
    }

  public:
    TrashInterface(size_t id, const std::string & name, const WorldBase & world)
      : InterfaceBase(id, name, world) { }
    ~TrashInterface() = default; // Already virtual from Entity

    // -- AgentBase overrides --

    bool Initialize() override {
      return true;
    }

    size_t SelectAction(const WorldGrid & grid) override
    {
      auto item_ids = world.GetKnownItems(*this);
      auto agent_ids = world.GetKnownAgents(*this);

      // Update the current state of the world.
      DrawGrid(grid, item_ids, agent_ids);

      // See if there are any keys waiting in standard input (wait if needed)
      char input;
      do {
        std::cin >> input;
      } while (!std::cin && wait_for_input);

      // Respond to the user input...
      size_t action_id = 0;
      switch (input) {
        case 'w': case 'W': action_id = GetActionID("up");    break;
        case 'a': case 'A': action_id = GetActionID("left");  break;
        case 's': case 'S': action_id = GetActionID("down");  break;
        case 'd': case 'D': action_id = GetActionID("right"); break;
        case 'q': case 'Q': exit(0); // Quit!
      }

      // If we waited for input, but don't understand it, notify the user.
      if (wait_for_input && action_id == 0) {
        std::cout << "Unknown key '" << input << "'." << std::endl;
      }

      // Do the action!
      return action_id;
    }
  };

} // End of namespace cse498

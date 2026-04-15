/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class for all World modules.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "AgentBase.hpp"
#include "ItemBase.hpp"
#include "Step.hpp"
#include "WorldGrid.hpp"
#include "AgentData.hpp"

namespace cse498 {

using item_ptr_t = std::unique_ptr<ItemBase>;
using item_set_t = std::vector<item_ptr_t>;
using cse498::steps::StepContainer;

template <typename T>
concept IsWorldData = Concepts::IsOneOf<T, TrafficData, InfectionData>;

template <IsWorldData WorldData>
class WorldBase {

  using agent_ptr_t = std::unique_ptr<AgentBase<WorldData>>;
  using agent_set_t = std::vector<agent_ptr_t>;

protected:
  /// NOTE: derived worlds may choose to have more than one grid.
  WorldGrid main_grid; ///< Main grid for this world

  item_set_t item_set; ///< Vector of pointers to non-agent entities (ItemBase)
  agent_set_t agent_set; ///< Vector of pointers to agent entities (AgentBase)

  bool run_over = false; ///< Are we finished executing and now shutting down?

  /// Helper function that is run whenever a new agent is created.
  /// @note Override this function to provide agents with actions or other
  /// setup.
  virtual void ConfigAgent(AgentBase<WorldData> & /* agent */) {}

public:
  WorldBase() = default;
  virtual ~WorldBase() = default;

  // -- Accessors --

  /// Get the total number of NON-agent entities
  [[nodiscard]] size_t GetNumItems() const { return item_set.size(); }

  /// Get the total number of AGENT entities
  [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }

  /// Return a reference to an Item with a given ID.
  [[nodiscard]] ItemBase &GetItem(size_t id) {
    assert(id < item_set.size());
    return *item_set[id];
  }

  /// Return a CONST reference to an Item with a given ID.
  [[nodiscard]] const ItemBase &GetItem(size_t id) const {
    assert(id < item_set.size());
    return *item_set[id];
  }

  /// Return a reference to an Agent with a given ID.
  [[nodiscard]] AgentBase<WorldData> &GetAgent(size_t id) {
    assert(id < agent_set.size());
    return *agent_set[id];
  }

  /// Return a CONST reference to an Agent with a given ID.
  [[nodiscard]] const AgentBase<WorldData> &GetAgent(size_t id) const {
    assert(id < agent_set.size());
    return *agent_set[id];
  }

  /// Return an editable version of the current grid for this world (main_grid
  /// by default)
  virtual WorldGrid &GetGrid() { return main_grid; }

  /// Return the current grid for this world (main_grid by default)
  virtual const WorldGrid &GetGrid() const { return main_grid; }

  /// Determine if the run has ended.
  virtual bool IsRunOver() const { return run_over; }

  // -- Agent Management --

  /// @brief Build a new agent of the specified type
  /// @param WorldData The data of this agent
  /// @return A reference to the newly created agent
  template <typename AGENT_T> AGENT_T &AddAgent(WorldData data) {
    auto agent_ptr = std::make_unique<AGENT_T>(data);
    AGENT_T &agent_ref = *agent_ptr;
    ConfigAgent(*agent_ptr);
    // if (agent_ptr->Initialize() == false) {
    //   std::cerr << "Failed to initialize agent '" << agent_name << "'."
    //             << std::endl;
    // }
    agent_set.emplace_back(
        std::move(agent_ptr)); // Move unique ptr for agent into set.
    return agent_ref;
  }

  // -- Action Management --

  /// @brief Central function for an agent to take any action
  /// @param agent The specific agent taking the action
  /// @param action The id of the action to take
  /// @return The result of this action (usually 0/1 to indicate success)
  /// @note This function must be overridden in any derived world.
  virtual void DoAction(AgentBase<WorldData> &agent) = 0;

  /// @brief Step through each agent giving them a chance to take an action.
  /// @note Override function to control execution order of agents.
  /// @note Override function to control which grid each agent receives.
  virtual void RunAgents() {
    for (const agent_ptr_t &agent_ptr : agent_set) {
      DoAction(*agent_ptr);
      // agent_ptr->SetState(new_data);
    }
  }

  /// @brief UpdateWorld() is run after every agent has a turn.
  /// Override this function to manage background events for a world.
  /// (E.g., weather, growth, regular physics, etc.)
  virtual void UpdateWorld() {}

  /// @brief Run all agents repeatedly until an end condition is met.
  virtual void Run() {
    run_over = false;
    while (!run_over) {
      RunAgents();
      UpdateWorld();

      /**
       * Since we only do simulation, and no user input, i moved the interface
       * display stuff here This more closely mimicks the compy design
       */
      DrawGrid(main_grid);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(750)); // Simulate a SLOW tick
    }
  }

  /// @brief Draws the 2D world grid
  /// @param grid The grid to draw
  void DrawGrid(const WorldGrid &grid) {
    std::vector<std::string> symbol_grid(grid.GetHeight());

    // Load the world into the symbol_grid;
    for (size_t y = 0; y < grid.GetHeight(); ++y) {
      symbol_grid[y].resize(grid.GetWidth());
      for (size_t x = 0; x < grid.GetWidth(); ++x) {
        symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x, y});
      }
    }

    // Substitute in items.
    for (auto &item : item_set) {
      WorldPosition pos = item->GetLocation().AsWorldPosition();
      symbol_grid[pos.CellY()][pos.CellX()] = '+';
    }

    // Substitute in agents.
    for (auto &agent : agent_set) {
      WorldPosition pos = agent->GetState().pos;
      symbol_grid[pos.CellY()][pos.CellX()] = agent->GetState().symbol;
    }

    // Print out the symbol_grid with a box around it.
    std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
    for (const auto &row : symbol_grid) {
      std::cout << "|";
      for (char cell : row)
        std::cout << cell;
      std::cout << "|\n";
    }
    std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
    std::cout.flush();
  }

  //////////////////////////////////////////////////////////////////////////
  //
  //  World API -- the member functions below are intended to be called
  //  from Agents to get more information about their senses and options.
  //
  //////////////////////////////////////////////////////////////////////////

  // Provide a vector of IDs for other agents that the input agent is aware of.
  // (If not overridden, return ALL agents.)
  // virtual std::vector<size_t> GetKnownAgents(
  //     [[maybe_unused]] const AgentBase<WorldData>& agent) const {
  //   std::vector<size_t> out_ids;
  //   for (const agent_ptr_t& ptr : agent_set) out_ids.push_back(ptr->GetID());
  //   return out_ids;
  // }

  // // Provide a vector of IDs for items that the input agent is aware of.
  // // (If not overridden, return ALL items.)
  // std::vector<size_t> GetKnownItems(
  //     [[maybe_unused]] const AgentBase<WorldData>& agent) const {
  //   std::vector<size_t> out_ids;
  //   for (const item_ptr_t& ptr : item_set) out_ids.push_back(ptr->GetID());
  //   return out_ids;
  // }
};

} // End of namespace cse498

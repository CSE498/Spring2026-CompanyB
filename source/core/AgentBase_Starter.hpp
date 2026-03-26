/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all agent types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

#include "Entity.hpp"
#include "ItemBase.hpp"
#include "Location.hpp"
#include "WorldGrid.hpp"

namespace cse498 {

  class AgentBase : public Entity {
  protected:
    /// A map of names to IDs for each available action
    std::unordered_map<std::string, size_t> action_map;

    /// Action results are a 0 for failure); Success is any non-zero value,
    /// which may provide more info about the type of success.
    int action_result = 1;

    char symbol = '*';

  public:
    AgentBase(size_t id, const std::string & name, const WorldBase & world)
      : Entity(id, name, world) { }
    ~AgentBase() = default; // Already virtual from Entity

    // Accessors

    [[nodiscard]] char GetSymbol() const { return symbol; }
    AgentBase & SetSymbol(char in) { symbol = in; return *this; }

    // -- World Interactions --

    /// @brief Run AFTER the world configures the agent, for additional tests or setup.
    /// @return Was the initialization successful?
    virtual bool Initialize() { return true; }

    // -- Entity Overrides --
    bool IsAgent() const override { return true; }


    // -- Action management --

    /// Test if agent already has a specified action.
    [[nodiscard]] bool HasAction(const std::string & action_name) const {
      return action_map.count(action_name);
    }

    /// Return an action ID *if* that action exists, otherwise return zero.
    [[nodiscard]] size_t GetActionID(const std::string & action_name) const {
      auto it = action_map.find(action_name);
      if (it == action_map.end()) return 0;
      return it->second;
    }

    /// Retrieve the result of the most recent action.
    [[nodiscard]] int GetActionResult() const { return action_result; }

    //////////////////////////////////////////////////////////////////////////
    //
    //  Agent API -- the member functions below are intended to be called
    //  from a World to:
    //   * provide options of available actions
    //   * request an agent to select their next action
    //   * notify an agent about the result of their latest action
    //   * provide any additional notifications to an agent
    //
    //////////////////////////////////////////////////////////////////////////

    /// Provide info about an action that this agent can take.
    virtual AgentBase & AddAction(const std::string & action_name, size_t action_id) {
      assert(!HasAction(action_name)); // Cannot add existing action name.
      action_map[action_name] = action_id;
      return *this;
    }

    /// @brief Decide the next action for this agent to perform; must be overridden!
    /// @param grid The current known portions of the WorldGrid
    /// @return ID of the action to perform; (0 is always "no action")
    /// @note Agents can use World API to query for more info (e.g., items, agents, or cell info)
    [[nodiscard]] virtual size_t SelectAction(const WorldGrid & grid) = 0;


    /// Provide the result of this agent's most recent action.
    void SetActionResult(int result) { action_result = result; }


    /// @brief Send a notification to this agent
    /// @param message Contents of the notification
    /// @param msg_type Category of message, such as "item_alert", "damage", or "enemy"
    /// @note: For DEVELOPERS - you may want more info provided with notifications.
    virtual void Notify(const std::string & /*message*/,
                        const std::string & /*msg_type*/="none") { }
  };

} // End of namespace cse498

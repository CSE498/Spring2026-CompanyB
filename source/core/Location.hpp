/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A specification of where something is (on a grid, in a chest, on an agent, etc)
 * @note Status: PROPOSAL
 * 
 * Depending on how a system is made, there are many possible locations an object can have,
 * particularly an item. The Location class handles all types of locations; for example, an item
 * can be sitting out on a grid (thus needing a grid location), it can be in the possession of
 * of an agent (thus needing the agent ID), or it can be inside of another item (thus needing
 * the item number.)
 **/

#pragma once

#include <cassert>   // For assert
#include <cstddef>   // For size_t
#include <variant>

#include "WorldPosition.hpp"

namespace cse498 {

  struct ItemID { size_t id; };
  struct AgentID { size_t id; };

  class Location {
  private:
    std::variant<WorldPosition, ItemID, AgentID> location;

  public:
    Location() = default;
    Location(const Location &) = default;
    Location(const WorldPosition & pos) : location(pos) { }
    Location(ItemID id) : location(id) { }
    Location(AgentID id) : location(id) { }

    Location & operator=(const Location &) = default;

    [[nodiscard]] bool IsPosition() const { return std::holds_alternative<WorldPosition>(location); }
    [[nodiscard]] bool IsAgentID() const { return std::holds_alternative<AgentID>(location); }
    [[nodiscard]] bool IsItemID() const { return std::holds_alternative<ItemID>(location); }

    [[nodiscard]] const WorldPosition & AsWorldPosition() const {
      assert (IsPosition());
      return std::get<WorldPosition>(location);
    }
    [[nodiscard]] size_t AsAgentID() const {
      assert(IsAgentID());
      return std::get<AgentID>(location).id;
    }
    [[nodiscard]] size_t AsItemID() const {
      assert(IsItemID());
      return std::get<ItemID>(location).id;
    }
  };

} // End of namespace cse498

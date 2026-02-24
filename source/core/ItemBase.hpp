/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all non-agent item types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <string>

#include "Entity.hpp"
#include "Location.hpp"

namespace cse498 {

  class ItemBase : public Entity {
  protected:
    // Any traits that ITEMS have that agents should not go here.
    // (All Entities have a unique ID, a name, a location, and a reference to their world.)

  public:
    ItemBase(size_t id, const std::string & name, const WorldBase & world)
      : Entity(id, name, world) { }
    ~ItemBase() override = default; // Already virtual from Entity

    // -- Entity Overrides --

    bool IsItem() const override { return true; }

    // -- Ownership Info --
    [[nodiscard]] bool IsOwned() const { return location.IsAgentID(); }
    [[nodiscard]] size_t GetOwnerID() const {
      assert(IsOwned());
      return location.AsAgentID();
    }

    // Set owner ID can be either a number or an AgentID
    ItemBase & SetOwnerID(AgentID id) { location = id; return *this; }
    ItemBase & SetOwnerID(size_t id) { return SetOwnerID(AgentID{id}); }
  };

} // End of namespace cse498

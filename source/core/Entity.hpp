/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class for all items OR agents that can exist in the world.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>        // For assert
#include <cstddef>        // For size_t
#include <string>

#include "Location.hpp"

namespace cse498 {

  class WorldBase; // A pre-declaration of WorldBase so that it can be referred to below.

  class Entity {
  protected:
    const size_t id;         ///< Unique ID for this entity; should never change.
    std::string name;        ///< Name for this entity (E.g., "Player 1" or "+2 Sword")
    Location location;       ///< Where is this entity located?
    const WorldBase & world; ///< Which world does this entity belong to?

    // @TODO: entities should probably have annotations or properties...
    //        Any public functions here for properties could be useful
    //        in agents, items, or interfaces.

  public:
    Entity(size_t id, const std::string & name, const WorldBase & world)
      : id(id), name(name), world(world) { }
    Entity(const Entity &) = delete; // Entities must be unique and shouldn't be copied.
    Entity(Entity &&) = default;     // Entities CAN be moved.
    virtual ~Entity() = default;     // Destructor must be virtual for derived class.

    Entity & operator=(const Entity &) = delete; // Entities must be unique and shouldn't be copied.

    // -- Accessors --
    [[nodiscard]] size_t GetID() const { return id; }
    [[nodiscard]] const std::string & GetName() const { return name; }
    [[nodiscard]] const Location & GetLocation() const { return location; }

    Entity & SetName(const std::string & in_name) { name = in_name; return *this; }
    Entity & SetLocation(Location in) { location = in; return *this; }

    /// Is this Entity actually an autonomous agent? (Overridden in AgentBase to return true)
    virtual bool IsAgent() const { return false; }

    /// Is this Entity actually an item? (Overridden in ItemBase to return true)
    virtual bool IsItem() const { return false; }

    /// Is this Entity actually a specialty Agent that's an Interface for a human player?
    /// (Overridden in InterfaceBase to return true)
    virtual bool IsInterface() const { return false; }
  };

} // End of namespace cse498

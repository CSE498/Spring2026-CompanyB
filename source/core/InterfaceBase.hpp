/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class for all .
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>

#include "AgentBase.hpp"

namespace cse498 {

  // NOTE: InterfaceBase derives from AgentBase, but this is only needed for software
  // where the user is a "player" with the same abilities as an agent and needs to be
  // tracked by the World.  Some software types won't need this style of interface.
  
  class InterfaceBase : public AgentBase {
  protected:

  public:
    InterfaceBase(size_t id, const std::string & name, const WorldBase & world)
      : AgentBase(id, name, world) { }
    ~InterfaceBase() = default; // Already virtual from Entity

    // -- Entity Overrides --

    bool IsInterface() const override { return true; }


    // -- AgentBase overrides --
    //
    // Multiple functions defined as virtual in AgentBase can be overridden from
    // custom Interfaces.  Specific available function to override include:

    // bool Initialize() { return true; }
    //
    // Initialize is run AFTER the world configures the interface with actions, etc, allowing
    // for additional setup, such as targeted buttons.


    // size_t SelectAction([[maybe_unused]] const WorldGrid & grid) { return 0; }
    //
    // SelectAction is run when the world is given the player an opportunity to choose
    // and action to perform. The interface will be provided with the current WorldGrid
    // Return the ID associated with the action to perform; (zero is always "no action")
  };

} // End of namespace cse498

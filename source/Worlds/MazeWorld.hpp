/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "../core/WorldBase.hpp"
#include "Step.hpp"

// clang-format off
namespace cse498 {

  class MazeWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id;  ///< Easy access to wall CellType ID.

    // /// Provide the agent with movement actions.
    // void ConfigAgent(AgentBase & agent) override {
    //   agent.AddAction("up", MOVE_UP);
    //   agent.AddAction("down", MOVE_DOWN);
    //   agent.AddAction("left", MOVE_LEFT);
    //   agent.AddAction("right", MOVE_RIGHT);
    // }

  public:
    MazeWorld() {
      floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
      wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');

      main_grid.Load(std::vector<std::string>{"#######################",
                                              "# #            ##     #",
                                              "# #  #  ######    ### #",
                                              "# #  #  #     #  #  # #",
                                              "# #  #  #  #  #  #  # #",
                                              "#    #     #     #    #",
                                              "##################  # #",
                                              "#                    ##",
                                              "#                    ##",
                                              "#  ####################",
                                              "#######################"} );
    }
    ~MazeWorld() = default;

    // We'll want to be able to represent success/failure when executing the
    // steps, so we define an error object for std::expected
    struct WorldErr {
      enum class Kind {
	LOC_INVALID, // Location invalid
      };

      Kind kind; // Enum representing type of error
      std::string why; // Extra text to further explain the error (if req)
    };

    // This will act as the functor input into std::visit
    struct StepVisitor {
      // We do want to represent failure, but don't need to represent
      // any output, so we'll define & alias our return as such:
      using VisitRet = std::expected<void, WorldErr>;

      // We'll want to modify the agent and the container, so we'll hold on to
      // some references for them. You'll have to do the same for any other
      // non-variant external context desired.
      AgentBase<DummyAgentData> &agent;
      StepContainer &container;

      // Now we'll need to have an operator() overload for each Step type.
      VisitRet operator()(steps::MovementStep step) {
	// The simplest step -- the agent just wants to move to a space.
	// FILL IN: Implement logic actually changing agent's position.
	return {};
      }

      VisitRet operator()(steps::InfoStep step) {
	// When we get an info step, the agent would like to do something
	// conditionally (the container handles the branching internally).
	// It will provide us with the "aspect" about the world, and the
	// location it wants the info about. In return, we'll "inform" the
	// container with what has been requested.
	switch (step.aspect) {
	using Aspect = cse498::steps::InfoStep::Aspect;
	case Aspect::LOC_AVAIL: {
	  // Is the location available? We'll inform the world with a bool
	  // FILL IN: Implement logic actually generating this value
	  container.inform(false);
	  break;
	}
	case Aspect::OCCUPANCY_FRAC: {
	  // What fraction of the location referred to is occupied? We'll
	  // inform the world with a double in [0, 1]
	  // FILL IN: Implement logic actually generating this value
	  container.inform(0.5);
	  break;
	}
	case Aspect::OCCUPANCY_RAW: {
	  // How many agents are in/at the location referred to? We'll inform
	  // the world with a integer
	  container.inform(5);
	  break;
	}
	};
	return {};
      }

      VisitRet operator()(steps::ConditionalStep step) {
	// The compiler needs this, but this should never be reached. A
	// better solution is forthcoming, for now just leave empty.
	return {};
      }

      VisitRet operator()(steps::ReconStep step) {
	// The compiler needs this, but this should (for now) never be
	// reached. A better solution is forthcoming, for now just leave empty.
	return {};
      }
    };

    /// Allow the agents to move around the maze.
    DummyAgentData DoAction(AgentBase<DummyAgentData>& agent, [[maybe_unused]] const StepContainer& steps) override {
      // Determine where the agent is trying to move.
      WorldPosition cur_position = agent.GetState().pos;
      WorldPosition new_position;
      
      using namespace cse498::steps;

      // Get the turn from the agent
      StepContainer agent_turn = agent.GetTurn();

      // The functor we'll dispatch our step calls out to doesn't change with
      // each step, so we can instantiate it here outside the loop. To be as
      // clear as possible, this object is to act SOLELY as a collection of
      // function calls, at times with additional context (here the agent and
      // step container, so that the function calls can update agent state and
      // update the stepcontainer for .inform() ).
      StepVisitor step_visitor(agent, agent_turn);

      while (!agent_turn.empty()) {
	std::expected<Step, StepErr> cur_step = agent_turn.get_next();
	if (!cur_step.has_value()) {
	  // Here we'll want to have a way to track and later inform the agent
	  // that an error occured. For the sake of this demo, we'll just
	  // assume that an unexpected halts an agent's turn.
	  break;
	}

	// This call will dispatch a call out to our prior functor, calling the
	// correct overload (   all w/o a vtable (;   ).
	StepVisitor::VisitRet step_res = std::visit(step_visitor, cur_step.value());
	if (!step_res.has_value()) {
	  // Here we'll want to handle an error within the world. I don't know
	  // what that would look like for this demo so again we'll just halt.
	  break;
	}

      }


      

      // Don't let the agent move off the world or into a wall.
      // if (!main_grid.IsValid(new_position)) { return DummyAgentData(cur_position); }
      // if (main_grid[new_position] == wall_id) { return DummyAgentData(cur_position); }

      // // Set the agent to its new postion.
      return DummyAgentData(new_position);
    }

  };
// clang-format on
} // End of namespace cse498

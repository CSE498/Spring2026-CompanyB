/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <thread>

#include "Step.hpp"
#include "core/AgentData.hpp"
#include "core/StepWorldBase.hpp"

// clang-format off
namespace cse498 {

  class ScriptingTestWorld : public StepWorldBase<TrafficData> {
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
    ScriptingTestWorld() {
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
      ~ScriptingTestWorld() = default;
      
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
        TrafficData &data;
        StepContainer &container;
        
        // Now we'll need to have an operator() overload for each Step type.
        VisitRet operator()(steps::MovementStep step) {
          // The simplest step -- the agent just wants to move to a space.
          // FILL IN: Implement logic actually changing agent's position.
          // data = TrafficData{};
          data.position = step.loc;
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
        
        VisitRet operator()([[maybe_unused]] steps::ConditionalStep step) {
          // The compiler needs this, but this should never be reached. A
          // better solution is forthcoming, for now just leave empty.
          return {};
        }
        
        VisitRet operator()([[maybe_unused]] steps::ReconStep step) {
          // The compiler needs this, but this should (for now) never be
          // reached. A better solution is forthcoming, for now just leave empty.
          return {};
        }
      };
      
      /// Allow the agents to move around the maze.
      TrafficData DoAction(std::shared_ptr<StepAgentBase<TrafficData>> agent) override {
        // Determine where the agent is trying to move.
        using namespace cse498::steps;
        
        // Get the turn from the agent
        StepContainer agent_turn = agent->GetTurn();
        TrafficData data = agent->GetState();

        // The functor we'll dispatch our step calls out to doesn't change with
        // each step, so we can instantiate it here outside the loop. To be as
        // clear as possible, this object is to act SOLELY as a collection of
        // function calls, at times with additional context (here the agent and
        // step container, so that the function calls can update agent state and
        // update the stepcontainer for .inform() ).
        StepVisitor step_visitor(data, agent_turn);

        while (!agent_turn.exhausted()) {
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
          break;
        }
        
       return data;
        
        
        // Don't let the agent move off the world or into a wall.
        // if (!main_grid.IsValid(new_position)) { return DummyAgentData(cur_position); }
        // if (main_grid[new_position] == wall_id) { return DummyAgentData(cur_position); }
        
      }
      

    void UpdateWorld() override {
        DrawGrid(main_grid);
        std::this_thread::sleep_for(
          std::chrono::milliseconds(750));  // Simulate a SLOW tick
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

        // Substitute in agents.
        for (auto &agent : agent_set) {
        WorldPosition pos = agent->GetState().position;
        symbol_grid[pos.CellY()][pos.CellX()] = agent->GetState().symbol;
        }

        // Print out the symbol_grid with a box around it.
        std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
        for (const auto &row : symbol_grid) {
        std::cout << "|";
        for (char cell : row) std::cout << cell;
        std::cout << "|\n";
        }
        std::cout << '+' << std::string(grid.GetWidth(), '-') << "+\n";
        std::cout.flush();
    }
    };
// clang-format on
}  // End of namespace cse498

/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../core/StepWorldBase.hpp"
#include "../core/Step.hpp"
#include "../core/AgentData.hpp"

// clang-format off
namespace cse498 {

  class MazeWorld : public StepWorldBase<cse498::TrafficData> {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id;  ///< Easy access to wall CellType ID.

    // /// Provide the agent with movement actions.
    // void ConfigAgent(StepAgentBase<TrafficData> & agent) override {
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

        // Hold references to the world, the working state (which we mutate
        // in place and the caller returns), and the container (for .inform()).
        MazeWorld &world;
        TrafficData &state;
        StepContainer &container;

        // Now we'll need to have an operator() overload for each Step type.
        VisitRet operator()(steps::MovementStep step) {
          if (!world.main_grid.IsValid(step.loc)) {
            return {};
          }

          if (world.main_grid[step.loc] == world.wall_id) {
            return {};
          }

          state.pos = step.loc;
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

              //container.inform(false);
              bool available = world.main_grid.IsValid(step.target) && (world.main_grid[step.target] != world.wall_id);
              
              container.inform(available);

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
        using namespace cse498::steps;

        // Start from the agent's current state and mutate a local copy; the
        // base class will call SetState() with our return value.
        TrafficData new_state = agent->GetState();
        StepContainer agent_turn = agent->GetTurn();

        StepVisitor step_visitor{*this, new_state, agent_turn};

        while (!agent_turn.exhausted()) {
          std::expected<Step, StepErr> cur_step = agent_turn.get_next();
          if (!cur_step.has_value()) {
            break;
          }

          StepVisitor::VisitRet step_res = std::visit(step_visitor, cur_step.value());
          if (!step_res.has_value()) {
            break;
          }
        }

        return new_state;
      }

      // ==================================================================
      //  Terminal display — adapted from Group 19's StepTrafficWorld.
      //  StepWorldBase has no interface-agent concept, so the display is
      //  integrated into the derived world's run loop instead.
      // ==================================================================

      using Clock = std::chrono::steady_clock;

      std::chrono::milliseconds frame_delay{200};
      Clock::time_point last_frame_time{};

      void DrawGrid() {
        const size_t W = main_grid.GetWidth();
        const size_t H = main_grid.GetHeight();

        std::vector<std::string> symbol_grid(H);
        for (size_t y = 0; y < H; ++y) {
          symbol_grid[y].resize(W);
          for (size_t x = 0; x < W; ++x) {
            symbol_grid[y][x] = main_grid.GetSymbol(WorldPosition{x, y});
          }
        }

        for (const auto &agent_ptr : agent_set) {
          const TrafficData &state = agent_ptr->GetState();
          const size_t ax = state.pos.CellX();
          const size_t ay = state.pos.CellY();
          if (ax >= W || ay >= H) continue;
          symbol_grid[ay][ax] = state.symbol;
        }

        std::cout << "\033[2J\033[H";
        std::cout << '+' << std::string(W, '-') << "+\n";
        for (const auto &row : symbol_grid) {
          std::cout << '|' << row << "|\n";
        }
        std::cout << '+' << std::string(W, '-') << "+\n";
        std::cout.flush();
      }

      MazeWorld &SetFrameDelay(std::chrono::milliseconds delay) {
        frame_delay = delay;
        return *this;
      }

      void RunWithDisplay() {
        run_over = false;
        last_frame_time = Clock::now();
        while (!run_over) {
          auto now = Clock::now();
          auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
              now - last_frame_time);
          if (elapsed < frame_delay) {
            std::this_thread::sleep_for(frame_delay - elapsed);
          }
          last_frame_time = Clock::now();

          DrawGrid();
          RunAgents();
          UpdateWorld();
        }
      }
    };
    // clang-format on
  }  // End of namespace cse498
  
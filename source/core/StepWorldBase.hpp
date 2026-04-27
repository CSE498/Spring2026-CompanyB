#pragma once

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include "Agents/ScriptedAgent.hpp"
#include "Interpreter/Parser.hpp"
#include "Step.hpp"
#include "StepAgentBase.hpp"
#include "WorldGrid.hpp"
#include "core.hpp"

namespace cse498 {

using namespace cse498::steps;
using Concepts::IsDataClass;

template <IsDataClass DataClass>
class StepWorldBase {
  using Agent = StepAgentBase<DataClass>;
  using AgentPtr = std::shared_ptr<Agent>;

  /// Id of the next created agent
  size_t next_id = 0;

  /// Parser for scripted agent
  Parser parser{};

 protected:
  /// Whether the simulation is running
  bool run_over = false;
  /// Collection of agents
  std::vector<AgentPtr> agent_set;
  /// Main grid for this world
  WorldGrid main_grid;

  /// Helper function that is run whenever a new agent is created.
  /// @note Override this function to provide agents with actions or other
  /// setup.
  virtual void ConfigAgent([[maybe_unused]] Agent& agent) {}

  std::string GetAgentFileContents(std::string const& path) {
    std::ifstream file(path);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  /** @brief Set up the scripted agents by parsing the intended script and
   * initializing the agents.
   * @param input Input script to interpret. If left empty, the file 'test.al'
   * is read as input. If supplied, the given string will be treated as the
   * input script itself.
   */
  void SetupScriptedAgents(std::string const& input = "") {
    std::string contents =
        (input.empty()) ? GetAgentFileContents("test.al") : input;
    std::stringstream ss{contents};
    std::cout << contents;

    auto parse_res = parser.parse(ss);
    if (!parse_res.has_value()) {
      std::cout << parse_res.error().ToStr() << "\n";
      // TODO LOG?
      // std::terminate();
      return;
    }
    for (auto& agent_def : parse_res.value()) {
      DataClass data;
      this->AddAgent<ScriptedAgent<DataClass>>(data)
          .SetInit(std::move(agent_def->m_Init))
          .SetTurn(std::move(agent_def->m_Turn));
    }
  }

 public:
  StepWorldBase() = default;
  virtual ~StepWorldBase() = default;

  ///@ brief Add an agent to the world
  ///@param data DataClass to add to agent
  ///@note This signature or logic is may not final, since other logic may need
  /// to be added to suport different agents, such as scripting agents.
  template <typename AgentDerived>
  AgentDerived& AddAgent(DataClass data, LogLevel loglevel = LogLevel::Normal,
                         uint64_t tick = 0) {
    auto agent_ptr =
        std::make_shared<AgentDerived>(data, next_id++, loglevel, tick);
    AgentDerived& agent_ref = *agent_ptr;
    ConfigAgent(agent_ref);
    agent_set.emplace_back(std::move(agent_ptr));
    return agent_ref;
  }

  /// @brief Central function for an agent to take any action
  /// @param agent The specific agent taking the action
  /// @return The step taken by this agent after the action
  /// @note This function must be overridden in any derived world.
  /// @note Do NOT call Agent.SetState() in this function, instead return
  /// the resulting state
  virtual DataClass DoAction(AgentPtr agent) = 0;

  /// @brief Step through each agent giving them a chance to take an action.kkk
  /// @note Override function to control execution order of agents.
  /// @note Override function to control which grid each agent receives.
  virtual void RunAgents(LogLevel loglevel = LogLevel::Normal,
                         uint64_t tick = 0) {
    for (const auto& agent_ptr : agent_set) {
      DataClass new_state = DoAction(agent_ptr);
      agent_ptr->SetState(new_state, loglevel, tick);
    }
  }

  /// @brief UpdateWorld() is run after every agent has a turn.
  /// Override this function to manage background events for a world.
  /// (E.g., weather, growth, regular physics, etc.)
  virtual void UpdateWorld() {}

  /// @brief Run all agents repeatedly until an end condition is met.
  virtual void Run() {
    SetupScriptedAgents();
    run_over = false;
    while (!run_over) {
      RunAgents();
      UpdateWorld();
    }
  }
};

}  // namespace cse498

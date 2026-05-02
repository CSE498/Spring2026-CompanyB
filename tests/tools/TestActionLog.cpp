#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include <vector>

#include "../../source/tools/ActionLog.hpp"

namespace cse498 {

// Mock Agent that satisfies AgentConcept
class ActionLogMockAgent {
 public:
  void AddAction(nlohmann::json action) {
    actions.push_back(std::move(action));
  }

  const std::vector<nlohmann::json>& GetStates() const { return actions; }

 private:
  std::vector<nlohmann::json> actions;
};

TEST_CASE("LogEventByNoagent", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;
  ActionLogMockAgent agent;

  std::vector<ActionLogMockAgent> agents = {};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.size() == 0);
  REQUIRE(failures.empty());
}

TEST_CASE("LogEventByOneAgent", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;
  ActionLogMockAgent agent;
  agent.AddAction({{"agentId", "agent1"},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", 100}});

  std::vector<ActionLogMockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.size() == 1);
  REQUIRE(failures.empty());
  REQUIRE(events[0]["agentId"] == "agent1");
}

TEST_CASE("LogEventByMultipleAgent", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent1;
  agent1.AddAction({{"agentId", "agent1"},
                    {"logLevel", static_cast<int>(LogLevel::Normal)},
                    {"timestamp", 100}});

  ActionLogMockAgent agent2;
  agent2.AddAction({{"agentId", "agent2"},
                    {"logLevel", static_cast<int>(LogLevel::Verbose)},
                    {"timestamp", 101}});

  std::vector<ActionLogMockAgent> agents = {agent1, agent2};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.size() == 2);
  REQUIRE(failures.empty());
  REQUIRE(events[0]["agentId"] == "agent1");
  REQUIRE(events[1]["agentId"] == "agent2");
}

TEST_CASE("LogEventByMultipleAgentMultipleActions", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent1;
  agent1.AddAction({{"agentId", "agent1"},
                    {"logLevel", static_cast<int>(LogLevel::Normal)},
                    {"timestamp", 100}});
  agent1.AddAction({{"agentId", "agent1"},
                    {"logLevel", static_cast<int>(LogLevel::Debug)},
                    {"timestamp", 105}});

  ActionLogMockAgent agent2;
  agent2.AddAction({{"agentId", "agent2"},
                    {"logLevel", static_cast<int>(LogLevel::Verbose)},
                    {"timestamp", 110}});

  std::vector<ActionLogMockAgent> agents = {agent1, agent2};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.size() == 3);
  REQUIRE(failures.empty());
}

TEST_CASE("LogEventFailed_Timestamp", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent;
  agent.AddAction({{"agentId", "agent1"},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", -1}});  // Negative timestamp

  std::vector<ActionLogMockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 1);
}

TEST_CASE("LogEventFailed_LogLevel", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent;
  // Out of range enum value
  agent.AddAction(
      {{"agentId", "agent1"}, {"logLevel", 99}, {"timestamp", 100}});

  std::vector<ActionLogMockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 1);
}

TEST_CASE("MultipleLogFailure", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent;
  agent.AddAction({{"agentId", ""},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", 100}});  // Fail 1
  agent.AddAction({{"agentId", "agent1"},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", -1}});  // Fail 2

  std::vector<ActionLogMockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 2);
}

TEST_CASE("PartialLogFailures", "[ActionLog]") {
  ActionLog<ActionLogMockAgent> actionLog;

  ActionLogMockAgent agent;
  agent.AddAction({{"agentId", "agent1"},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", 100}});  // Success
  agent.AddAction({{"agentId", ""},
                   {"logLevel", static_cast<int>(LogLevel::Normal)},
                   {"timestamp", 101}});  // Failure

  std::vector<ActionLogMockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentStates(agents);

  REQUIRE(events.size() == 1);
  REQUIRE(events[0]["agentId"] == "agent1");
  REQUIRE(failures.size() == 1);
}

}  // namespace cse498

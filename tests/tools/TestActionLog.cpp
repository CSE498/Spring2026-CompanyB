#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include <vector>

#include "../../source/tools/ActionLog.hpp"

namespace cse498 {

// Mock Agent that satisfies AgentConcept
class MockAgent {
 public:
  void AddAction(std::string_view agentId, std::string_view actionType,
                 LogLevel level, uint64_t timestamp) {
    actions.push_back({agentId, actionType, level, timestamp});
  }

  const std::vector<ActionEventBase>& GetActions() const { return actions; }

 private:
  std::vector<ActionEventBase> actions;
};

TEST_CASE("LogEventByOneAgent", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;
  MockAgent agent;
  agent.AddAction("agent1", "move", LogLevel::Normal, 100);

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.size() == 1);
  REQUIRE(failures.empty());
  REQUIRE(events[0].agentId == "agent1");
}

TEST_CASE("LogEventByMultipleAgent", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent1;
  agent1.AddAction("agent1", "move", LogLevel::Normal, 100);

  MockAgent agent2;
  agent2.AddAction("agent2", "stay", LogLevel::Verbose, 101);

  std::vector<MockAgent> agents = {agent1, agent2};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.size() == 2);
  REQUIRE(failures.empty());
  REQUIRE(events[0].agentId == "agent1");
  REQUIRE(events[1].agentId == "agent2");
}

TEST_CASE("LogEventByMultipleAgentMultipleActions", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent1;
  agent1.AddAction("agent1", "move", LogLevel::Normal, 100);
  agent1.AddAction("agent1", "pickup", LogLevel::Debug, 105);

  MockAgent agent2;
  agent2.AddAction("agent2", "drop", LogLevel::Verbose, 110);

  std::vector<MockAgent> agents = {agent1, agent2};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.size() == 3);
  REQUIRE(failures.empty());
}

TEST_CASE("LogEventFailed_AgentId", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent;
  agent.AddAction("", "move", LogLevel::Normal, 100);  // Empty agentId

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 1);
}

TEST_CASE("LogEventFailed_ActionType", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent;
  agent.AddAction("agent1", "", LogLevel::Normal, 100);  // Empty actionType

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 1);
}

TEST_CASE("LogEventFailed_LogLevel", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent;
  // Casting out of range enum value
  agent.AddAction("agent1", "move", static_cast<LogLevel>(99), 100);

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 1);
}

TEST_CASE("MultipleLogFailure", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent;
  agent.AddAction("", "move", LogLevel::Normal, 100);    // Fail 1
  agent.AddAction("agent1", "", LogLevel::Normal, 101);  // Fail 2

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.empty());
  REQUIRE(failures.size() == 2);
}

TEST_CASE("PartialLogFailures", "[ActionLog]") {
  ActionLog<MockAgent> actionLog;

  MockAgent agent;
  agent.AddAction("agent1", "move", LogLevel::Normal, 100);  // Success
  agent.AddAction("", "stay", LogLevel::Normal, 101);        // Failure

  std::vector<MockAgent> agents = {agent};
  auto [events, failures] = actionLog.LogAgentActions(agents);

  REQUIRE(events.size() == 1);
  REQUIRE(events[0].agentId == "agent1");
  REQUIRE(failures.size() == 1);
}

}  // namespace cse498

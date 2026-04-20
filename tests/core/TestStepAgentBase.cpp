#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <nlohmann/json.hpp>
#include <vector>

#include "../../source/Interfaces/IActionLog.hpp"
#include "../../source/core/AgentData.hpp"
#include "../../source/core/StepAgentBase.hpp"
#include "../../source/core/WorldPosition.hpp"

namespace cse498 {

// Test agent using TrafficData
class TestTrafficAgent : public StepAgentBase<TrafficData> {
 public:
  TestTrafficAgent(TrafficData data, size_t id,
                   LogLevel logLevel = LogLevel::Normal, uint64_t tick = 0)
      : StepAgentBase(data, id, logLevel, tick) {}

  [[nodiscard]] steps::StepContainer GetTurn() override {
    return steps::StepContainer();
  }

  void SetGoal(WorldPosition position) override {
    mData.destination = position;
  }
};

TEST_CASE("StepAgentBase - Single Action Logging", "[StepAgentBase]") {
  TrafficData initialData{.destination = std::nullopt,
                          .position = WorldPosition{0, 0},
                          .direction = Direction::North,
                          .is_active = true,
                          .symbol = 'A',
                          .colour = "red"};

  TestTrafficAgent agent(initialData, 42, LogLevel::Normal, 0);

  SECTION("No actions initially") {
    // REQUIRE(agent.GetStates().size() ==
    //         1);  // Initial state is logged in constructor
    // const auto& initialAction = agent.GetStates()[0];
    // REQUIRE(initialAction.agentId == "42");
    // REQUIRE(initialAction.actionType == "initial_state");
    // REQUIRE(initialAction.logLevel == LogLevel::Normal);
    // REQUIRE(initialAction.timestamp == 0);
    // REQUIRE(initialAction.details.position == initialData.position);

    const auto states = agent.GetStates();
    REQUIRE(states.size() == 1);  // Initial state is logged in constructor
    const auto& initialAction = states[0];
    REQUIRE(initialAction["agentId"] == "42");
    REQUIRE(initialAction["actionType"] == "initial_state");
    REQUIRE(initialAction["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(initialAction["timestamp"] == 0);
    
    auto reconstructedDetails = TrafficData::FromJSON(initialAction["details"]);
    REQUIRE(reconstructedDetails.position == initialData.position);
  }

  SECTION("SetState logs one action") {
    TrafficData newData{.destination = std::nullopt,
                        .position = WorldPosition{1, 0},
                        .direction = Direction::East,
                        .is_active = true,
                        .symbol = 'A',
                        .colour = "red"};

    agent.SetState(newData, LogLevel::Normal, 100);

    // REQUIRE(agent.GetStates().size() == 2);  // 1 initial state + 1 movement
    // const auto& action = agent.GetStates()[1];
    // REQUIRE(action.timestamp == 100);
    // REQUIRE(action.logLevel == LogLevel::Normal);
    // REQUIRE(action.actionType == "movement");

    const auto states = agent.GetStates();
    REQUIRE(states.size() == 2);  // 1 initial state + 1 movement
    const auto& action = states[1];
    REQUIRE(action["timestamp"] == 100);
    REQUIRE(action["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(action["actionType"] == "movement");
  }
}

TEST_CASE("StepAgentBase - Agent ID Caching", "[StepAgentBase]") {
  TrafficData data{.destination = std::nullopt,
                   .position = WorldPosition{0, 0},
                   .direction = Direction::North,
                   .is_active = true,
                   .symbol = 'B',
                   .colour = "blue"};

  SECTION("Numeric agent ID is stringified and cached") {
    TestTrafficAgent agent(data, 123, LogLevel::Normal, 0);

    TrafficData newData = data;
    newData.position = WorldPosition{1, 1};
    agent.SetState(newData, LogLevel::Normal, 50);

    // REQUIRE(agent.GetStates().size() == 2);  // 1 initial state + 1 movement
    // const auto& action = agent.GetStates()[0];
    // REQUIRE(action.agentId == "123");

    const auto states = agent.GetStates();
    REQUIRE(states.size() == 2);  // 1 initial state + 1 movement
    REQUIRE(states[0]["agentId"] == "123");
  }

  SECTION("Different agent IDs produce different strings") {
    TestTrafficAgent agent1(data, 1, LogLevel::Normal, 0);
    TestTrafficAgent agent2(data, 2, LogLevel::Normal, 0);
    agent1.SetState(data, LogLevel::Normal, 10);
    agent2.SetState(data, LogLevel::Normal, 10);

    // REQUIRE(agent1.GetStates()[0].agentId == "1");
    // REQUIRE(agent2.GetStates()[0].agentId == "2");

    REQUIRE(agent1.GetStates()[0]["agentId"] == "1");
    REQUIRE(agent2.GetStates()[0]["agentId"] == "2");
  }
}

TEST_CASE("StepAgentBase - Multiple Actions", "[StepAgentBase]") {
  TrafficData data{.destination = std::nullopt,
                   .position = WorldPosition{0, 0},
                   .direction = Direction::North,
                   .is_active = true,
                   .symbol = 'C',
                   .colour = "green"};

  TestTrafficAgent agent(data, 99, LogLevel::Normal, 0);

  SECTION("Multiple SetState calls accumulate") {
    for (int i = 0; i < 5; ++i) {
      TrafficData newData = data;
      newData.position = WorldPosition{i, i};
      agent.SetState(newData, LogLevel::Normal, 100 + i);
    }

    // REQUIRE(agent.GetStates().size() == 6);  // 1 initial state + 5 movements
    REQUIRE(agent.GetStates().size() == 6);
  }

  SECTION("Log levels are preserved") {
    agent.SetState(data, LogLevel::Normal, 1);
    agent.SetState(data, LogLevel::Verbose, 2);
    agent.SetState(data, LogLevel::Debug, 3);

    // REQUIRE(agent.GetStates()[0].logLevel ==
    //         LogLevel::Normal);  // Initial state log level
    // REQUIRE(agent.GetStates()[1].logLevel == LogLevel::Normal);
    // REQUIRE(agent.GetStates()[2].logLevel == LogLevel::Verbose);
    // REQUIRE(agent.GetStates()[3].logLevel == LogLevel::Debug);

    const auto states = agent.GetStates();
    REQUIRE(states[0]["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(states[1]["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(states[2]["logLevel"] == static_cast<int>(LogLevel::Verbose));
    REQUIRE(states[3]["logLevel"] == static_cast<int>(LogLevel::Debug));
  }

  SECTION("Timestamps are preserved") {
    agent.SetState(data, LogLevel::Normal, 1000);
    agent.SetState(data, LogLevel::Normal, 2000);
    agent.SetState(data, LogLevel::Normal, 3000);

    // REQUIRE(agent.GetStates()[0].timestamp == 0);  // Initial state timestamp
    // REQUIRE(agent.GetStates()[1].timestamp == 1000);
    // REQUIRE(agent.GetStates()[2].timestamp == 2000);
    // REQUIRE(agent.GetStates()[3].timestamp == 3000);

    const auto states = agent.GetStates();
    REQUIRE(states[0]["timestamp"] == 0);
    REQUIRE(states[1]["timestamp"] == 1000);
    REQUIRE(states[2]["timestamp"] == 2000);
    REQUIRE(states[3]["timestamp"] == 3000);
  }
}

TEST_CASE("StepAgentBase - JSON Serialization", "[StepAgentBase]") {
  TrafficData data{.destination = std::nullopt,
                   .position = WorldPosition{5, 10},
                   .direction = Direction::South,
                   .is_active = true,
                   .symbol = 'D',
                   .colour = "yellow"};

  TestTrafficAgent agent(data, 77, LogLevel::Normal, 0);
  agent.SetState(data, LogLevel::Verbose, 500);

  // const auto& actions = agent.GetStates();
  const auto actions = agent.GetStates();
  REQUIRE(actions.size() == 2);  // 1 initial state + 1 movement

  SECTION("Action can be serialized to JSON") {
    // nlohmann::json j;
    // j["agentId"] = actions[0].agentId;
    // j["actionType"] = actions[0].actionType;
    // j["logLevel"] = static_cast<int>(actions[0].logLevel);
    // j["timestamp"] = actions[0].timestamp;
    //
    // REQUIRE(j["agentId"] == "77");
    // REQUIRE(j["actionType"] == "initial_state");
    // REQUIRE(j["logLevel"] == static_cast<int>(LogLevel::Normal));
    // REQUIRE(j["timestamp"] == 0);
    //
    // j["agentId"] = actions[1].agentId;
    // j["actionType"] = actions[1].actionType;
    // j["logLevel"] = static_cast<int>(actions[1].logLevel);
    // j["timestamp"] = actions[1].timestamp;
    //
    // REQUIRE(j["agentId"] == "77");
    // REQUIRE(j["actionType"] == "movement");
    // REQUIRE(j["logLevel"] == static_cast<int>(LogLevel::Verbose));
    // REQUIRE(j["timestamp"] == 500);
    
    // The objects are already JSON natively, verify their properties directly
    REQUIRE(actions[0]["agentId"] == "77");
    REQUIRE(actions[0]["actionType"] == "initial_state");
    REQUIRE(actions[0]["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(actions[0]["timestamp"] == 0);

    REQUIRE(actions[1]["agentId"] == "77");
    REQUIRE(actions[1]["actionType"] == "movement");
    REQUIRE(actions[1]["logLevel"] == static_cast<int>(LogLevel::Verbose));
    REQUIRE(actions[1]["timestamp"] == 500);
  }
}

TEST_CASE("StepAgentBase - Cached Agent ID Lifetime", "[StepAgentBase]") {
  TrafficData data{.destination = std::nullopt,
                   .position = WorldPosition{0, 0},
                   .direction = Direction::North,
                   .is_active = true,
                   .symbol = 'E',
                   .colour = "cyan"};

  SECTION("Agent ID remains valid after construction") {
    TestTrafficAgent agent(data, 555, LogLevel::Normal, 0);
    agent.SetState(data, LogLevel::Normal, 100);

    // const auto& action = agent.GetStates()[0];
    // std::string agentId(action.agentId);
    // REQUIRE(agentId == "555");
    //
    // agent.SetState(data, LogLevel::Normal, 200);
    // REQUIRE(agent.GetStates()[0].agentId == "555");
    // REQUIRE(agent.GetStates()[1].agentId == "555");

    auto states = agent.GetStates();
    std::string agentId = states[0]["agentId"].get<std::string>();
    REQUIRE(agentId == "555");

    agent.SetState(data, LogLevel::Normal, 200);
    states = agent.GetStates(); 
    // Vector gets cleared upon read, so only 1 new event exists!
    REQUIRE(states[0]["agentId"] == "555");
  }
}

TEST_CASE("StepAgentBase - Data Preservation", "[StepAgentBase]") {
  TrafficData initialData{.destination = std::nullopt,
                          .position = WorldPosition{0, 0},
                          .direction = cse498::Direction::North,
                          .is_active = true,
                          .symbol = 'F',
                          .colour = "magenta"};

  TestTrafficAgent agent(initialData, 88, LogLevel::Normal, 0);

  TrafficData newData{.destination = WorldPosition{10, 20},
                      .position = WorldPosition{5, 5},
                      .direction = cse498::Direction::West,
                      .is_active = false,
                      .symbol = 'X',
                      .colour = "black"};

  agent.SetState(newData, LogLevel::Debug, 999);

  SECTION("SetState updates agent data") {
    const auto current = agent.GetState();
    REQUIRE(current.position.X() == 5);
    REQUIRE(current.position.Y() == 5);
    REQUIRE(current.direction == cse498::Direction::West);
    REQUIRE(current.is_active == false);
    REQUIRE(current.symbol == 'X');
    REQUIRE(current.colour == "black");
  }

  SECTION("Action stores the data details") {
    // const auto action = agent.GetStates();
    // REQUIRE(action[0].details.position.X() == 0);
    // REQUIRE(action[0].details.position.Y() == 0);
    // REQUIRE(action[0].details.direction == cse498::Direction::North);
    //
    // REQUIRE(action[1].details.position.X() == 5);
    // REQUIRE(action[1].details.position.Y() == 5);
    // REQUIRE(action[1].details.direction == cse498::Direction::West);

    const auto states = agent.GetStates();
    
    auto details0 = TrafficData::FromJSON(states[0]["details"]);
    REQUIRE(details0.position.X() == 0);
    REQUIRE(details0.position.Y() == 0);
    REQUIRE(details0.direction == cse498::Direction::North);

    auto details1 = TrafficData::FromJSON(states[1]["details"]);
    REQUIRE(details1.position.X() == 5);
    REQUIRE(details1.position.Y() == 5);
    REQUIRE(details1.direction == cse498::Direction::West);
  }
}

TEST_CASE("StepAgentBase - ActionLog Concept Compliance", "[StepAgentBase]") {
  TrafficData data{.destination = std::nullopt,
                   .position = WorldPosition{0, 0},
                   .direction = Direction::North,
                   .is_active = true,
                   .symbol = 'G',
                   .colour = "white"};

  TestTrafficAgent agent(data, 11, LogLevel::Normal, 0);
  agent.SetState(data, LogLevel::Normal, 100);
  agent.SetState(data, LogLevel::Normal, 200);

  // const auto& states = agent.GetStates();
  // REQUIRE(states.size() == 3);  // 1 initial state + 2 movements
  //
  // REQUIRE(states[0].agentId == "11");
  // REQUIRE(states[0].actionType == "initial_state");
  // REQUIRE(states[0].logLevel == LogLevel::Normal);
  // REQUIRE(states[0].timestamp == 0);
  //
  // REQUIRE(states[1].agentId == "11");
  // REQUIRE(states[1].actionType == "movement");
  // REQUIRE(states[1].logLevel == LogLevel::Normal);
  // REQUIRE(states[1].timestamp == 100);
  //
  // REQUIRE(states[2].agentId == "11");
  // REQUIRE(states[2].actionType == "movement");
  // REQUIRE(states[2].logLevel == LogLevel::Normal);
  // REQUIRE(states[2].timestamp == 200);

  const auto states = agent.GetStates();
  REQUIRE(states.size() == 3);  // 1 initial state + 2 movements

  REQUIRE(states[0]["agentId"] == "11");
  REQUIRE(states[0]["actionType"] == "initial_state");
  REQUIRE(states[0]["logLevel"] == static_cast<int>(LogLevel::Normal));
  REQUIRE(states[0]["timestamp"] == 0);

  REQUIRE(states[1]["agentId"] == "11");
  REQUIRE(states[1]["actionType"] == "movement");
  REQUIRE(states[1]["logLevel"] == static_cast<int>(LogLevel::Normal));
  REQUIRE(states[1]["timestamp"] == 100);

  REQUIRE(states[2]["agentId"] == "11");
  REQUIRE(states[2]["actionType"] == "movement");
  REQUIRE(states[2]["logLevel"] == static_cast<int>(LogLevel::Normal));
  REQUIRE(states[2]["timestamp"] == 200);
}

}  // namespace cse498

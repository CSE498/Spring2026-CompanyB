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
    REQUIRE(agent.GetStates().size() ==
            1);  // Initial state is logged in constructor
    const auto& initialAction = agent.GetStates()[0];
    REQUIRE(initialAction.at("agentId").get<std::string>() == "42");
    REQUIRE(initialAction.at("actionType").get<std::string>() ==
            "initial_state");
    REQUIRE(initialAction.at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Normal));
    REQUIRE(initialAction.at("timestamp").get<uint64_t>() == 0);
    REQUIRE(initialAction.at("details").at("position").at("x").get<int>() ==
            initialData.position.CellX());
    REQUIRE(initialAction.at("details").at("position").at("y").get<int>() ==
            initialData.position.CellY());
  }

  SECTION("SetState logs one action") {
    TrafficData newData{.destination = std::nullopt,
                        .position = WorldPosition{1, 0},
                        .direction = Direction::East,
                        .is_active = true,
                        .symbol = 'A',
                        .colour = "red"};

    agent.SetState(newData, LogLevel::Normal, 100);

    REQUIRE(agent.GetStates().size() == 2);  // 1 initial state + 1 movement
    const auto& action = agent.GetStates()[1];
    REQUIRE(action.at("timestamp").get<uint64_t>() == 100);
    REQUIRE(action.at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Normal));
    REQUIRE(action.at("actionType").get<std::string>() == "movement");
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

    REQUIRE(agent.GetStates().size() == 2);  // 1 initial state + 1 movement
    const auto& action = agent.GetStates()[0];
    REQUIRE(action.at("agentId").get<std::string>() == "123");
  }

  SECTION("Different agent IDs produce different strings") {
    TestTrafficAgent agent1(data, 1, LogLevel::Normal, 0);
    TestTrafficAgent agent2(data, 2, LogLevel::Normal, 0);
    agent1.SetState(data, LogLevel::Normal, 10);
    agent2.SetState(data, LogLevel::Normal, 10);

    REQUIRE(agent1.GetStates()[0].at("agentId").get<std::string>() == "1");
    REQUIRE(agent2.GetStates()[0].at("agentId").get<std::string>() == "2");
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

    REQUIRE(agent.GetStates().size() == 6);  // 1 initial state + 5 movements
  }

  SECTION("Log levels are preserved") {
    agent.SetState(data, LogLevel::Normal, 1);
    agent.SetState(data, LogLevel::Verbose, 2);
    agent.SetState(data, LogLevel::Debug, 3);

    REQUIRE(agent.GetStates()[0].at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Normal));  // Initial state log level
    REQUIRE(agent.GetStates()[1].at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Normal));
    REQUIRE(agent.GetStates()[2].at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Verbose));
    REQUIRE(agent.GetStates()[3].at("logLevel").get<int>() ==
            static_cast<int>(LogLevel::Debug));
  }

  SECTION("Timestamps are preserved") {
    agent.SetState(data, LogLevel::Normal, 1000);
    agent.SetState(data, LogLevel::Normal, 2000);
    agent.SetState(data, LogLevel::Normal, 3000);

    REQUIRE(agent.GetStates()[0].at("timestamp").get<uint64_t>() ==
            0);  // Initial state timestamp
    REQUIRE(agent.GetStates()[1].at("timestamp").get<uint64_t>() == 1000);
    REQUIRE(agent.GetStates()[2].at("timestamp").get<uint64_t>() == 2000);
    REQUIRE(agent.GetStates()[3].at("timestamp").get<uint64_t>() == 3000);
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

  const auto& actions = agent.GetStates();
  REQUIRE(actions.size() == 2);  // 1 initial state + 1 movement

  SECTION("Action can be serialized to JSON") {
    nlohmann::json j;
    j["agentId"] = actions[0].at("agentId").get<std::string>();
    j["actionType"] = actions[0].at("actionType").get<std::string>();
    j["logLevel"] = static_cast<int>(actions[0].at("logLevel").get<int>());
    j["timestamp"] = actions[0].at("timestamp").get<uint64_t>();

    REQUIRE(j["agentId"] == "77");
    REQUIRE(j["actionType"] == "initial_state");
    REQUIRE(j["logLevel"] == static_cast<int>(LogLevel::Normal));
    REQUIRE(j["timestamp"] == 0);

    j["agentId"] = actions[1].at("agentId").get<std::string>();
    j["actionType"] = actions[1].at("actionType").get<std::string>();
    j["logLevel"] = static_cast<int>(actions[1].at("logLevel").get<int>());
    j["timestamp"] = actions[1].at("timestamp").get<uint64_t>();

    REQUIRE(j["agentId"] == "77");
    REQUIRE(j["actionType"] == "movement");
    REQUIRE(j["logLevel"] == static_cast<int>(LogLevel::Verbose));
    REQUIRE(j["timestamp"] == 500);
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

    const auto& action = agent.GetStates()[0];
    std::string agentId(action.at("agentId").get<std::string>());
    REQUIRE(agentId == "555");

    agent.SetState(data, LogLevel::Normal, 200);
    REQUIRE(agent.GetStates()[0].at("agentId").get<std::string>() == "555");
    REQUIRE(agent.GetStates()[1].at("agentId").get<std::string>() == "555");
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
    const auto action = agent.GetStates();
    REQUIRE(action[0].at("details").at("position").at("x").get<int>() == 0);
    REQUIRE(action[0].at("details").at("position").at("y").get<int>() == 0);
    REQUIRE(action[0].at("details").at("direction").get<int>() ==
            static_cast<int>(cse498::Direction::North));

    REQUIRE(action[1].at("details").at("position").at("x").get<int>() == 5);
    REQUIRE(action[1].at("details").at("position").at("y").get<int>() == 5);
    REQUIRE(action[1].at("details").at("direction").get<int>() ==
            static_cast<int>(cse498::Direction::West));
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

  const auto& states = agent.GetStates();
  REQUIRE(states.size() == 3);  // 1 initial state + 2 movements

  REQUIRE(states[0].at("agentId").get<std::string>() == "11");
  REQUIRE(states[0].at("actionType").get<std::string>() == "initial_state");
  REQUIRE(states[0].at("logLevel").get<int>() ==
          static_cast<int>(LogLevel::Normal));
  REQUIRE(states[0].at("timestamp").get<uint64_t>() == 0);

  REQUIRE(states[1].at("agentId").get<std::string>() == "11");
  REQUIRE(states[1].at("actionType").get<std::string>() == "movement");
  REQUIRE(states[1].at("logLevel").get<int>() ==
          static_cast<int>(LogLevel::Normal));
  REQUIRE(states[1].at("timestamp").get<uint64_t>() == 100);

  REQUIRE(states[2].at("agentId").get<std::string>() == "11");
  REQUIRE(states[2].at("actionType").get<std::string>() == "movement");
  REQUIRE(states[2].at("logLevel").get<int>() ==
          static_cast<int>(LogLevel::Normal));
  REQUIRE(states[2].at("timestamp").get<uint64_t>() == 200);
}

}  // namespace cse498

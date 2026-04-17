#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <nlohmann/json.hpp>
#include <vector>

#include "../../source/core/WorldPosition.hpp"
#include "../../source/core/AgentData.hpp"
#include "../../source/core/StepAgentBase.hpp"
#include "../../source/Interfaces/IActionLog.hpp"

namespace cse498 {

// Test agent using TrafficData
class TestTrafficAgent : public StepAgentBase<TrafficData> {
 public:
  TestTrafficAgent(TrafficData data, size_t id)
      : StepAgentBase(data, id) {}

  [[nodiscard]] steps::StepContainer GetTurn() override {
    return steps::StepContainer();
  }

  void SetGoal(WorldPosition position) override {
    mData.destination = position;
  }
};

TEST_CASE("StepAgentBase - Single Action Logging", "[StepAgentBase]") {
  TrafficData initialData{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = Direction::North,
      .is_active = true,
      .symbol = 'A',
      .colour = "red"
  };

  TestTrafficAgent agent(initialData, 42);

  SECTION("No actions initially") {
    REQUIRE(agent.GetStates().empty());
  }

  SECTION("SetState logs one action") {
    TrafficData newData{
        .destination = std::nullopt,
        .position = WorldPosition{1, 0},
        .direction = Direction::East,
        .is_active = true,
        .symbol = 'A',
        .colour = "red"
    };

    agent.SetState(newData, LogLevel::Normal, 100);

    REQUIRE(agent.GetStates().size() == 1);
    const auto& action = agent.GetStates()[0];
    REQUIRE(action.timestamp == 100);
    REQUIRE(action.logLevel == LogLevel::Normal);
    REQUIRE(action.actionType == "movement");
  }
}

TEST_CASE("StepAgentBase - Agent ID Caching", "[StepAgentBase]") {
  TrafficData data{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = Direction::North,
      .is_active = true,
      .symbol = 'B',
      .colour = "blue"
  };

  SECTION("Numeric agent ID is stringified and cached") {
    TestTrafficAgent agent(data, 123);

    TrafficData newData = data;
    newData.position = WorldPosition{1, 1};
    agent.SetState(newData, LogLevel::Normal, 50);

    REQUIRE(agent.GetStates().size() == 1);
    const auto& action = agent.GetStates()[0];
    REQUIRE(action.agentId == "123");
  }

  SECTION("Different agent IDs produce different strings") {
    TestTrafficAgent agent1(data, 1);
    TestTrafficAgent agent2(data, 2);

    agent1.SetState(data, LogLevel::Normal, 10);
    agent2.SetState(data, LogLevel::Normal, 10);

    REQUIRE(agent1.GetStates()[0].agentId == "1");
    REQUIRE(agent2.GetStates()[0].agentId == "2");
  }
}

TEST_CASE("StepAgentBase - Multiple Actions", "[StepAgentBase]") {
  TrafficData data{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = Direction::North,
      .is_active = true,
      .symbol = 'C',
      .colour = "green"
  };

  TestTrafficAgent agent(data, 99);

  SECTION("Multiple SetState calls accumulate") {
    for (int i = 0; i < 5; ++i) {
      TrafficData newData = data;
      newData.position = WorldPosition{i, i};
      agent.SetState(newData, LogLevel::Normal, 100 + i);
    }

    REQUIRE(agent.GetStates().size() == 5);
  }

  SECTION("Log levels are preserved") {
    agent.SetState(data, LogLevel::Normal, 1);
    agent.SetState(data, LogLevel::Verbose, 2);
    agent.SetState(data, LogLevel::Debug, 3);

    REQUIRE(agent.GetStates()[0].logLevel == LogLevel::Normal);
    REQUIRE(agent.GetStates()[1].logLevel == LogLevel::Verbose);
    REQUIRE(agent.GetStates()[2].logLevel == LogLevel::Debug);
  }

  SECTION("Timestamps are preserved") {
    agent.SetState(data, LogLevel::Normal, 1000);
    agent.SetState(data, LogLevel::Normal, 2000);
    agent.SetState(data, LogLevel::Normal, 3000);

    REQUIRE(agent.GetStates()[0].timestamp == 1000);
    REQUIRE(agent.GetStates()[1].timestamp == 2000);
    REQUIRE(agent.GetStates()[2].timestamp == 3000);
  }
}

TEST_CASE("StepAgentBase - JSON Serialization", "[StepAgentBase]") {
  TrafficData data{
      .destination = std::nullopt,
      .position = WorldPosition{5, 10},
      .direction = Direction::South,
      .is_active = true,
      .symbol = 'D',
      .colour = "yellow"
  };

  TestTrafficAgent agent(data, 77);
  agent.SetState(data, LogLevel::Verbose, 500);

  const auto& actions = agent.GetStates();
  REQUIRE(actions.size() == 1);

  SECTION("Action can be serialized to JSON") {
    const auto& action = actions[0];
    nlohmann::json j;
    j["agentId"] = action.agentId;
    j["actionType"] = action.actionType;
    j["logLevel"] = static_cast<int>(action.logLevel);
    j["timestamp"] = action.timestamp;

    REQUIRE(j["agentId"] == "77");
    REQUIRE(j["actionType"] == "movement");
    REQUIRE(j["logLevel"] == static_cast<int>(LogLevel::Verbose));
    REQUIRE(j["timestamp"] == 500);
  }
}

TEST_CASE("StepAgentBase - Cached Agent ID Lifetime", "[StepAgentBase]") {
  TrafficData data{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = Direction::North,
      .is_active = true,
      .symbol = 'E',
      .colour = "cyan"
  };

  SECTION("Agent ID remains valid after construction") {
    TestTrafficAgent agent(data, 555);
    agent.SetState(data, LogLevel::Normal, 100);

    const auto& action = agent.GetStates()[0];
    std::string agentId(action.agentId);
    REQUIRE(agentId == "555");

    agent.SetState(data, LogLevel::Normal, 200);
    REQUIRE(agent.GetStates()[0].agentId == "555");
    REQUIRE(agent.GetStates()[1].agentId == "555");
  }
}

TEST_CASE("StepAgentBase - Data Preservation", "[StepAgentBase]") {
  TrafficData initialData{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = cse498::Direction::North,
      .is_active = true,
      .symbol = 'F',
      .colour = "magenta"
  };

  TestTrafficAgent agent(initialData, 88);

  TrafficData newData{
      .destination = WorldPosition{10, 20},
      .position = WorldPosition{5, 5},
      .direction = cse498::Direction::West,
      .is_active = false,
      .symbol = 'X',
      .colour = "black"
  };

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
    const auto& action = agent.GetStates()[0];
    REQUIRE(action.details.position.X() == 5);
    REQUIRE(action.details.position.Y() == 5);
    REQUIRE(action.details.direction == cse498::Direction::West);
  }
}

TEST_CASE("StepAgentBase - ActionLog Concept Compliance", "[StepAgentBase]") {
  TrafficData data{
      .destination = std::nullopt,
      .position = WorldPosition{0, 0},
      .direction = Direction::North,
      .is_active = true,
      .symbol = 'G',
      .colour = "white"
  };

  TestTrafficAgent agent(data, 11);
  agent.SetState(data, LogLevel::Normal, 100);
  agent.SetState(data, LogLevel::Normal, 200);

  const auto& states = agent.GetStates();
  REQUIRE(states.size() == 2);

  for (const auto& state : states) {
    REQUIRE(!state.agentId.empty());
    REQUIRE(state.actionType == "movement");
    REQUIRE(static_cast<int>(state.logLevel) >= static_cast<int>(LogLevel::Normal));
    REQUIRE(static_cast<int>(state.logLevel) <= static_cast<int>(LogLevel::Silent));
    REQUIRE(state.timestamp > 0);
  }
}

}  // namespace cse498

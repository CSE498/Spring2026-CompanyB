#include <catch2/catch_test_macros.hpp>

#include "../../source/Worlds/StepTrafficWorld.hpp"
#include "../../source/core/AgentData.hpp"
#include "../../source/core/StepAgentBase.hpp"
#include "../../source/tools/DataLog.hpp"

using namespace cse498;

TEST_CASE("Traffic world integrates with DataLog", "[DataLog][Traffic]") {
  // Minimal grid with one spawner and destination so agents can be spawned.
  static const std::vector<std::string> kMinimal = {
      "#########",
      "#S..|..D#",
      "#########",
  };

  // Provide a tiny concrete agent type so the world can instantiate agents.
  struct TestTrafficAgent : public StepAgentBase<TrafficData> {
    TestTrafficAgent(TrafficData d, size_t id)
        : StepAgentBase<TrafficData>(d, id) {}
    steps::StepContainer GetTurn() override { return steps::StepContainer(); }
    void SetGoal(WorldPosition) override {}
  };

  StepTrafficWorld<TestTrafficAgent> world{kMinimal};

  // Let the world run a few ticks by stepping RunAgents()+UpdateWorld()
  for (int i = 0; i < 3; ++i) {
    world.RunAgents();
    world.UpdateWorld();
  }

  const auto& log = world.GetTrafficDataLog().GetAggregationData();

  // Expect the known traffic keys to exist and have at least one tick recorded
  REQUIRE(log.contains("active_count"));
  REQUIRE_FALSE(log.at("active_count").empty());
  REQUIRE(log.contains("distance_driven"));
}

TEST_CASE("Infection DataLog aggregates declared fields",
          "[DataLog][Infection]") {
  using namespace cse498;

  // Create a tiny set of disease agents with varied health states.
  struct TestDiseaseAgent : public StepAgentBase<DiseaseData> {
    TestDiseaseAgent(DiseaseData d, size_t id)
        : StepAgentBase<DiseaseData>(d, id) {}
    steps::StepContainer GetTurn() override { return steps::StepContainer(); }
    void SetGoal(WorldPosition) override {}
  };

  std::vector<std::shared_ptr<StepAgentBase<DiseaseData>>> agents;
  agents.push_back(std::make_shared<TestDiseaseAgent>(
      DiseaseData{WorldPosition{0, 0}, HealthState::SUSCEPTIBLE, 0}, 0));
  agents.push_back(std::make_shared<TestDiseaseAgent>(
      DiseaseData{WorldPosition{1, 1}, HealthState::INFECTED, 0}, 1));
  agents.push_back(std::make_shared<TestDiseaseAgent>(
      DiseaseData{WorldPosition{2, 2}, HealthState::RECOVERED, 0}, 2));

  DataLog<DiseaseData> log{WorldType::Infection};
  log.AggregateData(agents);

  const auto& agg = log.GetAggregationData();
  REQUIRE(agg.contains("infection_count"));
  REQUIRE(agg.contains("susceptible_count"));
  REQUIRE(agg.contains("cured_count"));
  REQUIRE(agg.contains("infection_probability"));

  // Each declared field should have at least one tick recorded after
  // aggregation
  for (const auto& [k, v] : agg) {
    REQUIRE_FALSE(v.empty());
  }
}
// #include <catch2/catch_test_macros.hpp>
// #include <nlohmann/json.hpp>

// #include "../../source/tools/DataLog.hpp"
/*
TEST_CASE("DataLog can add and retrieve entries with duration", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json entry = {
        {"agentId", "agent_1"},
        {"actionType", "move"},
        {"duration", 5.0},
        {"summary", "Moved forward"}
    };

    dataLog.AddEntry(entry);

    REQUIRE(dataLog.GetCount() == 1);
    REQUIRE(dataLog.GetEntries().size() == 1);
    REQUIRE(dataLog.GetEntries()[0]["agentId"] == "agent_1");
    REQUIRE(dataLog.GetEntries()[0].contains("timestamp"));
}

TEST_CASE("DataLog calculates mean, median, min, and max correctly",
"[DataLog]") { cse498::DataLog dataLog;

    nlohmann::json entry1 = {
        {"agentId", "agent_1"},
        {"actionType", "move"},
        {"duration", 2.0},
        {"summary", "First move"}
    };

    nlohmann::json entry2 = {
        {"agentId", "agent_2"},
        {"actionType", "turn"},
        {"duration", 4.0},
        {"summary", "Turn action"}
    };

    nlohmann::json entry3 = {
        {"agentId", "agent_3"},
        {"actionType", "move"},
        {"duration", 6.0},
        {"summary", "Second move"}
    };

    dataLog.AddEntry(entry1);
    dataLog.AddEntry(entry2);
    dataLog.AddEntry(entry3);

    REQUIRE(dataLog.GetCount() == 3);
    REQUIRE(dataLog.GetMean().has_value());
    REQUIRE(dataLog.GetMean().value() == 4.0);
    REQUIRE(dataLog.GetMedian().has_value());
    REQUIRE(dataLog.GetMedian().value() == 4.0);
    REQUIRE(dataLog.GetMin().has_value());
    REQUIRE(dataLog.GetMin().value() == 2.0);
    REQUIRE(dataLog.GetMax().has_value());
    REQUIRE(dataLog.GetMax().value() == 6.0);
}

TEST_CASE("DataLog Reset clears all entries and statistics", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json entry = {
        {"agentId", "agent_1"},
        {"actionType", "move"},
        {"duration", 5.0},
        {"summary", "Test move"}
    };

    dataLog.AddEntry(entry);
    REQUIRE(dataLog.GetCount() == 1);

    dataLog.Reset();

    REQUIRE(dataLog.GetCount() == 0);
    REQUIRE(dataLog.GetEntries().empty());
    REQUIRE(!dataLog.GetMean().has_value());
}

TEST_CASE(
    "InfectiousWorld aggregates into world-owned DataLog after UpdateWorld",
    "[datalog][infection][world-hook]") {
  InfectiousWorld world(10, 8);

  DiseaseData a{};
  a.position = WorldPosition{2, 2};
  DiseaseData b{};
  b.position = WorldPosition{4, 3};

  world.AddAgent<SwarmingAgent<DiseaseData>>(a);
  world.AddAgent<SwarmingAgent<DiseaseData>>(b);

  world.RunAgents();
  world.UpdateWorld();

  auto const& data = world.GetDataLog().GetAggregationData();
  for (auto const& kv : data) {
    REQUIRE(kv.second.size() == 1);
    CHECK(kv.second.back().count == 2);
  }
}

TEST_CASE("DataLog traffic metrics aggregate from TrafficData",
          "[datalog][traffic]") {
  std::vector<std::shared_ptr<StepAgentBase<TrafficData>>> agents;

  TrafficData driving{};
  driving.position = WorldPosition{1, 1};
  driving.destination = WorldPosition{4, 1};
  driving.is_active = true;

  TrafficData waiting{};
  waiting.position = WorldPosition{3, 2};
  waiting.destination = std::nullopt;
  waiting.is_active = false;

  agents.push_back(std::make_shared<SwarmingAgent<TrafficData>>(driving, 0));
  agents.push_back(std::make_shared<SwarmingAgent<TrafficData>>(waiting, 1));

  DataLog<TrafficData> datalog(WorldType::Traffic);
  datalog.AggregateData(agents);

  auto const& data = datalog.GetAggregationData();

  REQUIRE(data.at("waiting_count").size() == 1);
  REQUIRE(data.at("driving_count").size() == 1);
  REQUIRE(data.at("active_count").size() == 1);
  REQUIRE(data.at("distance_driven").size() == 1);
  REQUIRE(data.at("time_to_arrive").size() == 1);

  CHECK(data.at("waiting_count").back().sum == 1.0);
  CHECK(data.at("driving_count").back().sum == 1.0);
  CHECK(data.at("active_count").back().sum == 1.0);
  CHECK(data.at("distance_driven").back().sum == 0.0);
  CHECK(data.at("time_to_arrive").back().sum == 3.0);
}
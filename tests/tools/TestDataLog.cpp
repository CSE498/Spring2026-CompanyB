#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

#include "Agents/SwarmingAgent.hpp"
#include "Worlds/InfectiousWorld.hpp"
#include "tools/DataLog.hpp"

using namespace cse498;
using Catch::Approx;

TEST_CASE("DataLog infection metrics aggregate from DiseaseData",
          "[datalog][infection]") {
  std::vector<std::shared_ptr<StepAgentBase<DiseaseData>>> agents;

  DiseaseData infected{};
  infected.position = WorldPosition{1, 1};
  infected.health = HealthState::INFECTED;

  DiseaseData susceptible{};
  susceptible.position = WorldPosition{2, 2};
  susceptible.health = HealthState::SUSCEPTIBLE;

  DiseaseData recovered{};
  recovered.position = WorldPosition{3, 3};
  recovered.health = HealthState::RECOVERED;

  agents.push_back(std::make_shared<SwarmingAgent<DiseaseData>>(infected, 0));
  agents.push_back(
      std::make_shared<SwarmingAgent<DiseaseData>>(susceptible, 1));
  agents.push_back(std::make_shared<SwarmingAgent<DiseaseData>>(recovered, 2));

  DataLog<DiseaseData> datalog(WorldType::Infection);
  datalog.AggregateData(agents);

  auto const& data = datalog.GetAggregationData();

  REQUIRE(data.at("infection_count").size() == 1);
  REQUIRE(data.at("susceptible_count").size() == 1);
  REQUIRE(data.at("cured_count").size() == 1);
  REQUIRE(data.at("infection_probability").size() == 1);

  CHECK(data.at("infection_count").back().sum == 1.0);
  CHECK(data.at("susceptible_count").back().sum == 1.0);
  CHECK(data.at("cured_count").back().sum == 1.0);
  CHECK(data.at("infection_probability").back().mean == Approx(1.0 / 3.0));
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
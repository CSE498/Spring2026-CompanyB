#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../../source/tools/ReplayDriver.hpp"
#include "MockWorld.hpp"

TEST_CASE("ReplayDriver can read and replay events from a JSON file",
          "[ReplayDriver]") {
  // Create a sample JSON file with event data
  cse498::MockWorld mockWorld;
  cse498::MockAgent agent;
  mockWorld.agents.push_back(&agent);
  cse498::MockAgent* mockAgent = mockWorld.agents[0];

  mockAgent->getLoggable();

  cse498::ReplayDriver<cse498::MockAgent> replayDriver;

  replayDriver.AgentMap("test_events.json");

  auto map = replayDriver.AgentMap("test_events.json");
  std::unordered_map<int, std::string> expectedMap = {{5, "MockAgent"}};

  REQUIRE(map.value() == expectedMap);

  auto& agents = mockWorld.getReplayAgents();
  auto result = replayDriver.ReplayFromFile("test_events.json", agents);

  REQUIRE(result.has_value());
  REQUIRE(result.value());

  // Verify that the agent's state has been updated based on the JSON data
  REQUIRE(mockAgent->getId() == "5");
  REQUIRE(mockAgent->position == std::vector<int>{1, 2});
  REQUIRE(mockAgent->etc == "test value");

  // Clean up the test file
  std::remove("test_events.json");
}

TEST_CASE("ReplayDriver handles invalid file paths gracefully",
          "[ReplayDriver]") {
  cse498::MockWorld mockWorld;
  cse498::ReplayDriver<cse498::MockAgent> replayDriver;

  auto& agents = mockWorld.getReplayAgents();
  auto result = replayDriver.ReplayFromFile("non_existent_file.json", agents);

  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == "Failed to open file");
}

TEST_CASE("ReplayDriver handles malformed JSON gracefully", "[ReplayDriver]") {
  // Create a malformed JSON file
  std::ofstream outFile("malformed.json");
  outFile << "{ invalid json }";
  outFile.close();

  cse498::MockWorld mockWorld;
  cse498::ReplayDriver<cse498::MockAgent> replayDriver;

  auto& agents = mockWorld.getReplayAgents();
  auto result = replayDriver.ReplayFromFile("malformed.json", agents);

  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == "Failed to parse JSON");

  // Clean up the test file
  std::remove("malformed.json");
}
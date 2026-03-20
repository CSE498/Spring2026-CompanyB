#include <catch2/catch_test_macros.hpp>
#include "../../source/tools/ReplayDriver.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

TEST_CASE("ReplayDriver can read and replay events from a JSON file", "[ReplayDriver]") {
    // Create a sample JSON file with event data
    auto mockWorld = std::make_shared<cse498::MockWorld>();

    cse498::MockAgent agent;

    mockWorld->agents.push_back(&agent);
    cse498::MockAgent* mockAgent = mockWorld->agents[0];
    
    mockAgent->getLoggable();

    cse498::ReplayDriver replayDriver(mockWorld);
    auto result = replayDriver.ReplayFromFile("test_events.json");

    REQUIRE(result.value());

    // Verify that the agent's state has been updated based on the JSON data
    REQUIRE(mockAgent->id == 5);
    REQUIRE(mockAgent->position == std::vector<int>{1, 2});
    REQUIRE(mockAgent->etc == "test value");

    // Clean up the test file
    std::remove("test_events.json");
}

TEST_CASE("ReplayDriver handles invalid file paths gracefully", "[ReplayDriver]") {
    auto mockWorld = std::make_shared<cse498::MockWorld>();
    cse498::ReplayDriver<cse498::MockWorld> replayDriver(mockWorld);
    
    auto result = replayDriver.ReplayFromFile("non_existent_file.json");
    
    REQUIRE(!result);
}

TEST_CASE("ReplayDriver handles malformed JSON gracefully", "[ReplayDriver]") {
    // Create a malformed JSON file
    std::ofstream outFile("malformed.json");
    outFile << "{ invalid json }";
    outFile.close();

    auto mockWorld = std::make_shared<cse498::MockWorld>();
    cse498::ReplayDriver<cse498::MockWorld> replayDriver(mockWorld);
    
    auto result = replayDriver.ReplayFromFile("malformed.json");
    
    REQUIRE(!result);

    // Clean up the test file
    std::remove("malformed.json");
}
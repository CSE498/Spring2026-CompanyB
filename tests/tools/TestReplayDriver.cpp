#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

TEST_CASE("ReplayDriver can read and replay events from a JSON file", "[ReplayDriver]") {
    // Create a sample JSON file with event data
    nlohmann::json sampleEventData = {
        {"events", {
            {"type", "agent_added"},
            {"agent_id", 1},
            {"position", {0, 0}}
        }}
    };

    std::string testFilePath = "test_events.json";
    std::ofstream outFile(testFilePath);
    outFile << sampleEventData.dump(4);
    outFile.close();

    cse498::ReplayDriver replayDriver;
    bool result = replayDriver.ReplayFromFile(testFilePath);

    REQUIRE(result == true);

    // Clean up the test file
    std::remove(testFilePath.c_str());
}
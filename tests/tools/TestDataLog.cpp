#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/DataLog.hpp"

#include <nlohmann/json.hpp>

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

TEST_CASE("DataLog calculates mean, median, min, and max correctly", "[DataLog]") {
    cse498::DataLog dataLog;
    
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
    REQUIRE(dataLog.GetMean() == 4.0);
    REQUIRE(dataLog.GetMedian() == 4.0);
    REQUIRE(dataLog.GetMin() == 2.0);
    REQUIRE(dataLog.GetMax() == 6.0);
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
    REQUIRE(dataLog.GetMean() == 0.0);
}
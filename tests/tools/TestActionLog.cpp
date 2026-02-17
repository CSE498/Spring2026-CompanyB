#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/ActionLog.hpp"
#include "../../source/tools/DataLog.hpp"
#include "../../source/tools/GlobalClock.hpp"
#include "../../source/tools/OutputManager.hpp"
#include <nlohmann/json.hpp>

using cse498::ActionLog;
using cse498::DataLog;
using cse498::LogEventStatus;
using cse498::LogLevel;
using nlohmann::json;

// Helper to create a valid event
json make_valid_event() {
    return json{
        {"type", "test_event"},
        {"log_level", LogLevel::Normal},
        {"id", "test_id"},
        {"details", json{{"key", "value"}}}
    };
}

TEST_CASE("ActionLog logs valid events and updates DataLog", "[ActionLog]") {
    DataLog dataLog;
    ActionLog actionLog(dataLog);
    dataLog.Reset();

    auto event = make_valid_event();
    REQUIRE(actionLog.LogEvent(event) == LogEventStatus::SUCCESS);
    REQUIRE(dataLog.GetEntries().size() == 1);
    const auto& logged = dataLog.GetEntries()[0];
    REQUIRE(logged["type"] == "test_event");
    REQUIRE(logged["log_level"] == LogLevel::Normal);
    REQUIRE(logged["id"] == "test_id");
    REQUIRE(logged["details"]["key"] == "value");
}

TEST_CASE("ActionLog rejects events with missing fields", "[ActionLog]") {
    DataLog dataLog;
    ActionLog actionLog(dataLog);
    dataLog.Reset();

    // Missing type
    auto event1 = make_valid_event();
    event1.erase("type");
    REQUIRE(actionLog.LogEvent(event1) == LogEventStatus::FAILURE);
    // Missing log_level
    auto event2 = make_valid_event();
    event2.erase("log_level");
    REQUIRE(actionLog.LogEvent(event2) == LogEventStatus::FAILURE);
    // Missing id
    auto event3 = make_valid_event();
    event3.erase("id");
    REQUIRE(actionLog.LogEvent(event3) == LogEventStatus::FAILURE);
    // Missing details
    auto event4 = make_valid_event();
    event4.erase("details");
    REQUIRE(actionLog.LogEvent(event4) == LogEventStatus::FAILURE);
    // No entries should be logged
    REQUIRE(dataLog.GetEntries().empty());
}

TEST_CASE("ActionLog rejects events with invalid field types or values", "[ActionLog]") {
    DataLog dataLog;
    ActionLog actionLog(dataLog);
    dataLog.Reset();

    // Empty type
    auto event3 = make_valid_event();
    event3["type"] = "";
    REQUIRE(actionLog.LogEvent(event3) == LogEventStatus::FAILURE);
    // Invalid log_level: string instead of integer
    auto event4 = make_valid_event();
    event4["log_level"] = "NOTALEVEL";
    REQUIRE(actionLog.LogEvent(event4) == LogEventStatus::FAILURE);
    // Invalid log_level: integer out of range
    auto event4b = make_valid_event();
    event4b["log_level"] = 999;
    REQUIRE(actionLog.LogEvent(event4b) == LogEventStatus::FAILURE);
    // Details not an object
    auto event5 = make_valid_event();
    event5["details"] = "not an object";
    REQUIRE(actionLog.LogEvent(event5) == LogEventStatus::FAILURE);
    // id is empty string
    auto event6 = make_valid_event();
    event6["id"] = "";
    REQUIRE(actionLog.LogEvent(event6) == LogEventStatus::FAILURE);
    // No entries should be logged
    REQUIRE(dataLog.GetEntries().empty());
}

TEST_CASE("ActionLog can log multiple valid events", "[ActionLog]") {
    DataLog dataLog;
    ActionLog actionLog(dataLog);
    dataLog.Reset();

    for (int i = 1; i <= 3; ++i) {
        auto event = make_valid_event();
        event["test_event"] = std::to_string(i);
        event["id"] = "test_id_" + std::to_string(i);
        REQUIRE(actionLog.LogEvent(event) == LogEventStatus::SUCCESS);
    }
    REQUIRE(dataLog.GetEntries().size() == 3);
    for (int i = 0; i < 3; ++i) {
        REQUIRE(dataLog.GetEntries()[i]["test_event"] == std::to_string(i + 1));
        REQUIRE(dataLog.GetEntries()[i]["id"] == "test_id_" + std::to_string(i + 1));
    }
}

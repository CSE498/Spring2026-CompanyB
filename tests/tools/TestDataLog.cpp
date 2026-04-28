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

TEST_CASE("DataLog handles entry without duration field", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json entryWithoutDuration = {
        {"agentId", "agent_1"},
        {"actionType", "move"},
        {"summary", "No duration"}
    };

    nlohmann::json entryWithDuration = {
        {"agentId", "agent_2"},
        {"actionType", "turn"},
        {"duration", 5.0},
        {"summary", "Has duration"}
    };

    dataLog.AddEntry(entryWithoutDuration);
    REQUIRE(dataLog.GetCount() == 0);  // Entry stored but not in statistics
    REQUIRE(dataLog.GetEntries().size() == 1);  // Entry still in log

    dataLog.AddEntry(entryWithDuration);
    REQUIRE(dataLog.GetCount() == 1);  // Only the one with duration is counted
    REQUIRE(dataLog.GetEntries().size() == 2);  // Both entries in log
}

TEST_CASE("DataLog ignores non-numeric duration field", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json entryWithStringDuration = {
        {"agentId", "agent_1"},
        {"actionType", "move"},
        {"duration", "five seconds"},
        {"summary", "String duration"}
    };

    nlohmann::json entryWithNumericDuration = {
        {"agentId", "agent_2"},
        {"actionType", "turn"},
        {"duration", 5.0},
        {"summary", "Numeric duration"}
    };

    dataLog.AddEntry(entryWithStringDuration);
    REQUIRE(dataLog.GetCount() == 0);  // String duration ignored

    dataLog.AddEntry(entryWithNumericDuration);
    REQUIRE(dataLog.GetCount() == 1);  // Only numeric duration counted
}

TEST_CASE("DataLog calculates median correctly with even number of entries",
"[DataLog]") { cse498::DataLog dataLog;

    // Add 4 entries: [1.0, 2.0, 3.0, 4.0]
    // Median should be average of middle two: (2.0 + 3.0) / 2.0 = 2.5
    dataLog.AddEntry({{"duration", 1.0}});
    dataLog.AddEntry({{"duration", 4.0}});
    dataLog.AddEntry({{"duration", 2.0}});
    dataLog.AddEntry({{"duration", 3.0}});

    REQUIRE(dataLog.GetCount() == 4);
    REQUIRE(dataLog.GetMedian().has_value());
    REQUIRE(dataLog.GetMedian().value() == 2.5);
}

TEST_CASE("DataLog handles zero duration", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json zeroEntry = {
        {"agentId", "agent_1"},
        {"actionType", "instant"},
        {"duration", 0.0},
        {"summary", "Instant action"}
    };

    nlohmann::json normalEntry = {
        {"agentId", "agent_2"},
        {"actionType", "move"},
        {"duration", 5.0},
        {"summary", "Normal action"}
    };

    dataLog.AddEntry(zeroEntry);
    dataLog.AddEntry(normalEntry);

    REQUIRE(dataLog.GetCount() == 2);
    REQUIRE(dataLog.GetMin().has_value());
    REQUIRE(dataLog.GetMin().value() == 0.0);  // Zero is a valid minimum
    REQUIRE(dataLog.GetMax().has_value());
    REQUIRE(dataLog.GetMax().value() == 5.0);
}

TEST_CASE("DataLog ignores negative duration", "[DataLog]") {
    cse498::DataLog dataLog;

    nlohmann::json negativeEntry = {
        {"agentId", "agent_1"},
        {"actionType", "rewind"},
        {"duration", -2.0},
        {"summary", "Negative duration"}
    };

    nlohmann::json positiveEntry = {
        {"agentId", "agent_2"},
        {"actionType", "move"},
        {"duration", 3.0},
        {"summary", "Positive duration"}
    };

    dataLog.AddEntry(negativeEntry);
    REQUIRE(dataLog.GetCount() == 0);  // Negative duration ignored
    REQUIRE(dataLog.GetEntries().size() == 1);  // Entry still stored in log

    dataLog.AddEntry(positiveEntry);
    REQUIRE(dataLog.GetCount() == 1);  // Only positive duration counted
    REQUIRE(dataLog.GetMin().has_value());
    REQUIRE(dataLog.GetMin().value() == 3.0);
    REQUIRE(dataLog.GetMax().has_value());
    REQUIRE(dataLog.GetMax().value() == 3.0);
}

*/
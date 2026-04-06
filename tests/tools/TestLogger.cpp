#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces/IActionLog.hpp"
#include "Interfaces/IDataLog.hpp"
#include "Interfaces/ILogger.hpp"
#include "Interfaces/IOutputManager.hpp"
#include "Interfaces/IReplayDriver.hpp"
#include "tools/Logger.hpp"

namespace cse498 {

// Minimal AgentBase to satisfy AgentConcept without pulling in full engine deps
// This allows us to test Logger isolated from the heavy WorldGrid and Entity
// dependencies Used AI to help with the test cases.
class AgentBase {
 public:
  virtual ~AgentBase() = default;

  std::string id = "0";

  void loadFromJson(const nlohmann::json& /*eventData*/) {}

  const std::vector<ActionEventBase>& GetActions() const { return actions; }

  std::string_view getId() const { return id; }

 private:
  std::vector<ActionEventBase> actions;
};

}  // namespace cse498

namespace cse498 {

// Mock implementation of IActionLog that just returns empty events
// Required because Logger needs an IActionLog, but we don't need its real logic
// here
class MockActionLog : public IActionLog<AgentBase> {
 public:
  std::vector<ActionEventBase> mEventsToReturn;
  std::vector<LogEventFailure> mFailuresToReturn;

  std::pair<std::vector<ActionEventBase>, std::vector<LogEventFailure>>
  LogAgentActions(const std::vector<AgentBase>& /*agents*/) override {
    return {mEventsToReturn, mFailuresToReturn};
  }
};

// Mock output manager that acts as a "spy"
// Instead of writing to a real file, it just remembers what was passed to it
// so we can verify the Logger's behavior later
class MockOutputManager : public IOutputManager {
 public:
  mutable bool mWriteCalled = false;
  mutable std::vector<ActionEventBase> mReceivedEvents;
  mutable std::string mLastSetFile;
  mutable std::vector<std::string> mLoggedMessages;

  void WriteActionEvents(const std::vector<ActionEventBase>& events) override {
    mWriteCalled = true;
    mReceivedEvents = events;
  }

  bool SetOutputFile(const std::string& path) override { 
    mLastSetFile = path;
    return true; 
  }
  void LogMessage(LogLevel /*level*/, const std::string& message) override {
    mLoggedMessages.push_back(message);
  }
  void SetLogLevel(LogLevel /*level*/) noexcept override {}
  bool Flush() override { return true; }
  void WriteSimulationOutput(const DataLog& /*dataLog*/) override {}
  const nlohmann::json& GetBufferedLog() const noexcept override {
    static nlohmann::json j;
    return j;
  }
};

// Mock replay driver to simulate reading replay files
// We can toggle 'should_succeed' to test both the success and error paths of
// the Logger
class MockReplayDriver : public IReplayDriver<AgentBase> {
 public:
  mutable bool mReplayCalled = false;
  mutable std::string mLastFile;
  mutable size_t mLastAgentCount = 0;
  bool mShouldSucceed = true;

  std::expected<bool, std::string> ReplayFromFile(const std::string& filePath,
                      std::vector<AgentBase*>& agents) override {
    mReplayCalled = true;
    mLastFile = filePath;
    mLastAgentCount = agents.size();
    if (mShouldSucceed) {
      return true;
    } else {
      return std::unexpected("Replay failed");
    }
  }
};

}  // namespace cse498

#include "tools/Logger.hpp"

namespace cse498 {

TEST_CASE("Logger - BeginReplay Operations", "[Logger]") {
  auto actionLog = std::make_unique<MockActionLog>();
  auto outputManager = std::make_unique<MockOutputManager>();
  auto replayDriver = std::make_unique<MockReplayDriver>();

  auto* replayPtr = replayDriver.get();

  Logger<AgentBase> logger(std::move(actionLog), std::move(outputManager),
                           std::move(replayDriver));

  std::vector<AgentBase*> agents;

  SECTION("Replay succeeds when driver succeeds") {
    replayPtr->mShouldSucceed = true;
    auto result = logger.BeginReplay("valid_path.json", agents);
    REQUIRE(result);
    REQUIRE(replayPtr->mReplayCalled == true);
    REQUIRE(replayPtr->mLastFile == "valid_path.json");
  }

  SECTION("Replay fails when driver fails") {
    replayPtr->mShouldSucceed = false;
    auto result = logger.BeginReplay("invalid_path.json", agents);
    REQUIRE_FALSE(result);
    REQUIRE(replayPtr->mReplayCalled == true);
    REQUIRE(replayPtr->mLastFile == "invalid_path.json");
  }
}

TEST_CASE("Logger - SaveAgentActions Operations", "[Logger]") {
  auto actionLog = std::make_unique<MockActionLog>();
  auto outputManager = std::make_unique<MockOutputManager>();
  auto replayDriver = std::make_unique<MockReplayDriver>();

  auto* actionLogPtr = actionLog.get();
  auto* outputManagerPtr = outputManager.get();

  Logger<AgentBase> logger(std::move(actionLog), std::move(outputManager),
                           std::move(replayDriver));

  std::vector<AgentBase> agents(2);

  SECTION("Saves with explicit file path") {
    actionLogPtr->mEventsToReturn.resize(1);

    logger.SaveAgentActions(agents, "custom_path.json");

    REQUIRE(outputManagerPtr->mWriteCalled == true);
    REQUIRE(outputManagerPtr->mReceivedEvents.size() == 1);
  }

  SECTION("Saves with empty file path generates default path") {
    logger.SaveAgentActions(agents, "");
    
    REQUIRE(outputManagerPtr->mWriteCalled == true);
    // Verify the auto-generated file starts with "replay_" and ends with ".json"
    REQUIRE(outputManagerPtr->mLastSetFile.find("replay_") == 0);
    REQUIRE(outputManagerPtr->mLastSetFile.find(".json") != std::string::npos);
  }

  SECTION("Logs validation failures when ActionLog returns errors") {
    ActionEventBase dummyEvent{"dummyId", "dummyType", LogLevel::Normal, 0};
    LogEventFailure failure{dummyEvent, "Test validation error"};
    actionLogPtr->mFailuresToReturn.push_back(failure);

    logger.SaveAgentActions(agents, "error_test.json");
    REQUIRE(outputManagerPtr->mLoggedMessages.size() == 1);
    REQUIRE(outputManagerPtr->mLoggedMessages.front().find("Test validation error") != std::string::npos);
  }
}

TEST_CASE("Logger - Stress Test for Memory Bloat", "[Logger][Stress]") {
  auto actionLog = std::make_unique<MockActionLog>();
  // Instantiating the real OutputManager to accurately test memory buffering
  auto outputManager = std::make_unique<OutputManager>();
  auto replayDriver = std::make_unique<MockReplayDriver>();

  auto* actionLogPtr = actionLog.get();

  Logger<AgentBase> logger(std::move(actionLog), std::move(outputManager),
                           std::move(replayDriver));

  std::vector<AgentBase> agents;

  SECTION("Handle 15000+ actions without crashing") {
    // RAII helper to ensure file cleanup even if an assertion throws and aborts the test
    struct FileCleaner {
      std::string path;
      FileCleaner(std::string p) : path(std::move(p)) { std::filesystem::remove(path); }
      ~FileCleaner() { std::filesystem::remove(path); }
    } cleaner("stress_test_log.json");

    constexpr size_t kNumActions = 15000;
    actionLogPtr->mEventsToReturn.resize(kNumActions);
    
    // We expect this to run without throwing std::bad_alloc or crashing
    REQUIRE_NOTHROW(logger.SaveAgentActions(agents, "stress_test_log.json"));

    // Verify the file was created as a result of the Flush() step
    REQUIRE(std::filesystem::exists("stress_test_log.json"));
  }
}

} // namespace cse498

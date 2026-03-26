#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cse498 {

// Minimal AgentBase to satisfy AgentConcept without pulling in full engine deps
// This allows us to test Logger isolated from the heavy WorldGrid and Entity dependencies
// Used AI to help with the test cases.
class AgentBase {
 public:
  virtual ~AgentBase() = default;
  void GetActions() const {}
};

}  // namespace cse498

#include "Interfaces/IActionLog.hpp"
#include "Interfaces/IDataLog.hpp"

namespace cse498 {
using AgentType = AgentBase;
}

#include "Interfaces/ILogger.hpp"
#include "Interfaces/IOutputManager.hpp"
#include "Interfaces/IReplayDriver.hpp"

namespace cse498 {

// Mock implementation of IActionLog that just returns empty events
// Required because Logger needs an IActionLog, but we don't need its real logic here
template <typename T>
class MockActionLog : public IActionLog<T> {
 public:
  std::vector<ActionEventBase> mEventsToReturn;

  std::vector<ActionEventBase> LogAgentActions(
      const std::vector<T>& /*agents*/) override {
    return mEventsToReturn;
  }
};

// Mock output manager that acts as a "spy"
// Instead of writing to a real file, it just remembers what was passed to it
// so we can verify the Logger's behavior later
class MockOutputManager : public IOutputManager {
 public:
  mutable bool mWriteCalled = false;
  mutable std::vector<ActionEventBase> mReceivedEvents;

  void WriteActionEvents(
      const std::vector<ActionEventBase>& events) override {
    mWriteCalled = true;
    mReceivedEvents = events;
  }

  bool SetOutputFile(const std::string& /*path*/) override { return true; }
  void LogMessage(LogLevel /*level*/, const std::string& /*message*/) override {}
  void SetLogLevel(LogLevel /*level*/) noexcept override {}
  bool Flush() override { return true; }
  void WriteSimulationOutput(const DataLog& /*dataLog*/) override {}
  const nlohmann::json& GetBufferedLog() const noexcept override {
    static nlohmann::json j;
    return j;
  }
};

// Mock replay driver to simulate reading replay files
// We can toggle 'should_succeed' to test both the success and error paths of the Logger
class MockReplayDriver : public IReplayDriver {
 public:
  mutable bool mReplayCalled = false;
  mutable std::string mLastFile;
  bool mShouldSucceed = true;

  bool ReplayFromFile(const std::string& filePath) override {
    mReplayCalled = true;
    mLastFile = filePath;
    return mShouldSucceed;
  }
};

}  // namespace cse498

// Hack to inject mocks into Logger and expose private members
#define IActionLog MockActionLog
#define IOutputManager MockOutputManager
#define IReplayDriver MockReplayDriver
#define private public

#include "tools/Logger.hpp"

#undef private
#undef IReplayDriver
#undef IOutputManager
#undef IActionLog

TEST_CASE("Test Logger replay and file saving functionalities", "[Logger]") {
  cse498::Logger logger;

  auto* rawActionLog = logger.mActionLog.get();
  auto* rawOutputManager = logger.mOutputManager.get();
  auto* rawReplayDriver = logger.mReplayDriver.get();

  SECTION("Test BeginReplay with a valid driver that succeeds") {
    // configure our mock to simulate a successful file read
    rawReplayDriver->mShouldSucceed = true;
    
    // logger should bubble up the true result
    REQUIRE(logger.BeginReplay("test.json"));

    // verify the logger actually tried to replay from the driver
    REQUIRE(rawReplayDriver->mReplayCalled);
    REQUIRE(rawReplayDriver->mLastFile == "test.json");
  }

  SECTION("Test BeginReplay with a failing driver") {
    // configure our mock to simulate a failed file read
    rawReplayDriver->mShouldSucceed = false;

    // logger should return false if the underlying driver fails
    REQUIRE_FALSE(logger.BeginReplay("test.json"));
    REQUIRE(rawReplayDriver->mReplayCalled);
  }

  SECTION("Test BeginReplay when no driver was provided") {
    // simulate a missing replay driver to test null safety
    logger.mReplayDriver.reset();

    // should safely return false without crashing
    REQUIRE_FALSE(logger.BeginReplay("test.json"));
  }

  SECTION("Test ExtractAgentActions successfully writes to the output manager") {
    std::vector<cse498::ActionEventBase> events = {
        {"agent1", "move", cse498::LogLevel::Normal, 123}};

    rawActionLog->mEventsToReturn = events;

    std::vector<cse498::AgentBase> agents;
    // logger should successfully extract and write
    logger.ExtractAgentActions(agents);

    // verify the output manager received our events
    REQUIRE(rawOutputManager->mWriteCalled);
    REQUIRE(rawOutputManager->mReceivedEvents.size() == 1);
    REQUIRE(rawOutputManager->mReceivedEvents[0].agentId == "agent1");
  }
}
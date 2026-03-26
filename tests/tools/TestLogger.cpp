#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Interfaces/IActionLog.hpp"
#include "Interfaces/IDataLog.hpp"
#include "Interfaces/IOutputManager.hpp"
#include "Interfaces/IReplayDriver.hpp"
#include "tools/Logger.hpp"

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

// Mock implementation of IActionLog that just returns empty events
// Required because Logger needs an IActionLog, but we don't need its real logic here
class MockActionLog : public cse498::IActionLog<cse498::AgentBase> {
 public:
  std::vector<cse498::ActionEventBase> LogAgentActions(
      const std::vector<cse498::AgentBase>& /*agents*/) override {
    return {};
  }
};

// Mock output manager that acts as a "spy"
// Instead of writing to a real file, it just remembers what was passed to it
// so we can verify the Logger's behavior later
class MockOutputManager : public cse498::IOutputManager {
 public:
  mutable bool mWriteCalled = false;
  mutable std::vector<cse498::ActionEventBase> mReceivedEvents;

  void WriteActionEvents(
      const std::vector<cse498::ActionEventBase>& events) override {
    mWriteCalled = true;
    mReceivedEvents = events;
  }

  bool SetOutputFile(const std::string& /*path*/) override { return true; }
  void LogMessage(cse498::LogLevel /*level*/, const std::string& /*message*/) override {}
  void SetLogLevel(cse498::LogLevel /*level*/) noexcept override {}
  bool Flush() override { return true; }
  void WriteSimulationOutput(const cse498::DataLog& /*dataLog*/) override {}
  const nlohmann::json& GetBufferedLog() const noexcept override {
    static nlohmann::json j;
    return j;
  }
};

// Mock replay driver to simulate reading replay files
// We can toggle 'should_succeed' to test both the success and error paths of the Logger
class MockReplayDriver : public cse498::IReplayDriver {
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

TEST_CASE("Test Logger replay and file saving functionalities", "[Logger]") {
  // Create our mock dependencies
  auto actionLog = std::make_unique<MockActionLog>();
  auto outputManager = std::make_unique<MockOutputManager>();
  auto replayDriver = std::make_unique<MockReplayDriver>();

  // Keep raw pointers to our mocks before we give ownership to the Logger.
  // This is crucial because std::move will clear our unique_ptrs, and we still 
  // need a way to check if the Logger actually called the right methods on them.
  auto* rawOutputManager = outputManager.get();
  auto* rawReplayDriver = replayDriver.get();

  // Pass ownership of the mocks to the Logger
  cse498::Logger logger(std::move(actionLog), std::move(outputManager),
                        std::move(replayDriver));

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
    // create a new logger without a replay driver to test null safety
    cse498::Logger noReplayLogger(std::make_unique<MockActionLog>(),
                                    std::make_unique<MockOutputManager>(), nullptr);

    // should safely return false without crashing
    REQUIRE_FALSE(noReplayLogger.BeginReplay("test.json"));
  }

  SECTION("Test SaveToFile successfully writes to the output manager") {
    std::vector<cse498::ActionEventBase> events = {
        {"agent1", "move", cse498::LogLevel::Normal, 123}};

    // logger should return true on successful write
    REQUIRE(logger.SaveToFile(events));

    // verify the output manager received our events
    REQUIRE(rawOutputManager->mWriteCalled);
    REQUIRE(rawOutputManager->mReceivedEvents.size() == 1);
    REQUIRE(rawOutputManager->mReceivedEvents[0].agentId == "agent1");
  }

  SECTION("Test SaveToFile when no output manager was provided") {
    // create a new logger without an output manager to test null safety
    cse498::Logger noOutputLogger(std::make_unique<MockActionLog>(), nullptr, nullptr);

    // should safely return false without crashing
    REQUIRE_FALSE(noOutputLogger.SaveToFile({{}}));
  }
}
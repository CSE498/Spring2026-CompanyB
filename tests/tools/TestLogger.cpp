#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Interfaces/IActionLog.hpp"
#include "Interfaces/IDataLog.hpp"
#include "Interfaces/ILogger.hpp"
#include "Interfaces/IOutputManager.hpp"
#include "Interfaces/IReplayDriver.hpp"

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

  std::vector<ActionEventBase> LogAgentActions(
      const std::vector<AgentBase>& /*agents*/) override {
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

  void WriteActionEvents(const std::vector<ActionEventBase>& events) override {
    mWriteCalled = true;
    mReceivedEvents = events;
  }

  bool SetOutputFile(const std::string& /*path*/) override { return true; }
  void LogMessage(LogLevel /*level*/, const std::string& /*message*/) override {
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

  bool ReplayFromFile(const std::string& filePath,
                      std::vector<AgentBase*>& agents) override {
    mReplayCalled = true;
    mLastFile = filePath;
    mLastAgentCount = agents.size();
    return mShouldSucceed;
  }
};

}  // namespace cse498

#include "tools/Logger.hpp"

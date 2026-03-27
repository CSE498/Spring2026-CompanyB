/**
 * @file Group20-DRIVER.cpp
 * @brief Demo for two logger workflows using the SAME agent type:
 * 1) save all typed action events to JSON, and
 * 2) load/replay those action events from JSON.
 **/

// Compile with:
// g++ -std=c++23 -Wall -Wextra -Wpedantic ./demo/Group20-DRIVER.cpp -I third-party/include -I ./source/ -o build/native/group20_demo
// Run with:
// ./build/native/group20_demo

#include "../source/tools/ActionLog.hpp"
#include "../source/tools/ReplayDriver.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "nlohmann/json.hpp"

using namespace cse498;

namespace {


/// @brief Example action details struct for movement actions
struct MovementDetails {
  std::string direction;
  int dx = 0;
  int dy = 0;
};

using MovementEvent = ActionEvent<MovementDetails>;

/**
  * @brief DemoAgent is a simple agent class that can log movement actions and
  * replay them from JSON data. It serves as an example of how the real agents
  * class should be implemente to work with Logger.
  */
class DemoAgent {
 private:
  int mId;
  std::string_view mAgentLabel;
  std::vector<MovementEvent> mActions;

 public:
  DemoAgent(int id, std::string_view label) : mId(id), mAgentLabel(label) {}

  void AddMovementAction(std::string_view direction, int dx, int dy,
                         uint64_t tick, LogLevel level = LogLevel::Normal) {
    mActions.push_back({mAgentLabel, "movement", level, tick,
                        MovementDetails{std::string(direction), dx, dy}});
  }

  void PrintActions() const {
    std::cout << "   Agent " << mId << " recorded " << mActions.size()
              << " actions\n";
    for (const auto& action : mActions) {
      std::cout << "      tick=" << action.timestamp
                << " type='" << action.actionType << "'"
                << " dir='" << action.details.direction << "'"
                << " delta=(" << action.details.dx << ", "
                << action.details.dy << ")\n";
    }
  }

  // --------------------------------------------------------------------
  // The following functions are required for compatibility with Logger.
  // --------------------------------------------------------------------
  int getId() const { return mId; }

  const std::vector<MovementEvent>& GetActions() const { return mActions; }

  // ReplayDriver passes one event object at a time.
  void loadFromJson(const nlohmann::json& eventData) {
    if (!eventData.contains("actionType") ||
        !eventData.at("actionType").is_string()) {
      return;
    }
    if (eventData.at("actionType").get<std::string>() != "movement") {
      return;
    }
    if (!eventData.contains("timestamp") ||
        !eventData.at("timestamp").is_number_unsigned()) {
      return;
    }
    if (!eventData.contains("logLevel") ||
        !eventData.at("logLevel").is_number_integer()) {
      return;
    }
    if (!eventData.contains("details") || !eventData.at("details").is_object()) {
      return;
    }

    const auto& details = eventData.at("details");
    if (!details.contains("direction") || !details.at("direction").is_string() ||
        !details.contains("dx") || !details.at("dx").is_number_integer() ||
        !details.contains("dy") || !details.at("dy").is_number_integer()) {
      return;
    }

    const int levelRaw = eventData.at("logLevel").get<int>();
    if (levelRaw < static_cast<int>(LogLevel::Normal) ||
        levelRaw > static_cast<int>(LogLevel::Silent)) {
      return;
    }

    AddMovementAction(details.at("direction").get<std::string>(),
                      details.at("dx").get<int>(),
                      details.at("dy").get<int>(),
                      eventData.at("timestamp").get<uint64_t>(),
                      static_cast<LogLevel>(levelRaw));
  }
};

bool SaveAgentsToFile(const std::filesystem::path& outputPath,
                      const std::vector<DemoAgent>& agents) {
  nlohmann::json events = nlohmann::json::array();
  for (const auto& agent : agents) {
    for (const auto& action : agent.GetActions()) {
      events.push_back({
          {"id", agent.getId()},
          {"agentId", std::string(action.agentId)},
          {"actionType", std::string(action.actionType)},
          {"logLevel", static_cast<int>(action.logLevel)},
          {"timestamp", action.timestamp},
          {"details",
           {{"direction", action.details.direction},
            {"dx", action.details.dx},
            {"dy", action.details.dy}}},
      });
    }
  }

  std::ofstream out(outputPath);
  if (!out.is_open()) {
    return false;
  }

  out << events.dump(2) << '\n';
  return !out.fail();
}

bool RunSaveScenario(const std::filesystem::path& outputPath) {
  std::cout << "=== SCENARIO 1: SAVE AGENT ACTIONS ===\n";

  std::vector<DemoAgent> agents;
  agents.emplace_back(1, "agent-1");
  agents.emplace_back(2, "agent-2");

  agents[0].AddMovementAction("east", 1, 0, 1001, LogLevel::Normal);
  agents[0].AddMovementAction("north", 0, 1, 1002, LogLevel::Verbose);
  agents[1].AddMovementAction("west", -1, 0, 2001, LogLevel::Normal);
  agents[1].AddMovementAction("south", 0, -1, 2002, LogLevel::Debug);

  // Validate and flatten via ActionLog, following the logger extraction flow.
  ActionLog<DemoAgent> actionLog;
  const std::vector<ActionEventBase> flattened = actionLog.LogAgentActions(agents);

  const bool saveOk = SaveAgentsToFile(outputPath, agents);
  const auto bytes = saveOk ? std::filesystem::file_size(outputPath) : 0;

  std::cout << "   Flattened events via ActionLog: " << flattened.size() << "\n";
  std::cout << "   Save status: " << (saveOk ? "success" : "failure") << "\n";
  std::cout << "   Output file: " << outputPath << " (" << bytes
            << " bytes)\n\n";

  return saveOk;
}

bool RunLoadScenario(const std::filesystem::path& replayPath) {
  std::cout << "=== SCENARIO 2: LOAD/REPLAY EVENTS ===\n";

  std::vector<DemoAgent> agents;
  agents.emplace_back(1, "agent-1");
  agents.emplace_back(2, "agent-2");
  std::vector<DemoAgent*> agentPointers{&agents[0], &agents[1]};

  ReplayDriver<DemoAgent> replayDriver;
  const auto replayResult = replayDriver.ReplayFromFile(replayPath.string(), agentPointers);

  if (!replayResult.has_value()) {
    std::cout << "   Replay failed: " << replayResult.error() << "\n\n";
    return false;
  }

  std::cout << "   Replay status: "
            << (replayResult.value() ? "success" : "failure") << "\n";
  agents[0].PrintActions();
  agents[1].PrintActions();
  std::cout << "   Replay input file: " << replayPath << "\n\n";

  const bool expectedLoaded =
      replayResult.value() && agents[0].GetActions().size() == 2 &&
      agents[1].GetActions().size() == 2;
  return expectedLoaded;
}

bool CleanupFile(const std::filesystem::path& filePath) {
  std::error_code ec;
  const bool removed = std::filesystem::remove(filePath, ec);
  if (ec) {
    std::cerr << "Cleanup warning: failed to remove file '" << filePath
              << "': " << ec.message() << '\n';
    return false;
  }
  if (removed) {
    std::cout << "   Cleanup: removed " << filePath << "\n";
  }

  const auto dir = filePath.parent_path();
  if (!dir.empty()) {
    std::error_code dirEc;
    if (std::filesystem::exists(dir, dirEc) && std::filesystem::is_empty(dir, dirEc)) {
      std::filesystem::remove(dir, dirEc);
    }
  }
  return true;
}

}  // namespace

int main() {
  namespace fs = std::filesystem;

  const fs::path logsDir = "logs";
  const fs::path saveAndReplayPath = logsDir / "group20_action_events_demo.json";

  std::error_code ec;
  fs::create_directories(logsDir, ec);
  if (ec) {
    std::cerr << "Failed to create logs directory: " << ec.message() << '\n';
    return 1;
  }

  const bool saveOk = RunSaveScenario(saveAndReplayPath);
  const bool loadOk = saveOk ? RunLoadScenario(saveAndReplayPath) : false;

  std::cout << "=== CLEANUP ===\n";
  const bool cleanupOk = CleanupFile(saveAndReplayPath);

  std::cout << "=== SUMMARY ===\n";
  std::cout << "   Save scenario: " << (saveOk ? "PASS" : "FAIL") << '\n';
  std::cout << "   Load scenario: " << (loadOk ? "PASS" : "FAIL") << '\n';
  std::cout << "   Cleanup: " << (cleanupOk ? "PASS" : "FAIL") << '\n';

  return (saveOk && loadOk && cleanupOk) ? 0 : 1;
}

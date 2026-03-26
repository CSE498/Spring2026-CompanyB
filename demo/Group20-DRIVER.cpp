/**
 * This file is a demo of the logger module.
 * @brief Demo for logger module
 **/

#include "../source/tools/ActionLog.hpp"
#include "../source/tools/OutputManager.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "nlohmann/json.hpp"

using namespace cse498;

class MockAgent {
 public:
  // This method does not need to be implemented in the real Agent class, just
  //  for demo purposes
  void AddAction(std::string_view agentId, std::string_view actionType,
                 LogLevel level, uint64_t timestamp) {
    actions.push_back({agentId, actionType, level, timestamp});
  }
  void setId(int newId) { id = newId; }

  // Replay loader for outputmanager format: { "action_events": [...] }
  void loadFromJson(const nlohmann::json& eventData) {
    replayed_count = 0;
    first_replayed_agent_id.clear();

    if (!eventData.contains("action_events") ||
        !eventData.at("action_events").is_array()) {
      return;
    }

    const auto& arr = eventData.at("action_events");
    replayed_count = static_cast<int>(arr.size());
    if (!arr.empty() && arr[0].contains("agentId")) {
      first_replayed_agent_id = arr[0].at("agentId").get<std::string>();
    }
  }
  const std::vector<ActionEventBase>& GetActions() const { return actions; }
  int getId() const { return id; }
  int GetReplayedCount() const { return replayed_count; }
  const std::string& GetFirstReplayedAgentId() const {
    return first_replayed_agent_id;
  }

 private:
  std::vector<ActionEventBase> actions;
  int id = 0;
  int replayed_count = 0;
  std::string first_replayed_agent_id;
};

int main() {
  ActionLog<MockAgent> actionLog;
  OutputManager outputManager(LogLevel::Verbose);

  const std::string outputPath = "logs/simulation_log.json";
  outputManager.SetOutputFile(outputPath);

  MockAgent agent;
  agent.AddAction("agent1", "move", LogLevel::Normal, 123456789);
  agent.AddAction("agent1", "attack", LogLevel::Debug, 123456790);

  std::vector<MockAgent> agents{agent};

  std::cout << "1) Extracting agent actions...\n";
  const std::vector<ActionEventBase> events = actionLog.LogAgentActions(agents);
  std::cout << "   Extracted " << events.size() << " events\n";

  std::cout << "2) Writing extracted actions to file...\n";
  outputManager.WriteActionEvents(events);
  const bool wrote = outputManager.Flush();
  std::cout << "   Write status: " << (wrote ? "true" : "false") << "\n";
  std::cout << "   Output file: " << outputPath << "\n";

  std::cout << "3) Enter replay file path (press Enter for default): ";
  std::string replayFile;
  std::getline(std::cin, replayFile);
  if (replayFile.empty()) {
    replayFile = outputPath;
  }

  bool replayOk = false;
  std::ifstream in(replayFile);
  if (in.is_open()) {
    nlohmann::json replayJson = nlohmann::json::parse(in, nullptr, false);
    if (!replayJson.is_discarded()) {
      // This proves we actually read from the provided replay file.
      agent.loadFromJson(replayJson);
      replayOk = true;
    }
  }

  std::cout << "4) Replay read result: " << (replayOk ? "true" : "false")
            << "\n";
  if (replayOk) {
    std::cout << "   Replayed action count: " << agent.GetReplayedCount()
              << "\n";
    std::cout << "   First replayed agent id: "
              << agent.GetFirstReplayedAgentId()
              << "\n";
  }

  // Clean up the output file
  std::cout << "\n5) Cleaning up...\n";
  std::error_code ec;
  size_t removed = std::filesystem::remove(outputPath, ec);
  std::cout << "   File removed: " << (removed > 0 ? "true" : "false") << "\n";

  return 0;
}

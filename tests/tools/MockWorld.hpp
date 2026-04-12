#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace cse498 {
class MockAgent {
 public:
  std::string id = "5";
  std::vector<int> position = {1, 2};
  std::string etc = "test value";

  MockAgent() = default;
  ~MockAgent() = default;

  void getLoggable() {
    nlohmann::json eventData;
    eventData["agentId"] = id;
    eventData["position"] = position;
    eventData["etc"] = etc;
    std::ofstream outFile("test_events.json");
    if (outFile.is_open()) {
        outFile << eventData.dump();
        outFile.close();
    }

    position.clear();
    etc = "";
  }
  void loadFromJson(const nlohmann::json& eventData) {
    if (eventData.contains("agentId")) {
        id = eventData.at("agentId").get<std::string>();
    }
    if (eventData.contains("position")) {
        position = eventData.at("position").get<std::vector<int>>();
    }
    if (eventData.contains("etc")) {
        etc = eventData.at("etc").get<std::string>();
    }
  }
  std::string getId() const { return id; }
};

class MockWorld {
 public:
  std::vector<MockAgent*> agents;

  MockWorld() = default;
  ~MockWorld() = default;

  MockAgent* getAgent(const std::string& id) {
    for (auto& agent : agents) {
      if (agent->id == id) {
        return agent;
      }
    }
    return nullptr;
  }

  std::vector<MockAgent*>& getAgents() { return agents; }
  const std::vector<MockAgent*>& getAgents() const { return agents; }

  std::vector<MockAgent*>& getReplayAgents() { return agents; }
};
}  // namespace cse498

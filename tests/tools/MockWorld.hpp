#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace cse498 {
class MockAgent {
 public:
  int id = 5;
  std::vector<int> position = {1, 2};
  std::string etc = "test value";

  MockAgent() = default;
  ~MockAgent() = default;

  void getLoggable() {
    nlohmann::json eventData;
    eventData["id"] = id;
    eventData["position"] = position;
    eventData["etc"] = etc;
    std::ofstream outFile("test_events.json");
    outFile << eventData.dump();
    outFile.close();

    position.clear();
    etc = "";
  }
  void loadFromJson(const nlohmann::json& eventData) {
    id = eventData.at("id");
    position = eventData.at("position").get<std::vector<int>>();
    etc = eventData.at("etc").get<std::string>();
  }
};

class MockWorld {
 public:
  std::vector<MockAgent*> agents;

  MockWorld() = default;
  ~MockWorld() = default;

  MockAgent* getAgent(int id) {
    for (auto& agent : agents) {
      if (agent->id == id) {
        return agent;
      }
    }
    return nullptr;
  }

  std::vector<MockAgent*>& getAgents() { return agents; }
};
}  // namespace cse498

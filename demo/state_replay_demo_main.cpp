#include "../source/core/AgentData.hpp"
#include "../source/core/StepAgentBase.hpp"
#include "../source/tools/Logger.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace cse498 {

class DemoInfectionAgent : public StepAgentBase<DiseaseData> {
public:
  DemoInfectionAgent(const DiseaseData &data, std::size_t id,
                     uint64_t initialTick = 0)
      : StepAgentBase<DiseaseData>(data, id, LogLevel::Normal, initialTick) {}

  [[nodiscard]] steps::StepContainer GetTurn() override { return {}; }

  void SetGoal(WorldPosition position) override {
    DiseaseData next = GetState();
    next.destination = position;
    SetStateNoLog(next);
  }
};

static const char *HealthStateName(HealthState state) {
  switch (state) {
  case HealthState::SUSCEPTIBLE:
    return "SUSCEPTIBLE";
  case HealthState::INFECTED:
    return "INFECTED";
  case HealthState::RECOVERED:
    return "RECOVERED";
  }
  return "UNKNOWN";
}

static void PrintAgentStates(const std::vector<DemoInfectionAgent> &agents,
                             const std::string &title) {
  std::cout << title << '\n';
  for (const auto &agent : agents) {
    const DiseaseData state = agent.GetState();
    std::cout << "  agent " << agent.getId() << " => "
              << HealthStateName(state.infection_state)
              << ", infection_probability=" << state.infection_probability
              << '\n';
  }
}

} // namespace cse498

int main() {
  using namespace cse498;
  namespace fs = std::filesystem;

  std::vector<DemoInfectionAgent> liveAgents;
  liveAgents.emplace_back(DiseaseData{0.20, HealthState::SUSCEPTIBLE,
                                      std::nullopt, WorldPosition{0, 0}},
                          1, 0);
  liveAgents.emplace_back(DiseaseData{0.85, HealthState::INFECTED, std::nullopt,
                                      WorldPosition{1, 0}},
                          2, 0);

  // Record two simulation ticks using one-hot disease-state JSON in details.
  DiseaseData a1Tick1 = liveAgents[0].GetState();
  a1Tick1.infection_state = HealthState::INFECTED;
  liveAgents[0].SetState(a1Tick1, LogLevel::Normal, 1);

  DiseaseData a2Tick1 = liveAgents[1].GetState();
  a2Tick1.infection_state = HealthState::RECOVERED;
  liveAgents[1].SetState(a2Tick1, LogLevel::Normal, 1);

  DiseaseData a1Tick2 = liveAgents[0].GetState();
  a1Tick2.infection_state = HealthState::RECOVERED;
  liveAgents[0].SetState(a1Tick2, LogLevel::Normal, 2);

  Logger<DemoInfectionAgent> saveLogger;
  const auto saveResult = saveLogger.SaveAgentActions(liveAgents);
  if (!saveResult.has_value()) {
    std::cerr << "Save failed.\n";
    return 1;
  }

  const fs::path replayFile =
      fs::current_path() / "logs" / "simulation_log.json";
  if (!fs::exists(replayFile)) {
    std::cerr << "Expected replay file missing: " << replayFile << '\n';
    return 1;
  }
  std::cout << "Saved state history to: " << replayFile << "\n\n";
  PrintAgentStates(liveAgents, "Live agents (after save):");

  // Start with intentionally different states, then replay from file.
  std::vector<DemoInfectionAgent> replayAgents;
  replayAgents.emplace_back(DiseaseData{0.01, HealthState::SUSCEPTIBLE,
                                        std::nullopt, WorldPosition{9, 9}},
                            1, 0);
  replayAgents.emplace_back(DiseaseData{0.01, HealthState::SUSCEPTIBLE,
                                        std::nullopt, WorldPosition{9, 8}},
                            2, 0);

  std::vector<DemoInfectionAgent *> replayPointers{&replayAgents[0],
                                                   &replayAgents[1]};
  Logger<DemoInfectionAgent> replayLogger;
  const auto replayLoadResult =
      replayLogger.BeginReplay(replayFile.string(), replayPointers);
  if (!replayLoadResult.has_value()) {
    std::cerr << "Replay load failed: " << replayLoadResult.error() << '\n';
    return 1;
  }

  // Simulate world ticks: SetState uses replay mode and applies state from
  // mStates.
  for (uint64_t tick = 0; tick <= 2; ++tick) {
    replayAgents[0].SetState(replayAgents[0].GetState(), LogLevel::Normal,
                             tick);
    replayAgents[1].SetState(replayAgents[1].GetState(), LogLevel::Normal,
                             tick);
  }

  std::cout << '\n';
  PrintAgentStates(replayAgents, "Replay agents (after ticks 0..2):");

  const bool ok =
      replayAgents[0].GetState().infection_state == HealthState::RECOVERED &&
      replayAgents[1].GetState().infection_state == HealthState::RECOVERED;
  std::cout << "\nReplay verification: " << (ok ? "PASS" : "FAIL") << '\n';
  return ok ? 0 : 1;
}

/**
 * @file ReplayDriver.hpp
 * @author Apar Mohabansi
 **/

#pragma once

#include <algorithm>
#include <concepts>
#include <expected>
#include <fstream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace cse498 {

// Concept: Ensures an agent pointer supports replay operations.
template <typename AgentT>
concept ReplayAgent = requires(AgentT* agent, const nlohmann::json& eventData) {
  { agent->loadFromJson(eventData) };
  { agent->getId() };
};

/* Class Description:
 *  ReplayDriver is responsible for replaying logged events from a JSON file.
 *  It reads the events and sends instructions to matching agents by id.
 */

template <ReplayAgent AgentT>
class ReplayDriver {
 public:
  ReplayDriver() = default;
  ~ReplayDriver() = default;

  /// @brief Method to replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @param agents Reference vector of replayable agents.
  /// @return Success status of the replay operation. True if successful and
  /// SendInstructions is called, false otherwise.
  std::expected<bool, std::string> ReplayFromFile(
      const std::string& filePath, std::vector<AgentT*>& agents) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
      return std::unexpected("Failed to open file" + filePath);
    }

    nlohmann::json eventData;
    try {
      inFile >> eventData;
    } catch (const nlohmann::json::parse_error& e) {
      return std::unexpected("Failed to parse JSON: " + std::string(e.what()));
    }

    if (eventData.is_array()) {
      for (const auto& event : eventData) {
        if (!SendInstructions(event, agents)) {
          return std::unexpected("Failed to replay event: agent not found");
        }
      }
      return true;
    }

    if (!SendInstructions(eventData, agents)) {
      return std::unexpected("Failed to replay event: agent not found");
    }

    return true;
  }
  /// @brief Method to send instructions to a matching agent.
  /// @param eventData JSON data containing the events to be replayed.
  /// @param agents Reference vector of replayable agents.
  /// @return true when a matching agent is updated, false otherwise.
  bool SendInstructions(const nlohmann::json& eventData,
                        std::vector<AgentT*>& agents) {
    if (!eventData.contains("id") || !eventData.at("id").is_number_integer()) {
      return false;
    }

    const int targetId = eventData.at("id").get<int>();
    const auto found = std::ranges::find_if(agents, [targetId](AgentT* agent) {
      return agent != nullptr && agent->getId() == targetId;
    });

    if (found == agents.end()) {
      return false;
    }

    (*found)->loadFromJson(eventData);
    return true;
  }
};
}  // namespace cse498

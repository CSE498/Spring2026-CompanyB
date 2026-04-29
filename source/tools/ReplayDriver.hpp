/**
 * @file ReplayDriver.hpp
 * @author Apar Mohabansi
 **/

#pragma once

#include <algorithm>
#include <concepts>
#include <expected>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../Interfaces/IReplayDriver.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

// Concept: Ensures an agent pointer supports replay operations.
template <typename AgentT>
concept ReplayAgent = requires(AgentT* agent, const nlohmann::json& eventData) {
  { agent->loadFromJson(eventData) };
} && (requires(AgentT* agent) {
                        { agent->getId() };
                      });

/// @brief ReplayDriver is responsible for replaying logged events from a JSON
/// file. It reads the events and sends instructions to matching agents by id.
template <ReplayAgent AgentT>
class ReplayDriver : public IReplayDriver<AgentT> {
 public:
  ReplayDriver() = default;
  ~ReplayDriver() override = default;

  std::expected<std::unordered_map<int, std::string>, std::string> AgentMap(
      const std::string& filePath) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
      return std::unexpected("Failed to open file");
    }
    std::unordered_map<int, std::string> agentMap;
    nlohmann::json eventData;
    try {
      inFile >> eventData;
      if (eventData.is_array()) {
        agentMap.reserve(eventData.size());
        for (const auto& event : eventData) {
          if (event.contains("agentId") && event.contains("agentType")) {
            int agentId = std::stoi(event["agentId"].get<std::string>());
            std::string agentType = event["agentType"].get<std::string>();
            std::cout << "Parsed agentId: " << agentId
                      << ", agentType: " << agentType << std::endl;
            agentMap.emplace(agentId, agentType);
          }
        }
      } else if (eventData.is_object()) {
        if (eventData.contains("agentId") &&
            eventData["agentId"].is_number_integer() &&
            eventData.contains("agentType") &&
            eventData["agentType"].is_string()) {
          int agentId = eventData["agentId"].get<int>();
          std::string agentType = eventData["agentType"].get<std::string>();
          agentMap.emplace(agentId, agentType);
        }
      }
      for (const auto& [id, type] : agentMap) {
        std::cout << "Parsed agentId: " << id << ", agentType: " << type
                  << std::endl;
      }
      return agentMap;
    } catch (..., const nlohmann::json::parse_error& e) {
      return std::unexpected("Failed to parse JSON" + std::string(e.what()));
    }
  }

  /// @brief Method to replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @param agents Reference vector of replayable agents.
  /// @return true if replay was successful, otherwise an error describing
  /// replay failure.
  std::expected<bool, std::string> ReplayFromFile(
      const std::string& filePath, std::vector<AgentT*>& agents) override {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
      return std::unexpected("Failed to open file");
    }

    nlohmann::json eventData;
    try {
      inFile >> eventData;
    } catch (..., const nlohmann::json::parse_error& e) {
      return std::unexpected("Failed to parse JSON" + std::string(e.what()));
    }

    // Accept either a top-level array of events or the OutputManager shape:
    // { "action_events": [ ... ] }.
    const nlohmann::json* eventsToReplay = &eventData;
    if (eventData.is_object() && eventData.contains("action_events")) {
      eventsToReplay = &eventData.at("action_events");
      if (!eventsToReplay->is_array()) {
        return std::unexpected("Invalid event format");
      }
    }

    if (eventsToReplay->is_array()) {
      for (const auto& event : *eventsToReplay) {
        if (!SendInstructions(event, agents)) {
          return std::unexpected("Failed to send instructions");
        }
      }
      return true;
    }

    if (!eventsToReplay->is_object()) {
      return std::unexpected("Invalid event format");
    }

    if (!SendInstructions(*eventsToReplay, agents)) {
      return std::unexpected("Failed to send instructions");
    }

    return true;
  }
  /// @brief Method to send instructions to a matching agent.
  /// @param eventData JSON data containing the events to be replayed.
  /// @param agents Reference vector of replayable agents.
  /// @return true when a matching agent is updated, false otherwise.
  bool SendInstructions(const nlohmann::json& eventData,
                        std::vector<AgentT*>& agents) {
    const auto GetIdString =
        [&eventData](const char* key) -> std::optional<std::string> {
      if (!eventData.contains(key)) {
        return std::nullopt;
      }
      const nlohmann::json& idField = eventData.at(key);
      if (idField.is_string()) {
        return idField.get<std::string>();
      }
      if (idField.is_number_integer()) {
        return std::to_string(idField.get<long long>());
      }
      return std::nullopt;
    };

    const std::optional<std::string> parsedAgentId = GetIdString("agentId");
    const std::optional<std::string> parsedId = GetIdString("id");
    if (!parsedAgentId.has_value() && !parsedId.has_value()) {
      return false;
    }
    const std::string targetId =
        parsedAgentId.has_value() ? parsedAgentId.value() : parsedId.value();

    const auto found = std::ranges::find_if(agents, [&targetId](AgentT* agent) {
      if (agent == nullptr) {
        return false;
      }

      const auto agentId = agent->getId();
      using IdType = std::decay_t<decltype(agentId)>;

      if constexpr (std::is_arithmetic_v<IdType>) {
        return std::to_string(agentId) == targetId;
      } else if constexpr (std::is_convertible_v<IdType, std::string_view>) {
        return std::string_view(agentId) == targetId;
      } else {
        return false;
      }
    });

    if (found == agents.end()) {
      return false;
    }

    (*found)->loadFromJson(eventData);
    return true;
  }
};
}  // namespace cse498

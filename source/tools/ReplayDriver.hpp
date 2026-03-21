/**
 * @file ReplayDriver.hpp
 * @author Apar Mohabansi
 **/

#pragma once

#include <concepts>
#include <expected>
#include <memory>

#include "../../tests/tools/MockWorld.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

// Concept: Ensures a type has a getLoggable() method
template <typename A>
concept IsLoggableAgent = requires(A a) {
  { a->getLoggable() };
};

// Concept: Ensures a type has a getAgents() method returning a range of
// IsLoggableAgents
template <typename W>
concept ValidWorld =
    requires(W w) {
      // Ensures getAgents() exists and returns something iterable (like
      // std::vector)
      { w.getAgents() } -> std::ranges::range;
    } &&
    // Ensures the elements inside that iterable satisfy IsLoggableAgent
    IsLoggableAgent<
        std::ranges::range_value_t<decltype(std::declval<W>().getAgents())>>;

/* Class Description:
 *  ReplayDriver is responsible for replaying logged events from a JSON file.
 *  It reads the events and sends instructions to the world based on the event
 * data. Citation: Used AI responsibly and actively in building the class below.
 */

template <ValidWorld WorldT>
class ReplayDriver {
 private:
  // world* world; need a pointer to the world object to instuct addtions and
  std::shared_ptr<WorldT> world;  // This is a placeholder. Replace with actual
                                  // world class pointer when available.
 public:
  /// @brief Basic constructor for the ReplayDriver.
  ReplayDriver(std::shared_ptr<WorldT> world) : world(world){};
  ~ReplayDriver() = default;

  /// @brief Method to replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return Success status of the replay operation. True if successful and
  /// SendInstructions is called, false otherwise.
  std::expected<bool, std::string> ReplayFromFile(const std::string &filePath) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
      return std::unexpected("Failed to open file" + filePath);
    }

    nlohmann::json eventData;
    try {
      inFile >> eventData;
    } catch (const nlohmann::json::parse_error &e) {
      return std::unexpected("Failed to parse JSON: " + std::string(e.what()));
    }

    SendInstructions(eventData);

    // Here we would add logic to process the eventData and replay the events.
    // For now, we will just print the loaded JSON data.
    // std::cout << "Loaded event data: " << eventData.dump(4) << std::endl;

    return true;
  }
  /// @brief Method to send instructions to the world based on the event data.
  /// @param eventData JSON data containing the events to be replayed.
  void SendInstructions(const nlohmann::json &eventData) {
    // This function would contain logic to send instructions to the world based
    // on the event data. For now, we will pretend to send it to the world
    // Here we would have logic to interpret the event and send instructions to
    // the world.
    world->getAgent(eventData.at("id"))->loadFromJson(eventData);
  }
};
}  // namespace cse498

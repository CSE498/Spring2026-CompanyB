/**
 * @file ReplayDriver.hpp
 * @author Apar Mohabansi
 **/

#pragma once

#include "nlohmann/json.hpp"
#include "../../tests/tools/MockWorld.hpp"
#include <expected>
#include <memory>

class MockWorld; // Forward declaration for MockWorld

namespace cse498 {
/* Class Description:
 *  ReplayDriver is responsible for replaying logged events from a JSON file.
 *  It reads the events and sends instructions to the world based on the event
 * data. Citation: Used AI responsibly and actively in building the class below.
 */
class MockWorld; // Forward declaration for MockWorld
class ReplayDriver {
 private:
  // world* world; need a pointer to the world object to instuct addtions and
  std::shared_ptr<MockWorld> world;  // This is a placeholder. Replace with actual world class pointer when available.
 public:
  /// @brief Basic constructor for the ReplayDriver.
  ReplayDriver(std::shared_ptr<MockWorld> world) : world(world) {};
  ~ReplayDriver() = default;

  /// @brief Method to replay logged events from a JSON file.
  /// @param filePath Path to the JSON file containing logged events.
  /// @return Success status of the replay operation. True if successful and SendInstructions is called, false otherwise.
  std::expected<bool,std::string> ReplayFromFile(const std::string& filePath);
  /// @brief Method to send instructions to the world based on the event data.
  /// @param eventData JSON data containing the events to be replayed.
  void SendInstructions(const nlohmann::json& eventData);
};
}  // namespace cse498

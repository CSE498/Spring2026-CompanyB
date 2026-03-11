#include "ReplayDriver.hpp"

#include <fstream>
#include <iostream>

namespace cse498 {
std::expected<bool,std::string> ReplayDriver::ReplayFromFile(const std::string &filePath) {
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
void ReplayDriver::SendInstructions(const nlohmann::json &eventData) {
  // This function would contain logic to send instructions to the world based
  // on the event data. For now, we will pretend to send it to the world
    // Here we would have logic to interpret the event and send instructions to
    // the world.
    world->getAgent(eventData.at("id"))->loadFromJson(eventData);
}
}  // namespace cse498

#include "ReplayDriver.hpp"
#include <fstream>
#include <iostream>

namespace cse498 {
    bool ReplayDriver::ReplayFromFile(const std::string& filePath) {
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return false;
        }

        nlohmann::json eventData;
        try {
            inFile >> eventData;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return false;
        }

        SendInstructions(eventData);

        // Here we would add logic to process the eventData and replay the events.
        // For now, we will just print the loaded JSON data.
        // std::cout << "Loaded event data: " << eventData.dump(4) << std::endl;

        return true;
    }
    void ReplayDriver::SendInstructions(const nlohmann::json& eventData) {
        // This function would contain logic to send instructions to the world based on the event data.
        // For now, we will pretend to send it to the world
        auto events = eventData.at("events");
        for (const auto& event : events) {
            // Here we would have logic to interpret the event and send instructions to the world.
            event["type"];
            event["agent_id"];
            event["position"];
            // world->getAgent(event.at("AgentID")).executeInstruction(event.at("Instruction"));
        }
    }
}

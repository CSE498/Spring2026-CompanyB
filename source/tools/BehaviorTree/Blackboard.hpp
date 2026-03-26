#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <expected>

// Based on Step.hpp since its the most relevent for our project
using InfoType = std::variant<int, double, bool>;

class Blackboard {
    public:
        std::expected<InfoType, std::string> getValue(const std::string& key) { 
            if (m_memoryMap.find(key) != m_memoryMap.end()) {
                return m_memoryMap[key];
            }

            else {
                return std::unexpected("Value not found");
            }
        }

        void setValue(const std::string& key, const InfoType& value) {
            auto result = m_memoryMap.find(key);

            if (result != m_memoryMap.end()) {
                result->second = value;
            }

            else {
                m_memoryMap[key] = value;
            }
        }

    private:
        std::unordered_map<std::string, InfoType> m_memoryMap;
};
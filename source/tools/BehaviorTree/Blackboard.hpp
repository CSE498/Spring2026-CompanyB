#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <expected>

// ATTRIBUTIONS: Used ChatGPT to create Docstrings. Further modifications come
// from my input

/**
 * @brief Variant type used to store values in the Blackboard.
 *
 * Based on Step.hpp since its the most relevent for our project
 */
using InfoType = std::variant<int, double, bool>;

/**
 * @brief A simple key-value storage system for sharing data between components.
 *
 * The Blackboard provides a centralized memory structure where values can be
 * stored and retrieved using string keys. It is commonly used in behavior tree
 * implementations to allow nodes to communicate indirectly.
 */
class Blackboard {
    public:
        /**
         * @brief Retrieves a value associated with the given key.
         *
         * @param key The string identifier for the stored value.
         * @return std::expected<InfoType, std::string>
         *         - Contains the value if the key exists.
         *         - Contains an error message if the key is not found.
         *
         * @note The caller is responsible for handling the returned variant type.
         */
        std::expected<InfoType, std::string> getValue(const std::string& key) const { 
            auto result = m_memoryMap.find(key);

            if (result != m_memoryMap.end()) {
                return result->second;
            }

            else {
                return std::unexpected("Value not found");
            }
        }

                /**
         * @brief Inserts or updates a value in the Blackboard.
         *
         * If the key already exists, its value is overwritten.
         * Otherwise, a new key-value pair is created.
         *
         * @param key The string identifier for the value.
         * @param value The value to store (int, double, or bool).
         */
        void setValue(const std::string& key, const InfoType& value) {
            auto result = m_memoryMap.find(key);

            if (result != m_memoryMap.end()) {
                result->second = value;
            }

            else {
                m_memoryMap[key] = value;
            }
        }

    /**
    * @brief Internal storage for key-value pairs.
    *
    * Maps string keys to InfoType values.
    */
    private:
        std::unordered_map<std::string, InfoType> m_memoryMap;
};
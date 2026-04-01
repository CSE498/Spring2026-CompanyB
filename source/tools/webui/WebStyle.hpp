/**
 * @file WebStyle.hpp
 * @brief Class to store and manage CSS styles for web elements.
 */

#pragma once

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace cse498 {

/**
 * @class WebStyle
 * @brief Stores a collection of CSS property-value pairs.
 */
class WebStyle {
 private:
  std::vector<std::pair<std::string, std::string>> styles_;

 public:
  /**
   * @brief Default constructor.
   */
  WebStyle() = default;

  /**
   * @brief Initialize with an initializer list of pairs.
   * @param styles An initializer list of (property, value) pairs.
   */
  WebStyle(std::initializer_list<std::pair<std::string, std::string>> styles)
      : styles_(styles) {}

  /**
   * @brief Initialize with a vector of style pairs.
   * @param styles A vector of (property, value) pairs.
   */
  WebStyle(const std::vector<std::pair<std::string, std::string>>& styles)
      : styles_(styles) {}

  /**
   * @brief Get the list of styles.
   * @return A constant reference to the vector of style pairs.
   */
  const std::vector<std::pair<std::string, std::string>>& GetStyles() const {
    return styles_;
  }
};

}  // namespace cse498

/**
 * @file WebOptions.hpp
 * @brief Struct to store properties of DOM Elements
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "WebStyle.hpp"

namespace cse498 {

// Forward declaration to avoid circular dependency with WebElement
class WebElement;

/**
 * @struct WebOptions
 * @brief Stores information to initialize properties of DOM Elements
 */
struct WebOptions {
  std::string id = "";
  std::vector<std::string> classes = {};
  WebStyle style = {};
  std::vector<std::shared_ptr<WebElement>> children = {};
};

/**
 * @brief Helper function to create shared pointers to WebComponents.
 * Equivalent to std::make_shared but provides a cleaner syntax for declarative UI.
 */
template <typename T, typename... Args>
std::shared_ptr<T> UIItem(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

}  // namespace cse498

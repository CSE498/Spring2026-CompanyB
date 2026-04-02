/**
 * @file WebOptions.hpp
 * @brief Struct to store properties of DOM Elements
 */

#pragma once

#include <vector>

#include "WebStyle.hpp"

namespace cse498 {

/**
 * @struct WebOptions
 * @brief Stores information to initialize properties of DOM Elements
 */
struct WebOptions {
  std::vector<std::string> classes = {};
  WebStyle style = {};
};

}  // namespace cse498

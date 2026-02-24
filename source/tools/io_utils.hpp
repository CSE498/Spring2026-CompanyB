/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A place to put short, helpful functions
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace cse498 {

  // Load a stream into a set of strings.
  inline std::vector<std::string> LoadStream(std::istream & in_stream) {
    std::vector<std::string> out_strings;
    std::string load_line;
    while (std::getline(in_stream, load_line)) {
      out_strings.push_back(load_line);
    }
    return out_strings;
  }

} // End of namespace cse498

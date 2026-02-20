/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>

namespace cse498 {
class WebElement {
 protected:
  std::string id = "";

  /// Constructor that only initializes the ID and does not create a DOM
  /// element. For use by subclasses that create their own element types
  /// (canvas) until further discussion.
  WebElement(const std::string& id, bool) : id(id) {}

 public:
  WebElement() = delete;
  // Create an element with the given ID in the DOM
  WebElement(const std::string& id);
  virtual ~WebElement() = default;

  [[nodiscard]] std::string GetId() const { return id; }
};
}  // namespace cse498

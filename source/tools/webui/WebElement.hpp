/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief
 * @note Status: PROPOSAL
 **/

#pragma once

#include <optional>
#include <string>

#include <emscripten/val.h>

namespace cse498 {
class WebElement {
 protected:
  /// @brief Handle to the DOM element
  emscripten::val dom_element = emscripten::val::null();
  
  /// @brief ID of the DOM element
  std::optional<std::string> id = std::nullopt;

 public:
  /// @brief Delete the default constructor
  WebElement() = delete;

  /// @brief Create an element with the given ID in the DOM
  WebElement(const std::string& tag = "div", const std::string& elem_id = "") {
    // Set id 
    if (!elem_id.empty()) {
      id = elem_id;
    } else {
      id = std::nullopt;
    }

    emscripten::val document = emscripten::val::global("document");

    // Create the element in the DOM
    if (id.has_value()) {
      emscripten::val existing = document.call<emscripten::val>("getElementById", id.value());

      assert((existing.isNull() || existing.isUndefined()) &&
            "Element with this ID already exists in the DOM");
    }

    dom_element = document.call<emscripten::val>("createElement", std::string(tag));

    if (id) dom_element.set("id", id.value());

    document["body"].call<void>("appendChild", dom_element);
  }

  /// @brief Destructor for WebElement that removes the DOM node
  virtual ~WebElement() {
    if (!dom_element.isNull() && !dom_element.isUndefined()) {
      dom_element.call<void>("remove");
    }
  };

  /**
   * @brief Get the ID of the DOM element
   * @return The ID as an optional
   */
  [[nodiscard]] std::optional<std::string> GetId() const { return id; }

  /**
   * @brief Returns the emscripten::val for the DOM Element
   * @return The DOM Element emscripten::val
   */
  [[nodiscard]] emscripten::val& GetDOMElement() {
    return dom_element;
  }
};
}  // namespace cse498

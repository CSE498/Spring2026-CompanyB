/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>

#include <emscripten/val.h>
#include "WebOptions.hpp"

namespace cse498 {
class WebElement : public std::enable_shared_from_this<WebElement> {
 protected:
  /// @brief Handle to the DOM element
  emscripten::val dom_element = emscripten::val::null();
  
  /// @brief ID of the DOM element
  std::string id = "";

 public:
  /// @brief Delete the default constructor
  WebElement() = delete;

  /**
   * @brief Get shared pointer to the WebElement
   */
  std::shared_ptr<WebElement> GetSharedPtr() {
    return shared_from_this();
  }

  operator std::shared_ptr<WebElement>() {
    return shared_from_this();
  }

  /**
   * @brief Create an element with the given tag and options in the DOM
   * @param tag The HTML tag name (e.g. "div", "span")
   * @param options The WebOptions object containing ID, CSS properties and classes (optional)
   */
  WebElement(const std::string& tag = "div", const WebOptions& options = {}) {
    // Set id 
    id = options.id;

    emscripten::val document = emscripten::val::global("document");

    // Create the element in the DOM
    if (!id.empty()) {
      emscripten::val existing = document.call<emscripten::val>("getElementById", id);

      assert((existing.isNull() || existing.isUndefined()) &&
            "Element with this ID already exists in the DOM");
    }

    dom_element = document.call<emscripten::val>("createElement", std::string(tag));

    if (!id.empty()) dom_element.set("id", id);

    // Apply styles from WebOptions.style
    for (const auto& [property, value] : options.style.GetStyles()) {
      dom_element["style"].set(property, value);
    }

    // Apply classes from WebOptions.classes
    for (const auto& css_class : options.classes) {
      dom_element["classList"].call<void>("add", css_class);
    }

    document["body"].call<void>("appendChild", dom_element);
  }

  /**
   * @brief Add a CSS class to the element
   * @param css_class The name of the class to add
   */
  void AddClass(const std::string& css_class) {
    dom_element["classList"].call<void>("add", css_class);
  }

  /**
   * @brief Remove a CSS class from the element
   * @param css_class The name of the class to remove
   */
  void RemoveClass(const std::string& css_class) {
    dom_element["classList"].call<void>("remove", css_class);
  }

  /**
   * @brief Apply a set of CSS styles to the element
   * @param style The WebStyle object containing CSS property-value pairs
   */
  void SetStyle(const WebStyle& style) {
    for (const auto& [property, value] : style.GetStyles()) {
      dom_element["style"].set(property, value);
    }
  }

  /// @brief Destructor for WebElement that removes the DOM node
  virtual ~WebElement() {
    if (!dom_element.isNull() && !dom_element.isUndefined()) {
      dom_element.call<void>("remove");
    }
  };

  /**
   * @brief Get the ID of the DOM element
   * @return The ID as a string
   */
  [[nodiscard]] std::string GetId() const { return id; }

  /**
   * @brief Returns the emscripten::val for the DOM Element
   * @return The DOM Element emscripten::val
   */
  [[nodiscard]] emscripten::val& GetDOMElement() {
    return dom_element;
  }
};
}  // namespace cse498

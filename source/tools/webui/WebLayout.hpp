/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class to layout web UI elements in a simple flex container.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <emscripten/val.h>

#include <expected>
#include <memory>
#include <utility>
#include <vector>

#include "WebElement.hpp"

namespace cse498 {

/**
 * @brief Manages a flex container in the DOM for laying out WebElement
 * children.
 */
class WebLayout : public WebElement {
 protected:
  std::vector<std::shared_ptr<WebElement>> elements;
  /// The DOM layout element
  emscripten::val dom_element = emscripten::val::null();

 public:
  enum class Error {
    NullPtr,
    ElementNotFound,
    DOMElementNotFound,
    ElementAlreadyMember,
  };

  /**
   * @brief Constructs a WebLayout, creating a flex div in the DOM with the
   * given ID.
   * @param id The HTML element ID to assign to the layout container.
   */
  WebLayout(std::string id);

  /**
   * @brief Destructor. Removes the layout container from the DOM.
   */
  ~WebLayout();

  /**
   * @brief Appends a child WebElement to this layout in the DOM.
   * @param elem The child element to add.
   * @return Reference to this layout for method chaining.
   */
  std::expected<void, WebLayout::Error> AddChild(
      std::shared_ptr<WebElement> elem);

  /**
   * @brief Removes a child WebElement from this layout in the DOM.
   * @param elem The child element to remove.
   * @return Reference to this layout for method chaining.
   */
  std::expected<void, WebLayout::Error> RemoveChild(
      std::shared_ptr<WebElement> elem);

  size_t GetNumChildren() const;

  bool ContainsChild(std::shared_ptr<WebElement> elem) const;

  /**
   * @brief Sets the flex-direction CSS property of the layout container.
   * @param dir The flex direction value (e.g., "row", "column").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetDirection(std::string dir);

  /**
   * @brief Sets the justify-content CSS property of the layout container.
   * @param justify The justify-content value (e.g., "center", "space-between").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetJustifyContent(std::string justify);

  /**
   * @brief Sets the align-items CSS property of the layout container.
   * @param align The align-items value (e.g., "center", "flex-start").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetAlignItems(std::string align);

  /**
   * @brief Sets the align-content CSS property of the layout container.
   * @param alignContent The align-content value (e.g., "stretch",
   * "space-around").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetAlignContent(std::string alignContent);

  /**
   * @brief Sets the gap CSS property of the layout container.
   * @param gap The gap value (e.g., "10px", "1rem").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetGap(std::string gap);
};

}  // End of namespace cse498

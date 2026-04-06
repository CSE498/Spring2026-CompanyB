/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class to layout web UI elements in a simple flex container.
 * @note Status: PROPOSAL
 * @author Udbhav Saxena
 * Note: Claude was used to generate docstrings
 **/

#pragma once

#include <expected>
#include <memory>
#include <utility>
#include <vector>

#include "WebElement.hpp"

namespace cse498 {

/**
 * @brief Manages a flex container in the DOM for laying out WebElement
 * children.
 * 
 * Note: `std::enable_shared_from_this` allows us to use fluent interface style APIs with shared pointers
 */
class WebLayout : public WebElement {
 private:
  std::vector<std::shared_ptr<WebElement>> elements;

 public:
  enum class Error {
    NullPtr,
    ElementNotFound,
    DOMElementNotFound,
    ElementAlreadyMember,
  };

  /**
   * @brief Constructs a WebLayout, creating a flex div in the DOM.
   * @param options Optional WebOptions to apply to the layout.
   */
  WebLayout(const WebOptions& options = {});

  /// Disable copy assignment operator
  WebLayout& operator=(const WebLayout&) = delete;

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

  /**
   * @brief Get the number of children in the WebLayout
   * @return The number of children
   */
  [[nodiscard]] size_t GetNumChildren() const;

  bool ContainsChild(std::shared_ptr<WebElement> elem) const;

  /**
   * @brief Sets the flex-direction CSS property of the layout container.
   * @param dir The flex direction value (e.g., "row", "column").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetDirection(const std::string& dir);

  /**
   * @brief Gets the flex-direction CSS property of the layout container.
   * @return The current flex direction value.
   */
  [[nodiscard]] std::string GetDirection() const;

  /**
   * @brief Sets the height CSS property of the layout container.
   * @param height The height value (e.g., "100px", "50%").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetHeight(const std::string& height);

  /**
   * @brief Gets the height CSS property of the layout container.
   * @return The current height value.
   */
  [[nodiscard]] std::string GetHeight() const;

  /**
   * @brief Sets the justify-content CSS property of the layout container.
   * @param justify The justify-content value (e.g., "center", "space-between").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetJustifyContent(const std::string& justify);

  /**
   * @brief Gets the justify-content CSS property of the layout container.
   * @return The current justify-content value.
   */
  [[nodiscard]] std::string GetJustifyContent() const;

  /**
   * @brief Sets the align-items CSS property of the layout container.
   * @param align The align-items value (e.g., "center", "flex-start").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetAlignItems(const std::string& align);

  /**
   * @brief Gets the align-items CSS property of the layout container.
   * @return The current align-items value.
   */
  [[nodiscard]] std::string GetAlignItems() const;

  /**
   * @brief Sets the align-content CSS property of the layout container.
   * @param alignContent The align-content value (e.g., "stretch",
   * "space-around").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetAlignContent(const std::string& alignContent);

  /**
   * @brief Gets the align-content CSS property of the layout container.
   * @return The current align-content value.
   */
  [[nodiscard]] std::string GetAlignContent() const;

  /**
   * @brief Sets the gap CSS property of the layout container.
   * @param gap The gap value (e.g., "10px", "1rem").
   * @return Reference to this layout for method chaining.
   */
  WebLayout& SetGap(const std::string& gap);

  /**
   * @brief Gets the gap CSS property of the layout container.
   * @return The current gap value.
   */
  [[nodiscard]] std::string GetGap() const;
};

}  // End of namespace cse498

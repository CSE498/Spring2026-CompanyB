/**
 * @file WebTextbox.hpp
 * @brief C++ API for the DOM Text/Div Element using Emscripten.
 */

#pragma once

#include <emscripten/val.h>

#include <algorithm>  // For std::clamp
#include <expected>   // For std::expected
#include <functional>
#include <string>

namespace cse498 {

/**
 * @struct TextStyle
 * @brief Holds CSS styling properties for the WebTextbox.
 */
struct TextStyle {
  std::string font_family = "Arial";
  std::string font_size = "16px";
  std::string color = "black";
  std::string background_color = "transparent";
  bool bold = false;
};

/**
 * @class WebTextbox
 * @brief Manages a DOM div element to display text.
 * Designed for high-throughput logging with memory constraints and headless CI
 * support.
 */
class WebTextbox {
 private:
  // Google Style: Member variables end with a trailing underscore.
  std::string id_;
  emscripten::val div_element_ = emscripten::val::null();

  std::string mock_text_content_;
  size_t max_length_ = 50000;

  // Google Style: constexpr variables start with 'k' and use mixed case.
  static constexpr const char* kDefaultBorder = "1px solid #ccc";
  static constexpr const char* kDefaultPadding = "5px";
  static constexpr const char* kDefaultOverflow = "auto";
  static constexpr const char* kDefaultZIndex = "9999";
  static constexpr size_t kMinAllowedLength = 1;
  static constexpr size_t kMaxAllowedLength = 52428800;  // 50MB cap

  /**
   * @brief Checks if the environment lacks a DOM (e.g., running in Node.js).
   * @return true if headless, false if running in a web browser.
   */
  [[nodiscard]] bool IsHeadless() const;

 public:
  /**
   * @brief Constructs a new WebTextbox and attaches it to the DOM.
   * @param id The unique HTML ID for the generated div.
   * @param style Optional TextStyle to apply upon creation.
   */
  WebTextbox(const std::string& id, const TextStyle& style = TextStyle());

  // Prevent copying to avoid DOM element duplication and memory leaks.
  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  /**
   * @brief Destroys the WebTextbox and removes the element from the DOM.
   */
  ~WebTextbox();

  /**
   * @brief Overwrites the current text, truncating if it exceeds max length.
   * @param text The new text to display.
   */
  void SetText(const std::string& text);

  /**
   * @brief Appends text to the end of the box and auto-scrolls to the bottom.
   * @param text The text to append.
   */
  void AppendText(const std::string& text);

  /**
   * @brief Clears all text from the box.
   */
  void Clear();

  /**
   * @brief Applies CSS styles based on a TextStyle struct.
   * @param style The struct containing the desired visual parameters.
   */
  void SetStyle(const TextStyle& style);

  /**
   * @brief Assigns a CSS class for external stylesheet management.
   * @param css_class The name of the CSS class.
   */
  void SetClass(const std::string& css_class);

  /**
   * @brief Toggles the visibility of the textbox on the screen.
   * @param visible True to show, false to hide (display: none).
   */
  void SetVisible(bool visible);

  /**
   * @brief Sets the absolute position of the textbox on the screen.
   * @param x The left offset in pixels.
   * @param y The top offset in pixels.
   */
  void SetPosition(int x, int y);

  /**
   * @brief Sets the dimensions of the textbox.
   * @param width The width in pixels.
   * @param height The height in pixels.
   */
  void SetSize(int width, int height);

  /**
   * @brief Sets the maximum character limit to prevent memory bloat.
   * @param length The maximum number of characters (clamped to safe bounds).
   */
  void SetMaxLength(size_t length);

  /**
   * @brief Retrieves the current text contained in the box.
   * @return A std::expected containing the inner text, or an error string if
   * invalid.
   */
  [[nodiscard]] std::expected<std::string, std::string> GetText() const;

  /**
     * @brief Template method to append numeric values directly.
     * @tparam T Any numeric type supported by std::to_string.
     * @param value The value to append to the textbox.
     */
  template <typename T>
  void AppendValue(const T& value) {
    // Uses type deduction to convert the raw value to a string
    AppendText(std::to_string(value));
  }

  /**
     * @brief Applies a custom lambda transformation to the current text.
     * @param transform_fn A lambda function that takes a string and returns a modified string.
     */
  void TransformText(const std::function<std::string(const std::string&)>& transform_fn);

  /**
     * @brief Appends a new line of text wrapped in a styled span.
     * @param text The string to append.
     * @param css_class The CSS class name to apply to the span (e.g., "error-text").
     */
  void AppendStyledLine(const std::string& text, const std::string& css_class);
};

}  // namespace cse498
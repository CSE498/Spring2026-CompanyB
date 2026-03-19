/**
 * @file WebTextbox.cpp
 * @brief Implementation of WebTextbox using Embind.
 */

#include "WebTextbox.hpp"

#include <cassert>
#include <iostream>

using namespace emscripten;

namespace cse498 {

/**
 * @brief Internal helper to determine if the code is running without a browser
 * window. This prevents Embind from crashing when queried in a Node.js testing
 * environment.
 */
bool WebTextbox::IsHeadless() const {
  val global = val::global("globalThis");
  return global["document"].isUndefined();
}

/**
 * @brief Initializes the DOM element, sets its baseline styles, and appends it
 * to the body. If running headlessly, it bypasses DOM creation entirely.
 */
WebTextbox::WebTextbox(const std::string& id, const TextStyle& style)
    : id_(id) {
  if (IsHeadless()) return;

  val document = val::global("document");
  val existing = document.call<val>("getElementById", id_);
  assert((existing.isNull() || existing.isUndefined()) &&
         "WebTextbox ID already exists in DOM");

  div_element_ = document.call<val>("createElement", std::string("div"));
  div_element_.set("id", id_);

  div_element_["style"].set("position", "absolute");
  div_element_["style"].set("border", kDefaultBorder);
  div_element_["style"].set("padding", kDefaultPadding);
  div_element_["style"].set("overflow", kDefaultOverflow);
  div_element_["style"].set("zIndex", kDefaultZIndex);

  document["body"].call<void>("appendChild", div_element_);
  SetStyle(style);
}

/**
 * @brief Safely removes the div element from the webpage to prevent ghost
 * elements and memory leaks when the C++ object goes out of scope.
 */
WebTextbox::~WebTextbox() {
  if (IsHeadless()) return;

  if (!div_element_.isNull() && !div_element_.isUndefined()) {
    div_element_.call<void>("remove");
  }
}

/**
 * @brief Replaces the contents of the textbox. Safe against cross-site
 * scripting (XSS) by utilizing innerText, and bounded by max_length_ to prevent
 * memory issues.
 */
void WebTextbox::SetText(const std::string& text) {
  std::string safe_text = text.length() > max_length_
                              ? text.substr(text.length() - max_length_)
                              : text;

  if (IsHeadless()) {
    mock_text_content_ = safe_text;
    return;
  }

  div_element_.set("innerText", safe_text);
}

/**
 * @brief Appends text and automatically pushes the scrollbar to the bottom.
 * If the total string exceeds max_length_, the oldest characters are discarded.
 */
void WebTextbox::AppendText(const std::string& text) {
  if (IsHeadless()) {
    mock_text_content_ += text;
    if (mock_text_content_.length() > max_length_) {
      mock_text_content_ =
          mock_text_content_.substr(mock_text_content_.length() - max_length_);
    }
    return;
  }

  std::string current = div_element_["innerText"].as<std::string>();
  current += text;

  if (current.length() > max_length_) {
    current = current.substr(current.length() - max_length_);
  }

  div_element_.set("innerText", current);
  div_element_.set("scrollTop", div_element_["scrollHeight"]);  // Auto-scroll
}

/**
 * @brief Utility function to quickly wipe the textbox clean.
 */
void WebTextbox::Clear() { SetText(""); }

/**
 * @brief Reads the current text back from the DOM (or the mock buffer if
 * headless). Uses std::expected to safely handle cases where the DOM element
 * failed to initialize.
 */
std::expected<std::string, std::string> WebTextbox::GetText() const {
  if (IsHeadless()) return mock_text_content_;

  if (div_element_.isNull() || div_element_.isUndefined()) {
    return std::unexpected("Error: DOM element is null or undefined.");
  }
  return div_element_["innerText"].as<std::string>();
}

/**
 * @brief Translates the C++ TextStyle struct into direct CSS property
 * modifications.
 */
void WebTextbox::SetStyle(const TextStyle& style) {
  if (IsHeadless()) return;

  val css = div_element_["style"];
  css.set("fontFamily", style.font_family);
  css.set("fontSize", style.font_size);  // Now directly applies the string
  css.set("color", style.color);
  css.set("backgroundColor", style.background_color);
  css.set("fontWeight", style.bold ? "bold" : "normal");
}

/**
 * @brief Allows the UI team to apply a pre-defined CSS class to the element.
 */
void WebTextbox::SetClass(const std::string& css_class) {
  if (IsHeadless()) return;
  div_element_.set("className", css_class);
}

/**
 * @brief Hides or shows the element without removing it from the DOM entirely.
 */
void WebTextbox::SetVisible(bool visible) {
  if (IsHeadless()) return;
  div_element_["style"].set("display", visible ? "block" : "none");
}

/**
 * @brief Updates the X and Y coordinates of the element relative to its
 * container.
 */
void WebTextbox::SetPosition(int x, int y) {
  if (IsHeadless()) return;
  val css = div_element_["style"];
  css.set("left", std::to_string(x) + "px");
  css.set("top", std::to_string(y) + "px");
}

/**
 * @brief Hard-codes the bounding box dimensions of the text area.
 */
void WebTextbox::SetSize(int width, int height) {
  if (IsHeadless()) return;
  val css = div_element_["style"];
  css.set("width", std::to_string(width) + "px");
  css.set("height", std::to_string(height) + "px");
}

/**
 * @brief Updates the truncation limit. Clamps the input to prevent developers
 * from passing 0 or values large enough to crash the browser.
 */
void WebTextbox::SetMaxLength(size_t length) {
  max_length_ = std::clamp(length, kMinAllowedLength, kMaxAllowedLength);
}

}  // namespace cse498
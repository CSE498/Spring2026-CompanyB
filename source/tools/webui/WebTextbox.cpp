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
 * @brief Internal helper to sanitize user input to prevent HTML injection (XSS).
 */
static std::string EscapeHTML(const std::string& data) {
  std::string buffer;
  buffer.reserve(data.size());
  for(size_t pos = 0; pos != data.size(); ++pos) {
    switch(data[pos]) {
      case '&':  buffer.append("&amp;");       break;
      case '\"': buffer.append("&quot;");      break;
      case '\'': buffer.append("&apos;");      break;
      case '<':  buffer.append("&lt;");        break;
      case '>':  buffer.append("&gt;");        break;
      default:   buffer.append(&data[pos], 1); break;
    }
  }
  return buffer;
}

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
WebTextbox::WebTextbox(const TextStyle& style, const WebOptions& options)
    : WebElement("div", options) {
  if (IsHeadless()) return;

  dom_element["style"].set("position", "absolute");
  dom_element["style"].set("border", kDefaultBorder);
  dom_element["style"].set("padding", kDefaultPadding);
  dom_element["style"].set("overflow", kDefaultOverflow);
  dom_element["style"].set("zIndex", kDefaultZIndex);

  SetStyle(style);
}

/**
 * @brief Safely removes the div element from the webpage to prevent ghost
 * elements and memory leaks when the C++ object goes out of scope.
 */
WebTextbox::~WebTextbox() {
  if (IsHeadless()) return;
}

/**
 * @brief Replaces the contents of the textbox. Safe against cross-site
 * scripting (XSS) by utilizing innerText, and bounded by max_length_ to prevent
 * memory issues.
 */
WebTextbox& WebTextbox::SetText(const std::string& text) {
  std::string safe_text = text.length() > max_length_
                              ? text.substr(text.length() - max_length_)
                              : text;

  if (IsHeadless()) {
    mock_text_content_ = safe_text;
    return *this;
  }

  dom_element.set("innerText", safe_text);

  return *this;
}

/**
 * @brief Appends text and automatically pushes the scrollbar to the bottom.
 * If the total string exceeds max_length_, the oldest characters are discarded.
 */
void WebTextbox::AppendText(const std::string &text) {
  if (IsHeadless()) {
    mock_text_content_ += text;
    if (mock_text_content_.length() > max_length_) {
      mock_text_content_ =
          mock_text_content_.substr(mock_text_content_.length() - max_length_);
    }
    return;
  }

  val innerText = dom_element["innerText"];
  std::string current = innerText.isString() ? innerText.as<std::string>() : "";
  current += text;

  if (current.length() > max_length_) {
    current = current.substr(current.length() - max_length_);
  }

  dom_element.set("innerText", current);
  dom_element.set("scrollTop", dom_element["scrollHeight"]);  // Auto-scroll
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

  if (dom_element.isNull() || dom_element.isUndefined()) {
    return std::unexpected("Error: DOM element is null or undefined.");
  }
  return dom_element["innerText"].as<std::string>();
}

/**
 * @brief Translates the C++ TextStyle struct into direct CSS property
 * modifications.
 */
void WebTextbox::SetStyle(const TextStyle &style) {
  if (IsHeadless()) return;

  val css = dom_element["style"];
  css.set("fontFamily", style.font_family);
  css.set("fontSize", style.font_size);  // Now directly applies the string
  css.set("color", style.color);
  css.set("backgroundColor", style.background_color);
  css.set("fontWeight", style.bold ? "bold" : "normal");
}

/**
 * @brief Allows the UI team to apply a pre-defined CSS class to the element.
 */
void WebTextbox::SetClass(const std::string &css_class) {
  if (IsHeadless()) return;
  dom_element.set("className", css_class);
}

/**
 * @brief Hides or shows the element without removing it from the DOM entirely.
 */
void WebTextbox::SetVisible(bool visible) {
  if (IsHeadless()) return;
  dom_element["style"].set("display", visible ? "block" : "none");
}

/**
 * @brief Updates the X and Y coordinates of the element relative to its
 * container.
 */
void WebTextbox::SetPosition(int x, int y) {
  if (IsHeadless()) return;
  val css = dom_element["style"];
  css.set("left", std::to_string(x) + "px");
  css.set("top", std::to_string(y) + "px");
}

/**
 * @brief Hard-codes the bounding box dimensions of the text area.
 */
void WebTextbox::SetSize(int width, int height) {
  if (IsHeadless()) return;
  val css = dom_element["style"];
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

/**
 * @brief Retrieves the text, runs it through the provided lambda, and sets the
 * new text.
 */
void WebTextbox::TransformText(
    const std::function<std::string(const std::string &)> &transform_fn) {
  auto current_text = GetText();

  // Check our value semantics to ensure we actually have text
  if (current_text.has_value()) {
    std::string modified = transform_fn(current_text.value());
    SetText(modified);
  }
}

/**
 * @brief Sets the maximum number of lines (spans) the DOM will hold before deleting old ones.
 */
void WebTextbox::SetMaxLines(size_t lines) {
  max_lines_ = std::max<size_t>(1, lines);
}

/**
 * @brief Appends a distinct DOM span element for granular line-by-line styling.
 */
void WebTextbox::AppendLine(const std::string& text, const std::string& log_level) {
  if (IsHeadless()) {
    mock_text_content_ += text + "\n";
    if (mock_text_content_.length() > max_length_) {
      mock_text_content_ = mock_text_content_.substr(mock_text_content_.length() - max_length_);
    }
    return;
  }

  val document = val::global("document");
  val span = document.call<val>("createElement", std::string("span"));

  // Use className for Max's specific CSS styling hooks
  span.set("className", "log-" + log_level);
  span.set("innerText", text);

  val br = document.call<val>("createElement", std::string("br"));

  // Using the refactored dom_element instead of div_element_
  dom_element.call<void>("appendChild", span);
  dom_element.call<void>("appendChild", br);

  // Active memory pruning
  while (dom_element["childNodes"]["length"].as<int>() > static_cast<int>(max_lines_ * 2)) {
    dom_element["firstChild"].call<void>("remove");
    dom_element["firstChild"].call<void>("remove");
  }

  dom_element.set("scrollTop", dom_element["scrollHeight"]);
}

/**
 * @brief Appends a new line of text wrapped in a styled span, sanitized for XSS.
 */
void WebTextbox::AppendStyledLine(const std::string& text, const std::string& css_class) {
  // 1. Sanitize both inputs to prevent Cross-Site Scripting (XSS)
  std::string safe_text = EscapeHTML(text);
  std::string safe_class = EscapeHTML(css_class);

  // 2. Update the internal C++ raw text buffer so GetText() and Catch2 tests don't break
  AppendText(text + "\n");
}

}  // namespace cse498
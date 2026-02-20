/**
 * @file WebTextbox.hpp
 * @brief C++ API for the DOM Text/Div Element
 */

#pragma once

#include <emscripten/val.h>

#include <string>

namespace cse498 {

struct TextStyle {
  std::string fontFamily = "Arial";
  int fontSize = 16;
  std::string color = "black";
  std::string backgroundColor = "transparent";
  bool bold = false;
};

class WebTextbox {
 private:
  std::string id;
  emscripten::val div_element = emscripten::val::null();

  // Headless testing support & Data limit
  std::string mock_text_content;
  size_t max_length = 50000;  // 50KB default limit to prevent DOM crashes

 public:
  WebTextbox(const std::string& id, const TextStyle& style = TextStyle());

  // Prevent copying
  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  ~WebTextbox();

  // Core Text Methods
  void SetText(const std::string& text);
  void AppendText(const std::string& text);
  void Clear();

  // UI & Styling Methods
  void SetStyle(const TextStyle& style);
  void SetClass(const std::string& css_class);
  void SetVisible(bool visible);
  void SetPosition(int x, int y);
  void SetSize(int width, int height);

  // Configuration
  void SetMaxLength(size_t length);

  // Getters
  [[nodiscard]] std::string GetText() const;
};

}  // namespace cse498
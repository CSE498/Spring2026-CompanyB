/**
 * @file WebImage.cpp
 * @brief C++ wrapper for HTML image elements
 * @author Mariana Vangelov
 **/

#include "WebImage.hpp"

#include <emscripten/val.h>

#include <cassert>
#include <expected>

using emscripten::val;

namespace cse498 {

WebImage::WebImage(const std::string& src,
                   const std::string& alt_text,
                   const WebOptions& options)
    : WebElement("img", options), src(src), alt(alt_text) {
  dom_element.set("src", src);
  dom_element.set("alt", alt);

  // Enable absolute positioning
  dom_element["style"].set("position", std::string("absolute"));
}

WebImage::~WebImage() {
}

void WebImage::SetSource(const std::string& new_src) {
  src = new_src;
  dom_element.set("src", src);
}

WebImage& WebImage::SetPosition(int x, int y) {
  x_pos = x;
  y_pos = y;

  dom_element["style"].set("left", std::to_string(x_pos) + "px");
  dom_element["style"].set("top", std::to_string(y_pos) + "px");

  return *this;
}

void WebImage::SetAlt(const std::string& alt_text) {
  alt = alt_text;
  dom_element.set("alt", alt);
}

void WebImage::SetVisible(bool is_visible) {
  if (is_visible) {
    dom_element["style"].set("display", std::string("block"));
  } else {
    dom_element["style"].set("display", std::string("none"));
  }
}

bool WebImage::IsLoaded() const { return dom_element["complete"].as<bool>(); }

std::expected<void, std::string> WebImage::HasError() const {
  // Check the naturalWidth to insure it isnt 0 and is properly loaded
  if (dom_element["complete"].as<bool>() &&
      dom_element["naturalWidth"].as<int>() == 0) {
    return std::unexpected("Image failed to load: " + src);
  }
  return {};
}

}  // namespace cse498
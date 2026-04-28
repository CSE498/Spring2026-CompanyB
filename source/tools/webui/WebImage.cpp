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

WebImage::WebImage(const std::string& src, const std::string& alt_text,
                   const WebOptions& options)
    : WebElement("img", options), src_(src), alt_(alt_text) {
  dom_element.set("src", src_);
  dom_element.set("alt", alt_);

  // Enable absolute positioning
  dom_element["style"].set("position", std::string("absolute"));
}

WebImage::~WebImage() {}

WebImage& WebImage::SetSource(const std::string& new_src) {
  assert(!new_src.empty() && "Image source path cannot be empty");
  src_ = new_src;
  dom_element.set("src", src_);
  return *this;
}

WebImage& WebImage::SetPosition(int x, int y) {
  x_pos_ = x;
  y_pos_ = y;

  dom_element["style"].set("left", std::to_string(x_pos_) + "px");
  dom_element["style"].set("top", std::to_string(y_pos_) + "px");

  return *this;
}

WebImage& WebImage::SetAlt(const std::string& alt_text) {
  alt_ = alt_text;
  dom_element.set("alt", alt_);
  return *this;
}

WebImage& WebImage::SetVisible(bool is_visible) {
  visible_ = is_visible;
  dom_element["style"].set("display",
                           std::string(is_visible ? "block" : "none"));
  return *this;
}

bool WebImage::IsLoaded() const { return dom_element["complete"].as<bool>(); }

std::expected<void, std::string> WebImage::HasError() const {
  // Check the naturalWidth to insure it isnt 0 and is properly loaded
  if (dom_element["complete"].as<bool>() &&
      dom_element["naturalWidth"].as<int>() == 0) {
    return std::unexpected("Image failed to load: " + src_);
  }
  return {};
}

}  // namespace cse498

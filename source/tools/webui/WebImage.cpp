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

WebImage::WebImage(const std::string& img_id, const std::string& src,
                   const std::string& alt_text)
    : WebElement(img_id, true), src_(src), alt_(alt_text) {
  val document = val::global("document");

  // Check if ID already exists
  val existing = document.call<val>("getElementById", id);
  assert((existing.isNull() || existing.isUndefined()) &&
         "Image element with this ID already exists");

  // Create the <img> element
  img_element_ = document.call<val>("createElement", std::string("img"));
  img_element_.set("id", id);
  img_element_.set("src", src_);
  img_element_.set("alt", alt_);

  // Add to the page
  document["body"].call<void>("appendChild", img_element_);

  // Enable absolute positioning
  img_element_["style"].set("position", std::string("absolute"));
}

WebImage::~WebImage() {
  if (!img_element_.isNull() && !img_element_.isUndefined()) {
    img_element_.call<void>("remove");
  }
}

void WebImage::SetSource(const std::string& new_src) {
  assert(!new_src.empty() && "Image source path cannot be empty");
  src_ = new_src;
  img_element_.set("src", src_);
}

void WebImage::SetPosition(int x, int y) {
  x_pos_ = x;
  y_pos_ = y;

  img_element_["style"].set("left", std::to_string(x_pos_) + "px");
  img_element_["style"].set("top", std::to_string(y_pos_) + "px");
}

void WebImage::SetAlt(const std::string& alt_text) {
  alt_ = alt_text;
  img_element_.set("alt", alt_);
}

void WebImage::SetVisible(bool is_visible) {
  visible_ = is_visible;
  img_element_["style"].set("display",
                             std::string(is_visible ? "block" : "none"));
}

bool WebImage::IsLoaded() const { return img_element_["complete"].as<bool>(); }

std::expected<void, std::string> WebImage::HasError() const {
  // Check the naturalWidth to insure it isnt 0 and is properly loaded
  if (img_element_["complete"].as<bool>() &&
      img_element_["naturalWidth"].as<int>() == 0) {
    return std::unexpected("Image failed to load: " + src_);
  }
  return {};
}

}  // namespace cse498

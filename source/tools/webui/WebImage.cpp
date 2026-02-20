/**
 * @file WebImage.cpp
 * @brief C++ wrapper for HTML image elements
 * @author Mariana Vangelov
 **/

#include "WebImage.hpp"
#include <emscripten/val.h>
#include <cassert>

using emscripten::val;

namespace cse498 {


WebImage::WebImage(const std::string& img_id, const std::string& src) 
    : id(img_id), src(src) {
  
  val document = val::global("document");
  
  // Check if ID already exists
  val existing = document.call<val>("getElementById", id);
  assert((existing.isNull() || existing.isUndefined()) && 
         "Image element with this ID already exists");
  
  // Create the <img> element
  img_element = document.call<val>("createElement", std::string("img"));
  img_element.set("id", id);
  img_element.set("src", src);
  
  // Add to the page
  document["body"].call<void>("appendChild", img_element);
  
  // Enable absolute positioning
  img_element["style"].set("position", std::string("absolute"));
}


WebImage::~WebImage() {
  if (!img_element.isNull() && !img_element.isUndefined()) {
    img_element.call<void>("remove");
  }
}


void WebImage::SetSource(const std::string& new_src) {
  src = new_src;
  img_element.set("src", src);
}


void WebImage::SetSize(int w, int h) {
  assert(w >= 0 && "Width must be non-negative");
  assert(h >= 0 && "Height must be non-negative");
  
  width = w;
  height = h;
  
  img_element["style"].set("width", std::to_string(width) + "px");
  img_element["style"].set("height", std::to_string(height) + "px");
}

void WebImage::SetPosition(int x, int y) {
  x_pos = x;
  y_pos = y;
  
  img_element["style"].set("left", std::to_string(x_pos) + "px");
  img_element["style"].set("top", std::to_string(y_pos) + "px");
}

void WebImage::SetAlt(const std::string& alt_text) {
  alt = alt_text;
  img_element.set("alt", alt);
}


void WebImage::SetVisible(bool is_visible) {
  if (is_visible) {
    img_element["style"].set("display", std::string("block"));
  } else {
    img_element["style"].set("display", std::string("none"));
  }
}

bool WebImage::IsLoaded() const{
  return img_element["complete"].as<bool>();

}

bool WebImage::HasError() const{
  //Check the naturalWidth to insure it isnt 0 and is properly loaded
  return img_element["complete"].as<bool>() && 
  img_element["naturalWidth"].as<int>() == 0;

}


} 
/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class to layout web UI elements in a simple flex container.
 * @note Status: PROPOSAL
 **/

#include "WebLayout.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <algorithm>

using emscripten::val;

namespace cse498 {

WebLayout::WebLayout(const WebOptions& options) : WebElement("div", options) {
  dom_element["style"].set("display", "flex");

  // Add children from options if any
  for (const auto& child : options.children) {
    AddChild(child);
  }
}

std::expected<void, WebLayout::Error> WebLayout::AddChild(
    std::shared_ptr<WebElement> elem) {
  if (!elem) {
    return std::unexpected(WebLayout::Error::NullPtr);
  }
  if (ContainsChild(elem)) {
    return std::unexpected(WebLayout::Error::ElementAlreadyMember);
  }

  // Add child element to the layout's DOM tree
  dom_element.call<void>("appendChild", elem->GetDOMElement());

  // Add pointer to the element to the list of children
  elements.push_back(elem);

  return {};
}

std::expected<void, WebLayout::Error> WebLayout::RemoveChild(
    std::shared_ptr<WebElement> elem) {
  if (!elem) {
    return std::unexpected(WebLayout::Error::NullPtr);
  }
  if (!ContainsChild(elem)) {
    return std::unexpected(WebLayout::Error::ElementNotFound);
  }

  // Remove child element from the layout
  dom_element.call<void>("removeChild", elem->GetDOMElement());

  // Remove child element pointer from the list of children
  std::erase(elements, elem);

  return {};
}

size_t WebLayout::GetNumChildren() const { return elements.size(); }

bool WebLayout::ContainsChild(std::shared_ptr<WebElement> elem) const {
  return std::find(elements.begin(), elements.end(), elem) != elements.end();
}

WebLayout& WebLayout::SetDirection(const std::string& dir) {
  dom_element["style"].set("flexDirection", dir);
  return *this;
}

std::string WebLayout::GetDirection() const {
  return dom_element["style"]["flexDirection"].as<std::string>();
}

WebLayout& WebLayout::SetJustifyContent(const std::string& justify) {
  dom_element["style"].set("justifyContent", justify);
  return *this;
}

std::string WebLayout::GetJustifyContent() const {
  return dom_element["style"]["justifyContent"].as<std::string>();
}

WebLayout& WebLayout::SetHeight(const std::string& height) {
  dom_element["style"].set("height", height);
  return *this;
}

std::string WebLayout::GetHeight() const {
  return dom_element["style"]["height"].as<std::string>();
}

WebLayout& WebLayout::SetAlignItems(const std::string& align) {
  dom_element["style"].set("alignItems", align);
  return *this;
}

std::string WebLayout::GetAlignItems() const {
  return dom_element["style"]["alignItems"].as<std::string>();
}

WebLayout& WebLayout::SetAlignContent(const std::string& alignContent) {
  dom_element["style"].set("alignContent", alignContent);
  return *this;
}

std::string WebLayout::GetAlignContent() const {
  return dom_element["style"]["alignContent"].as<std::string>();
}

WebLayout& WebLayout::SetGap(const std::string& gap) {
  dom_element["style"].set("gap", gap);
  return *this;
}

std::string WebLayout::GetGap() const {
  return dom_element["style"]["gap"].as<std::string>();
}
}  // namespace cse498

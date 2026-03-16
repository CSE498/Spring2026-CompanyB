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

WebLayout::WebLayout(std::string id) : WebElement(id, false) {
  val document = val::global("document");
  val existing = document.call<val>("getElementById", id);

  assert((existing.isNull() || existing.isUndefined()) &&
         "Element with this ID already exists in the DOM");

  dom_element = document.call<val>("createElement", std::string("div"));
  dom_element["style"].set("display", "flex");
  dom_element.set("id", id);
  document["body"].call<void>("appendChild", dom_element);
}

WebLayout::~WebLayout() {
  if (!dom_element.isNull() && !dom_element.isUndefined()) {
    dom_element.call<void>("remove");
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
  val document = val::global("document");
  val childElem = document.call<val>("getElementById", elem->GetId());
  if (childElem.isNull() || childElem.isUndefined()) {
    return std::unexpected(WebLayout::Error::DOMElementNotFound);
  }
  dom_element.call<void>("appendChild", childElem);

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
  val document = val::global("document");
  val childElem = document.call<val>("getElementById", elem->GetId());
  if (childElem.isNull() || childElem.isUndefined()) {
    return std::unexpected(WebLayout::Error::DOMElementNotFound);
  }
  dom_element.call<void>("removeChild", childElem);

  std::erase(elements, elem);

  return {};
}

size_t WebLayout::GetNumChildren() const { return elements.size(); }

bool WebLayout::ContainsChild(std::shared_ptr<WebElement> elem) const {
  return std::find(elements.begin(), elements.end(), elem) != elements.end();
}

WebLayout& WebLayout::SetDirection(std::string dir) {
  dom_element["style"].set("flexDirection", dir);
  return *this;
}

WebLayout& WebLayout::SetJustifyContent(std::string justify) {
  dom_element["style"].set("justifyContent", justify);
  return *this;
}

WebLayout& WebLayout::SetAlignItems(std::string align) {
  dom_element["style"].set("alignItems", align);
  return *this;
}

WebLayout& WebLayout::SetAlignContent(std::string alignContent) {
  dom_element["style"].set("alignContent", alignContent);
  return *this;
}

WebLayout& WebLayout::SetGap(std::string gap) {
  dom_element["style"].set("gap", gap);
  return *this;
}
}  // namespace cse498

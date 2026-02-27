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

std::expected<void, WebLayout::Error> WebLayout::AddChild(std::shared_ptr<WebElement> elem) {
  if (!elem) {
    return std::unexpected(WebLayout::Error::NullPtr);
  }
  if (ContainsChild(elem)) {
    return std::unexpected(WebLayout::Error::ElementAlreadyMember);
  }

  // Implementation to add child element to the layout
  auto childId = elem->GetId();
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let childId = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        let child = document.getElementById(childId);

        if (layout && child) {
          layout.appendChild(child);
        } else {
          // TODO: Handle error case
          if (!layout) throw `Layout with id ${layoutId} not found!`;
          else if (!child) throw `Child with id ${childId} not found!`;
        }
      },
      id.c_str(), childId.c_str());

  elements.push_back(elem);

  return {};
}

std::expected<void, WebLayout::Error> WebLayout::RemoveChild(std::shared_ptr<WebElement> elem) {
  if (!elem) {
    return std::unexpected(WebLayout::Error::NullPtr);
  }
  if (!ContainsChild(elem)) {
    return std::unexpected(WebLayout::Error::ElementNotFound);
  }

  // Implementation to remove child element from the layout
  auto childId = elem->GetId();
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let childId = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        let child = document.getElementById(childId);
        if (layout && child) {
          layout.removeChild(child);
        }
      },
      id.c_str(), childId.c_str());
  std::erase(elements, elem);

  return {};
}

size_t WebLayout::GetNumChildren() const {
  return elements.size();
}

bool WebLayout::ContainsChild(std::shared_ptr<WebElement> elem) const {
  return std::find(elements.begin(), elements.end(), elem) != elements.end();
}

WebLayout& WebLayout::SetDirection(std::string dir) {
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let direction = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        if (layout) {
          layout.style.flexDirection = direction;
        }
      },
      id.c_str(), dir.c_str());
  return *this;
}

WebLayout& WebLayout::SetJustifyContent(std::string justify) {
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let justifyContent = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        if (layout) {
          layout.style.justifyContent = justifyContent;
        }
      },
      id.c_str(), justify.c_str());
  return *this;
}

WebLayout& WebLayout::SetAlignItems(std::string align) {
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let alignItems = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        if (layout) {
          layout.style.alignItems = alignItems;
        }
      },
      id.c_str(), align.c_str());
  return *this;
}

WebLayout& WebLayout::SetAlignContent(std::string alignContent) {
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let alignContent = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        if (layout) {
          layout.style.alignContent = alignContent;
        }
      },
      id.c_str(), alignContent.c_str());
  return *this;
}

WebLayout& WebLayout::SetGap(std::string gap) {
  EM_ASM(
      {
        let layoutId = UTF8ToString($0);
        let gap = UTF8ToString($1);

        let layout = document.getElementById(layoutId);
        if (layout) {
          layout.style.gap = gap;
        }
      },
      id.c_str(), gap.c_str());
  return *this;
}
}  // namespace cse498

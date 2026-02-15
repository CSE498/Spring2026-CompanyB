/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class to layout web UI elements in a simple flex container.
 * @note Status: PROPOSAL
 **/

#include <emscripten.h>
#include <emscripten/html5.h>

#include "WebLayout.hpp"

namespace cse498
{

  WebLayout::WebLayout(std::string id) : WebElement(id)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let layout = document.getElementById(layoutId);
          if (!layout)
          {
            layout = document.createElement('div');
            layout.id = layoutId;
            layout.style.display = 'flex';
            document.body.appendChild(layout);
          }
        },
        id.c_str());
  }

  WebLayout &WebLayout::AddChild(std::shared_ptr<WebElement> elem)
  {
    // Implementation to add child element to the layout
    auto childId = elem->GetId();
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let childId = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          let child = document.getElementById(childId);
          if (layout && child)
          {
            layout.appendChild(child);
          }
        },
        id.c_str(), childId.c_str());
    elements.push_back(elem);
    return *this;
  }

  WebLayout &WebLayout::RemoveChild(std::shared_ptr<WebElement> elem)
  {
    // Implementation to remove child element from the layout
    auto childId = elem->GetId();
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let childId = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          let child = document.getElementById(childId);
          if (layout && child)
          {
            layout.removeChild(child);
          }
        },
        id.c_str(), childId.c_str());
    elements.erase(std::remove(elements.begin(), elements.end(), elem), elements.end());
    return *this;
  }

  WebLayout &WebLayout::SetDirection(std::string dir)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let direction = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          if (layout)
          {
            layout.style.flexDirection = direction;
          }
        },
        id.c_str(), dir.c_str());
    return *this;
  }

  WebLayout &WebLayout::SetJustifyContent(std::string justify)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let justifyContent = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          if (layout)
          {
            layout.style.justifyContent = justifyContent;
          }
        },
        id.c_str(), justify.c_str());
    return *this;
  }

  WebLayout &WebLayout::SetAlignItems(std::string align)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let alignItems = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          if (layout)
          {
            layout.style.alignItems = alignItems;
          }
        },
        id.c_str(), align.c_str());
    return *this;
  }

  WebLayout &WebLayout::SetAlignContent(std::string alignContent)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let alignContent = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          if (layout)
          {
            layout.style.alignContent = alignContent;
          }
        },
        id.c_str(), alignContent.c_str());
    return *this;
  }

  WebLayout &WebLayout::SetGap(std::string gap)
  {
    EM_ASM(
        {
          let layoutId = UTF8ToString($0);
          let gap = UTF8ToString($1);

          let layout = document.getElementById(layoutId);
          if (layout)
          {
            layout.style.gap = gap;
          }
        },
        id.c_str(), gap.c_str());
    return *this;
  }
}

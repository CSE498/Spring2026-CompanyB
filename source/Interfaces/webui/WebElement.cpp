#pragma once

#include <emscripten.h>
#include <emscripten/html5.h>

#include "WebElement.hpp"

namespace cse498 {

WebElement::WebElement(const std::string& id) : id(id) {
  // Create the element in the DOM if it doesn't already exist
  EM_ASM(
      {
        let elemId = UTF8ToString($0);
        let elem = document.getElementById(elemId);
        if (!elem)
        {
          elem = document.createElement('div');
          elem.id = elemId;
          document.body.appendChild(elem);
        }
      },
      id.c_str());
}

}  // namespace cse498
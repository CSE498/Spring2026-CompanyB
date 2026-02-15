/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class to layout web UI elements in a simple flex container.
 * @note Status: PROPOSAL
 **/

#pragma once

#include "WebElement.hpp"
#include <utility>
#include <memory>
#include <vector>

namespace cse498 {

class WebLayout : WebElement {
protected:
  std::vector<std::shared_ptr<WebElement>> elements;

public:
  // Create a container in the DOM for the layout
  WebLayout(std::string id);

  WebLayout& AddChild(std::shared_ptr<WebElement> elem);
  WebLayout& RemoveChild(std::shared_ptr<WebElement> elem);

  WebLayout& SetDirection(std::string dir);
  WebLayout& SetJustifyContent(std::string justify);
  WebLayout& SetAlignItems(std::string align);
  WebLayout& SetAlignContent(std::string alignContent);
  WebLayout& SetGap(std::string gap);
};

} // End of namespace cse498

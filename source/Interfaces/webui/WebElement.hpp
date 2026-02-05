/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief
 * @note Status: PROPOSAL
 **/

#pragma once

#include <string>

namespace cse498
{
  class WebElement
  {
  protected:
    std::string id = "";

  public:
    WebElement() = delete;
    // Create an element with the given ID in the DOM
    WebElement(const std::string &id);
    virtual ~WebElement() = default;

    [[nodiscard]] std::string GetId() const { return id; }
  };
}
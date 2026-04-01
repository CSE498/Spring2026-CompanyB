#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <print>

#include "tools/webui/WebLayout.hpp"

struct SetupMockDOMWebLayout {
  SetupMockDOMWebLayout() {
    // clang-format off
    EM_ASM({
      /// Creates a document if it doesn't exist
      if (typeof document === 'undefined') {
        const { JSDOM } = jsdom;
        const dom = new JSDOM("<!DOCTYPE html> <html><head></head><body></body></html>");
        globalThis.window = dom.window;
        globalThis.document = dom.window.document;
      }
    });
    // clang-format on
  }

  ~SetupMockDOMWebLayout() {
    // clang-format off
      EM_ASM({
        delete globalThis.document;
        delete globalThis.window;
      });
    // clang-format on
  }
};

using namespace cse498;

// -------- Utilities for testing WebLayout --------

/**
 * Helper function that checks if an element of given ID is present in the DOM
 */
int ELEMENT_PRESENT(std::string elemId) {
  int elementPresent = EM_ASM_INT(
      {
        const id = UTF8ToString($0);
        const elem = document.getElementById(id);
        if (elem) {
          return 1;
        } else
          return 0;
      },
      elemId.c_str());

  return elementPresent;
}

/**
 * Checks all invariants after adding an element to a WebLayout
 */
void TEST_ADD_ELEMENT(WebLayout& layout, std::shared_ptr<WebElement> elem) {
  auto lengthBefore = layout.GetNumChildren();

  REQUIRE(layout.AddChild(elem).has_value());

  REQUIRE(layout.GetNumChildren() == lengthBefore + 1);

  // Check that element is in the DOM, and it's parent is the layout
  std::string layoutId = layout.GetId().value();
  std::string elemId = elem->GetId().value();
  int domCheck = EM_ASM_INT(
      {
        const layoutId = UTF8ToString($0);
        const elemId = UTF8ToString($1);

        const elem = document.getElementById(elemId);

        if (!elem) return 0;
        if (elem.parentElement?.id != layoutId) return 0;

        return 1;
      },
      layoutId.c_str(), elemId.c_str());

  REQUIRE(domCheck == 1);
}

/**
 * Checks all invariants after removing an element from a WebLayout
 */
void TEST_REMOVE_ELEMENT(WebLayout& layout, std::shared_ptr<WebElement> elem) {
  auto lengthBefore = layout.GetNumChildren();

  // Print elements of WebLayout
  // std::println("Elements before: ");
  // for (auto& elem : layout.elements) {
  //   std::println("{}", elem->GetId());
  // }

  REQUIRE(layout.RemoveChild(elem).has_value());

  REQUIRE(layout.GetNumChildren() == lengthBefore - 1);

  // Child should not be in the DOM anymore
  // REQUIRE(ELEMENT_PRESENT(elem->GetId()) == 0);
}

// -------- Tests for WebLayout --------

TEST_CASE("WebLayout constructor creates DOM element with ID", "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  REQUIRE(layout.GetId().value_or("") == "layout1");

  int layoutPresent = EM_ASM_INT(
      {
        const id = UTF8ToString($0);
        const elem = document.getElementById(id);

        if (elem) {
          return 1;
        } else
          return 0;
      },
      layout.GetId().value().c_str());

  REQUIRE(layoutPresent == 1);
}

TEST_CASE("WebLayout DOM element is removed on object being destructed",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  {
    WebLayout layout("layout1");
  }

  std::string id = "layout1";

  REQUIRE(ELEMENT_PRESENT("layout1") == 0);
}

TEST_CASE("WebLayout can add and remove children", "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  auto elem1 = std::make_shared<WebElement>("div", "elem1");
  auto elem2 = std::make_shared<WebElement>("div", "elem2");

  TEST_ADD_ELEMENT(layout, elem1);
  TEST_ADD_ELEMENT(layout, elem2);

  TEST_REMOVE_ELEMENT(layout, elem1);
  TEST_REMOVE_ELEMENT(layout, elem2);
}

TEST_CASE("WebLayout: adding or removing null pointers is an error",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  auto elem1 = std::make_shared<WebElement>("div", "elem1");
  auto elem2 = std::shared_ptr<WebElement>();

  TEST_ADD_ELEMENT(layout, elem1);

  REQUIRE(layout.AddChild(elem2).error() == WebLayout::Error::NullPtr);
  REQUIRE(layout.RemoveChild(elem2).error() == WebLayout::Error::NullPtr);
}

TEST_CASE("WebLayout: adding the same element twice gives an error",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  auto elem1 = std::make_shared<WebElement>("div", "elem1");

  TEST_ADD_ELEMENT(layout, elem1);

  REQUIRE(layout.AddChild(elem1).error() ==
          WebLayout::Error::ElementAlreadyMember);
}

TEST_CASE("WebLayout can set properties with chained function calls",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  REQUIRE_NOTHROW(layout.SetDirection("column")
                      .SetJustifyContent("flex-start")
                      .SetAlignItems("center")
                      .SetAlignContent("center")
                      .SetGap("5px")
                      .SetHeight("100px"));
}

TEST_CASE("WebLayout properties can be retrieved via getters", "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout2");

  layout.SetDirection("row");
  CHECK(layout.GetDirection() == "row");

  layout.SetJustifyContent("space-between");
  CHECK(layout.GetJustifyContent() == "space-between");

  layout.SetAlignItems("stretch");
  CHECK(layout.GetAlignItems() == "stretch");

  layout.SetAlignContent("flex-end");
  CHECK(layout.GetAlignContent() == "flex-end");

  layout.SetGap("1rem");
  CHECK(layout.GetGap() == "1rem");

  layout.SetHeight("50%");
  CHECK(layout.GetHeight() == "50%");
}

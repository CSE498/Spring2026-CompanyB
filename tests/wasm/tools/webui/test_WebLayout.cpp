#include <emscripten.h>
#include <memory>
#include <print>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebLayout.hpp"

struct SetupMockDOMWebLayout {
  SetupMockDOMWebLayout() {
    // clang-format off
    EM_ASM({
      /// creates a document if it doesn't exist
      if (typeof document === 'undefined') {
        globalThis.document = {};
      }

      /// Store created DOM elements for retrieval by ID
      var elements = {};

      /// Mock implementation of document.getElementById and
      /// document.createElement
      document.getElementById = function(id) { return elements[id] || null; };

      document.createElement = function(tag) {
        var elem = {};

        /// Create style object
        elem.style = {};
        elem.style.position = "";
        elem.style.width = "";
        elem.style.height = "";
        elem.style.left = "";
        elem.style.top = "";
        elem.style.display = "";

        // Allow appending children to other elements
        elem.childNodes = [];
        elem.appendChild = function(child) {
          if (!elem.childNodes.includes(child)) {
            elem.childNodes.push(child);
            child.parentElement = elem;
          }
        };
        elem.removeChild = function(child) {
          const index = elem.childNodes.indexOf(child);
          if (index > -1) { 
            elem.childNodes.splice(index, 1);
          }
        };

        /// Define getters/setters for id to manage the elements map
        Object.defineProperty(elem, 'id', {
          get: function() { return elem._id; },
          set : function(v) {
            elem._id = v;
            elements[v] = elem;
          }
        });

        /// Define a remove method to clean up the elements map when an element is
        /// removed
        elem.remove = function() {
          if (elem._id && elements[elem._id]) {
            delete elements[elem._id];
          }
        };

        return elem;
      };

      /// Mock implementation of document.body and appendChild
      if (!document.body) {
        document.body = {};
      }

      document.body.appendChild = function(elem) { return elem; };
    });
    // clang-format on
    }

    ~SetupMockDOMWebLayout() {
      // clang-format off
    EM_ASM({
      delete globalThis.document;
      delete globalThis.window;
      delete globalThis.Image;
    });
      // clang-format on
    }
    }
    ;

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

  REQUIRE(layout.GetNumChildren() == lengthBefore+1);

  // Check that element is in the DOM, and it's parent is the layout
  std::string layoutId = layout.GetId();
  std::string elemId = elem->GetId();
  int domCheck = EM_ASM_INT(
      {
        const layoutId = UTF8ToString($0);
        const elemId = UTF8ToString($1);
        
        const elem = document.getElementById(elemId);

        if (!elem) return 0;
        if (elem.parentElement?.id != layoutId) return 0;

        return 1;
      },
      layoutId.c_str(),
      elemId.c_str());

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

  REQUIRE(layout.GetNumChildren() == lengthBefore-1);
  
  // Child should not be in the DOM anymore 
  // REQUIRE(ELEMENT_PRESENT(elem->GetId()) == 0);
}

// -------- Tests for WebLayout --------

TEST_CASE("WebLayout constructor creates DOM element with ID",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  REQUIRE(layout.GetId() == "layout1");

  int layoutPresent = EM_ASM_INT(
      {
        const id = UTF8ToString($0);
        const elem = document.getElementById(id);

        if (elem) {
          return 1;
        } else
          return 0;
      },
      layout.GetId().c_str());

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

  auto elem1 = std::make_shared<WebElement>("elem1");
  auto elem2 = std::make_shared<WebElement>("elem2");

  TEST_ADD_ELEMENT(layout, elem1);
  TEST_ADD_ELEMENT(layout, elem2);

  TEST_REMOVE_ELEMENT(layout, elem1);
  TEST_REMOVE_ELEMENT(layout, elem2);
}

TEST_CASE("WebLayout: adding element twice is error", "[WebLayout]") {
  // TODO: Implement test
}

TEST_CASE("WebLayout can set properties with chained function calls",
          "[WebLayout]") {
  SetupMockDOMWebLayout mock;
  WebLayout layout("layout1");

  REQUIRE_NOTHROW(layout.SetDirection("column")
                      .SetJustifyContent("flex-start")
                      .SetAlignItems("center")
                      .SetAlignContent("center")
                      .SetGap("5px"));
}

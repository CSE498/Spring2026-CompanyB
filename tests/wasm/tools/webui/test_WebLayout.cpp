#include <emscripten.h>
#include <memory>

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

      int layoutPresent = EM_ASM_INT(
          {
            const id = UTF8ToString($0);
            const elem = document.getElementById(id);
            if (elem) {
              return 1;
            } else
              return 0;
          },
          id.c_str());

      REQUIRE(layoutPresent == 0);
    }

    TEST_CASE("WebLayout can add and remove children", "[WebLayout]") {
      SetupMockDOMWebLayout mock;
      WebLayout layout("layout1");

      auto elem1 = std::make_shared<WebElement>("elem1");
      auto elem2 = std::make_shared<WebElement>("elem2");

      REQUIRE_NOTHROW(layout.AddChild(elem1));
      REQUIRE_NOTHROW(layout.AddChild(elem2));
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

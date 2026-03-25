
#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebButton.h"

using namespace cse498;

struct SetupMockDOM {
  SetupMockDOM() {
    // clang-format off
    EM_ASM({
      if (typeof document === 'undefined') {
        globalThis.document = {};
      }

      var elements = {};

      document.getElementById = function(id) { return elements[id] || null; };

      document.createElement = function(tag) {
        var elem = {};
        elem._id = "";
        elem.innerText = "";
        elem.disabled = false;
        elem.style = {};
        elem.style.display = "";

        Object.defineProperty(elem, 'id', {
          get: function() { return elem._id; },
          set: function(v) {
            elem._id = v;
            elements[v] = elem;
          }
        });

        elem.remove = function() {
          if (elem._id && elements[elem._id]) {
            delete elements[elem._id];
          }
        };

        return elem;
      };

      if (!document.body) {
        document.body = {};
      }

      document.body.appendChild = function(elem) { return elem; };
      document.body.removeChild = function(elem) {};

    });
    // clang-format on
    }

    ~SetupMockDOM() {
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

    TEST_CASE("WebButton basics") {
      SetupMockDOM mock;
      cse498::WebButton b("Start", "startBtn");

      SECTION("Label set/get works") {
        REQUIRE(b.GetLabel() == "Start");
        b.SetLabel("Pause");
        REQUIRE(b.GetLabel() == "Pause");
      }

      SECTION("Visibility works") {
        REQUIRE(b.IsVisible());
        b.Hide();
        REQUIRE_FALSE(b.IsVisible());
        b.Show();
        REQUIRE(b.IsVisible());
      }

      SECTION("Enable/Disable works") {
        REQUIRE(b.IsEnabled());
        b.Disable();
        REQUIRE_FALSE(b.IsEnabled());
        b.Enable();
        REQUIRE(b.IsEnabled());
      }

      SECTION("Click runs callback only when enabled and visible") {
        int count = 0;
        b.SetOnClick([&count]() { ++count; });

        b.Click();
        REQUIRE(count == 1);

        b.Disable();
        b.Click();
        REQUIRE(count == 1);

        b.Enable();
        b.Hide();
        b.Click();
        REQUIRE(count == 1);

        b.Show();
        b.Click();
        REQUIRE(count == 2);
      }
    }
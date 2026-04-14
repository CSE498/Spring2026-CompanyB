
#include <emscripten.h>
#include <emscripten/val.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebButton.h"

using namespace cse498;

struct SetupMockDOMWebButton {
  SetupMockDOMWebButton() {
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

  ~SetupMockDOMWebButton() {
    // clang-format off
      EM_ASM({
        delete globalThis.document;
        delete globalThis.window;
      });
    // clang-format on
  }
};

TEST_CASE("WebButton basics") {
  SetupMockDOMWebButton mock;
  cse498::WebButton b("Start", { .id = "startBtn" });

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

    emscripten::val arg;

    b.Click(arg);
    REQUIRE(count == 1);

    b.Disable();
    b.Click(arg);
    REQUIRE(count == 1);

    b.Enable();
    b.Hide();
    b.Click(arg);
    REQUIRE(count == 1);

    b.Show();
    b.Click(arg);
    REQUIRE(count == 2);
  }
}
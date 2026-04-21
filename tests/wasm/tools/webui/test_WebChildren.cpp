#include <emscripten.h>
#include <emscripten/val.h>

#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "tools/webui/WebButton.h"
#include "tools/webui/WebLayout.hpp"
#include "tools/webui/WebOptions.hpp"

using namespace cse498;

struct SetupMockDOMWebChildren {
  SetupMockDOMWebChildren() {
    // clang-format off
    EM_ASM({
      const { JSDOM } = jsdom;
      const dom = new JSDOM("<!DOCTYPE html> <html><head></head><body></body></html>");
      globalThis.window = dom.window;
      globalThis.document = dom.window.document;
    });
    // clang-format on
  }

  ~SetupMockDOMWebChildren() {
    // clang-format off
    EM_ASM({
      if (globalThis.window && globalThis.window.close) {
        globalThis.window.close();
      }
      delete globalThis.document;
      delete globalThis.window;
    });
    // clang-format on
  }
};

TEST_CASE("WebLayout adds children from WebOptions",
          "[WebLayout][WebOptions]") {
  SetupMockDOMWebChildren mock;

  auto layout = std::make_shared<WebLayout>(WebOptions{
      .id = "parent-layout",
      .children = {
          UIItem<WebButton>("Btn 1", WebOptions{.id = "child-btn-1"}),
          UIItem<WebButton>("Btn 2", WebOptions{.id = "child-btn-2"})}});

  REQUIRE(layout->GetNumChildren() == 2);

  emscripten::val document = emscripten::val::global("document");
  emscripten::val btn1 = document.call<emscripten::val>(
      "getElementById", std::string("child-btn-1"));
  emscripten::val btn2 = document.call<emscripten::val>(
      "getElementById", std::string("child-btn-2"));

  REQUIRE_FALSE(btn1.isNull());
  REQUIRE_FALSE(btn2.isNull());

  REQUIRE(btn1["parentElement"]["id"].as<std::string>() == "parent-layout");
  REQUIRE(btn2["parentElement"]["id"].as<std::string>() == "parent-layout");
}

#include <emscripten.h>
#include <emscripten/val.h>
#include <catch2/catch_test_macros.hpp>
#include "tools/webui/WebElement.hpp"
#include "tools/webui/WebStyle.hpp"

using namespace cse498;

struct SetupMockDOMWebStyle {
  SetupMockDOMWebStyle() {
    // clang-format off
    EM_ASM({
      if (typeof document === 'undefined') {
        const { JSDOM } = jsdom;
        const dom = new JSDOM("<!DOCTYPE html> <html><head></head><body></body></html>");
        globalThis.window = dom.window;
        globalThis.document = dom.window.document;
      }
    });
    // clang-format on
  }

  ~SetupMockDOMWebStyle() {
    // clang-format off
    EM_ASM({
      delete globalThis.document;
      delete globalThis.window;
    });
    // clang-format on
  }
};

TEST_CASE("WebStyle application to WebElement", "[WebStyle]") {
  SetupMockDOMWebStyle mock;
  
  WebStyle style({
    {"backgroundColor", "red"},
    {"textAlign", "center"},
    {"fontSize", "20px"}
  });

  WebElement elem("div", "style-test-elem", style);

  emscripten::val document = emscripten::val::global("document");
  emscripten::val dom_elem = document.call<emscripten::val>("getElementById", std::string("style-test-elem"));

  REQUIRE(dom_elem["style"]["backgroundColor"].as<std::string>() == "red");
  REQUIRE(dom_elem["style"]["textAlign"].as<std::string>() == "center");
  REQUIRE(dom_elem["style"]["fontSize"].as<std::string>() == "20px");
}

TEST_CASE("WebStyle direct initialization in constructor", "[WebStyle]") {
  SetupMockDOMWebStyle mock;
  
  // Test direct initialization with initializer list in constructor
  WebElement elem("div", "direct-style-test", {
    {"color", "blue"},
    {"margin", "10px"}
  });

  emscripten::val document = emscripten::val::global("document");
  emscripten::val dom_elem = document.call<emscripten::val>("getElementById", std::string("direct-style-test"));

  REQUIRE(dom_elem["style"]["color"].as<std::string>() == "blue");
  REQUIRE(dom_elem["style"]["margin"].as<std::string>() == "10px");
}

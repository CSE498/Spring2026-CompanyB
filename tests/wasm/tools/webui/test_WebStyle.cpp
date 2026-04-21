#include <emscripten.h>
#include <emscripten/val.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebElement.hpp"
#include "tools/webui/WebOptions.hpp"

using namespace cse498;

struct SetupMockDOMWebStyle {
  SetupMockDOMWebStyle() {
    // clang-format off
    EM_ASM({
      const { JSDOM } = jsdom;
      const dom = new JSDOM("<!DOCTYPE html> <html><head></head><body></body></html>");
      globalThis.window = dom.window;
      globalThis.document = dom.window.document;
    });
    // clang-format on
  }

  ~SetupMockDOMWebStyle() {
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

TEST_CASE("WebStyle application to WebElement", "[WebStyle]") {
  SetupMockDOMWebStyle mock;

  WebOptions options = {.id = "style-test-elem",
                        .style = {{"backgroundColor", "red"},
                                  {"textAlign", "center"},
                                  {"fontSize", "20px"}}};

  WebElement elem("div", options);

  emscripten::val document = emscripten::val::global("document");
  emscripten::val dom_elem = document.call<emscripten::val>(
      "getElementById", std::string("style-test-elem"));

  REQUIRE(dom_elem["style"]["backgroundColor"].as<std::string>() == "red");
  REQUIRE(dom_elem["style"]["textAlign"].as<std::string>() == "center");
  REQUIRE(dom_elem["style"]["fontSize"].as<std::string>() == "20px");
}

TEST_CASE("WebStyle direct initialization in constructor", "[WebStyle]") {
  SetupMockDOMWebStyle mock;

  // Test direct initialization with initializer list in constructor
  WebElement elem("div", {.id = "direct-style-test",
                          .style = {{"color", "blue"}, {"margin", "10px"}}});

  emscripten::val document = emscripten::val::global("document");
  emscripten::val dom_elem = document.call<emscripten::val>(
      "getElementById", std::string("direct-style-test"));

  REQUIRE(dom_elem["style"]["color"].as<std::string>() == "blue");
  REQUIRE(dom_elem["style"]["margin"].as<std::string>() == "10px");
}

TEST_CASE("WebOptions classes application to WebElement", "[WebOptions]") {
  SetupMockDOMWebStyle mock;

  WebOptions options = {.id = "class-test-elem",
                        .classes = {"test-class-1", "test-class-2"}};

  WebElement elem("div", options);

  emscripten::val document = emscripten::val::global("document");
  emscripten::val dom_elem = document.call<emscripten::val>(
      "getElementById", std::string("class-test-elem"));
  emscripten::val class_list = dom_elem["classList"];

  REQUIRE(class_list.call<bool>("contains", std::string("test-class-1")));
  REQUIRE(class_list.call<bool>("contains", std::string("test-class-2")));
}

TEST_CASE("WebElement AddClass, RemoveClass, and SetStyle", "[WebElement]") {
  SetupMockDOMWebStyle mock;
  WebElement elem("div", {.id = "dynamic-test-elem"});

  SECTION("AddClass") {
    elem.AddClass("new-class");
    emscripten::val document = emscripten::val::global("document");
    emscripten::val dom_elem = document.call<emscripten::val>(
        "getElementById", std::string("dynamic-test-elem"));
    REQUIRE(
        dom_elem["classList"].call<bool>("contains", std::string("new-class")));
  }

  SECTION("RemoveClass") {
    elem.AddClass("to-remove");
    elem.RemoveClass("to-remove");
    emscripten::val document = emscripten::val::global("document");
    emscripten::val dom_elem = document.call<emscripten::val>(
        "getElementById", std::string("dynamic-test-elem"));
    REQUIRE_FALSE(
        dom_elem["classList"].call<bool>("contains", std::string("to-remove")));
  }

  SECTION("SetStyle") {
    elem.SetStyle({{"color", "green"}, {"border", "1px solid black"}});
    emscripten::val document = emscripten::val::global("document");
    emscripten::val dom_elem = document.call<emscripten::val>(
        "getElementById", std::string("dynamic-test-elem"));
    REQUIRE(dom_elem["style"]["color"].as<std::string>() == "green");
    REQUIRE(dom_elem["style"]["border"].as<std::string>() == "1px solid black");
  }
}

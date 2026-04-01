#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebImage.hpp"
#include <emscripten/val.h>

struct SetupMockDOMWebImage {
  SetupMockDOMWebImage() {
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

  ~SetupMockDOMWebImage() {
    // clang-format off
      EM_ASM({
        delete globalThis.document;
        delete globalThis.window;
      });
    // clang-format on
  }
};

/// @brief Utility for testing images. 
/// `complete` and `naturalWidth` are read-only in image elements in DOM, this makes them writeable for an image element
void patch_image_writable(emscripten::val elem) {
  emscripten::val descriptor_complete = emscripten::val::object();
  descriptor_complete.set("value", true);
  descriptor_complete.set("writable", true); // Make it writable for your test
  emscripten::val::global("Object").call<void>("defineProperty", elem, emscripten::val("complete"), descriptor_complete);

  emscripten::val descriptor_naturalWidth = emscripten::val::object();
  descriptor_naturalWidth.set("value", 0);
  descriptor_naturalWidth.set("writable", true); // Make it writable for your test
  emscripten::val::global("Object").call<void>("defineProperty", elem, emscripten::val("naturalWidth"), descriptor_naturalWidth);
}

TEST_CASE("WebImage constructor creates image with correct properties",
          "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("assets/test.png", "A test image", "test-img-1");

  REQUIRE(img.GetId().has_value());
  REQUIRE(img.GetId().value() == "test-img-1");
  REQUIRE(img.GetSource() == "assets/test.png");
  REQUIRE(img.GetAlt() == "A test image");
  REQUIRE(img.GetWidth() == 0);
  REQUIRE(img.GetHeight() == 0);
}

TEST_CASE("WebImage SetSource changes and properly updates the image source",
          "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("initial.png", "Initial image", "test-img-2");

  img.SetSource("new_image.png");

  REQUIRE(img.GetSource() == "new_image.png");

  img.SetSource("another_image.jpg");

  REQUIRE(img.GetSource() == "another_image.jpg");
}

TEST_CASE("WebImage SetSize updates dimensions correctly", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "Size test image", "test-img-3");

  img.SetSize(200, 150);

  REQUIRE(img.GetWidth() == 200);
  REQUIRE(img.GetHeight() == 150);

  img.SetSize(300, 250);

  REQUIRE(img.GetWidth() == 300);
  REQUIRE(img.GetHeight() == 250);
}

TEST_CASE("WebImage GetSize returns correct dimensions and updates properly",
          "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "GetSize test image", "test-img-4");

  img.SetSize(300, 250);
  auto [width, height] = img.GetSize();

  REQUIRE(width == 300);
  REQUIRE(height == 250);

  img.SetSize(400, 350);
  auto [width2, height2] = img.GetSize();

  REQUIRE(width2 == 400);
  REQUIRE(height2 == 350);
}

TEST_CASE("WebImage SetPosition updates position correctly", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "Position test image", "test-img-5");

  img.SetPosition(100, 50);
  auto [x, y] = img.GetPosition();

  REQUIRE(x == 100);
  REQUIRE(y == 50);

  img.SetPosition(200, 150);
  auto [x2, y2] = img.GetPosition();

  REQUIRE(x2 == 200);
  REQUIRE(y2 == 150);
}

TEST_CASE("WebImage SetAlt updates alt text properly", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "Initial alt text", "test-img-6");

  img.SetAlt("A test image");

  REQUIRE(img.GetAlt() == "A test image");

  img.SetAlt("Updated alt text");

  REQUIRE(img.GetAlt() == "Updated alt text");
}

TEST_CASE("WebImage handles multiple property changes", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("original.png",
                       "Multi-property test image", "test-img-7");

  img.SetSource("updated.png");
  img.SetSize(400, 300);
  img.SetPosition(50, 75);
  img.SetAlt("Updated image");

  REQUIRE(img.GetSource() == "updated.png");
  REQUIRE(img.GetWidth() == 400);
  REQUIRE(img.GetHeight() == 300);
  auto [x, y] = img.GetPosition();
  REQUIRE(x == 50);
  REQUIRE(y == 75);
  REQUIRE(img.GetAlt() == "Updated image");
}

TEST_CASE("WebImage IsLoaded returns false for unloaded images",
          "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "Loading test image", "test-img-loading");

  REQUIRE(img.IsLoaded() == false);
}

TEST_CASE("WebImage SetSize with zero values", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png",
                       "Zero size test image", "test-img-zero-size");

  img.SetSize(0, 0);

  REQUIRE(img.GetWidth() == 0);
  REQUIRE(img.GetHeight() == 0);
}

TEST_CASE("WebImage handles empty strings", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("", "Empty source test", "test-img-empty");

  REQUIRE(img.GetSource() == "");

  img.SetAlt("");
  REQUIRE(img.GetAlt() == "");
}

TEST_CASE(
    "WebImage HasError returns unexpected with error message when complete = "
    "true but naturalWidth = 0",
    "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("bad-path.png",
                       "Error test image", "test-img-has-error");

  // Since there are no setters for these properties, manually edit the mock
  // element and simulate a failed load
  emscripten::val document = emscripten::val::global("document");
  emscripten::val elem = document.call<emscripten::val>(
      "getElementById", std::string("test-img-has-error"));

  patch_image_writable(elem);
  elem.set("complete", true);
  elem.set("naturalWidth", 0);

  auto result = img.HasError();
  
  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error().find("bad-path.png") != std::string::npos);
}

TEST_CASE(
    "WebImage HasError returns empty expected for a successfully loaded "
    "image",
    "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("good-image.png",
                       "Loaded ok test image", "test-img-loaded-ok");

  // Since there are no setters for these properties, manually edit the mock
  // element and simulate a successful load
  emscripten::val document = emscripten::val::global("document");
  emscripten::val elem = document.call<emscripten::val>(
      "getElementById", std::string("test-img-loaded-ok"));

  patch_image_writable(elem);
  elem.set("complete", true);
  elem.set("naturalWidth", 200);

  REQUIRE(elem["complete"].as<bool>() == true);
  REQUIRE(elem["naturalWidth"].as<int>() == 200);

  REQUIRE(img.HasError().has_value());
}

TEST_CASE("WebImage SetSize respects SizeUnit enum", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png", "Unit test image", "test-img-units");

  img.SetSize(50, 25, cse498::SizeUnit::percent);
  REQUIRE(img.GetWidth() == 50);
  REQUIRE(img.GetHeight() == 25);

  img.SetSize(10, 5, cse498::SizeUnit::em);
  REQUIRE(img.GetWidth() == 10);
  REQUIRE(img.GetHeight() == 5);

  img.SetSize(20, 15, cse498::SizeUnit::vw);
  REQUIRE(img.GetWidth() == 20);
  REQUIRE(img.GetHeight() == 15);
}

TEST_CASE("WebImage SetSize supports fractional dimensions", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png",
                       "Fractional size test image", "test-img-fractional");

  img.SetSize(150.5, 75.25);
  REQUIRE(img.GetWidth() == 150.5);
  REQUIRE(img.GetHeight() == 75.25);
}

TEST_CASE("WebImage SetSize supports all SizeUnit values", "[WebImage]") {
  SetupMockDOMWebImage mock;
  cse498::WebImage img("test.png",
                       "All units test image", "test-img-all-units");

  img.SetSize(100, 50, cse498::SizeUnit::px);
  REQUIRE(img.GetWidth() == 100);
  REQUIRE(img.GetHeight() == 50);

  img.SetSize(10, 5, cse498::SizeUnit::em);
  REQUIRE(img.GetWidth() == 10);
  REQUIRE(img.GetHeight() == 5);

  img.SetSize(20, 15, cse498::SizeUnit::rem);
  REQUIRE(img.GetWidth() == 20);
  REQUIRE(img.GetHeight() == 15);

  img.SetSize(50, 25, cse498::SizeUnit::percent);
  REQUIRE(img.GetWidth() == 50);
  REQUIRE(img.GetHeight() == 25);

  img.SetSize(30, 20, cse498::SizeUnit::vw);
  REQUIRE(img.GetWidth() == 30);
  REQUIRE(img.GetHeight() == 20);

  img.SetSize(40, 30, cse498::SizeUnit::vh);
  REQUIRE(img.GetWidth() == 40);
  REQUIRE(img.GetHeight() == 30);
}
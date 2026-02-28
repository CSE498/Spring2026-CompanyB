#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebImage.hpp"

/// Creates fake DOM elements for WebImage tests to run on.
struct SetupMockDOM {
  SetupMockDOM() {
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
        /// Initialize properties for <img> elements
        elem._id = "";
        elem.src = "";
        elem.alt = "";
        elem.complete = false;
        elem.naturalWidth = 100;

        /// Create style object
        elem.style = {};
        elem.style.position = "";
        elem.style.width = "";
        elem.style.height = "";
        elem.style.left = "";
        elem.style.top = "";
        elem.style.display = "";

        /// Define getters/setters for id to manage the elements map
      Object.defineProperty(elem, 'id', {
        get: function() { return elem._id;
      }
      , set : function(v) {
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

  TEST_CASE("WebImage constructor creates image with correct properties",
            "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-1", "assets/test.png");

    REQUIRE(img.GetId() == "test-img-1");
    REQUIRE(img.GetSource() == "assets/test.png");
    REQUIRE(img.GetWidth() == 0);
    REQUIRE(img.GetHeight() == 0);
  }

  TEST_CASE("WebImage SetSource changes and properly updates the image source",
            "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-2", "initial.png");

    img.SetSource("new_image.png");

    REQUIRE(img.GetSource() == "new_image.png");

    img.SetSource("another_image.jpg");

    REQUIRE(img.GetSource() == "another_image.jpg");
  }

  TEST_CASE("WebImage SetSize updates dimensions correctly", "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-3", "test.png");

    img.SetSize(200, 150);

    REQUIRE(img.GetWidth() == 200);
    REQUIRE(img.GetHeight() == 150);

    img.SetSize(300, 250);

    REQUIRE(img.GetWidth() == 300);
    REQUIRE(img.GetHeight() == 250);
  }

  TEST_CASE("WebImage GetSize returns correct dimensions and updates properly",
            "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-4", "test.png");

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
    SetupMockDOM mock;
    cse498::WebImage img("test-img-5", "test.png");

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
    SetupMockDOM mock;
    cse498::WebImage img("test-img-6", "test.png");

    img.SetAlt("A test image");

    REQUIRE(img.GetAlt() == "A test image");

    img.SetAlt("Updated alt text");

    REQUIRE(img.GetAlt() == "Updated alt text");
  }

  TEST_CASE("WebImage handles multiple property changes", "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-7", "original.png");

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
    SetupMockDOM mock;
    cse498::WebImage img("test-img-loading", "test.png");

    REQUIRE(img.IsLoaded() == false);
  }

  TEST_CASE("WebImage SetSize with zero values", "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-zero-size", "test.png");

    img.SetSize(0, 0);

    REQUIRE(img.GetWidth() == 0);
    REQUIRE(img.GetHeight() == 0);
  }

  TEST_CASE("WebImage handles empty strings", "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-empty", "");

    REQUIRE(img.GetSource() == "");

    img.SetAlt("");
    REQUIRE(img.GetAlt() == "");
  }

  TEST_CASE(
      "WebImage HasError returns true when complete = true but naturalWidth = "
      "0",
      "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-has-error", "bad-path.png");

    // Since there are no setters for these properties, manually edit the mock
    // element and simulate a failed load
    emscripten::val document = emscripten::val::global("document");
    emscripten::val elem = document.call<emscripten::val>(
        "getElementById", std::string("test-img-has-error"));
    elem.set("complete", true);
    elem.set("naturalWidth", 0);

    REQUIRE(img.HasError() == true);
  }

  TEST_CASE("WebImage HasError returns false for a successfully loaded image",
            "[WebImage]") {
    SetupMockDOM mock;
    cse498::WebImage img("test-img-loaded-ok", "good-image.png");

    // Since there are no setters for these properties, manually edit the mock
    // element and simulate a successful load
    emscripten::val document = emscripten::val::global("document");
    emscripten::val elem = document.call<emscripten::val>(
        "getElementById", std::string("test-img-loaded-ok"));
    elem.set("complete", true);
    elem.set("naturalWidth", 200);

    REQUIRE(img.HasError() == false);
  }

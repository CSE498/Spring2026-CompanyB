#include <emscripten.h>
#include <catch2/catch_test_macros.hpp>
#include "tools/webui/WebImage.hpp"

/// Creates fake DOM elements for WebImage tests to run on.
struct SetupMockDOM {
    SetupMockDOM(){
  EM_ASM({
    ///creates a document if it doesn't exist
    if (typeof document === 'undefined') {
      globalThis.document = {};
    }

    /// Store created DOM elements for retrieval by ID
    var elements = {};

    /// Mock implementation of document.getElementById and document.createElement
    document.getElementById = function(id) {
      return elements[id] || null;
    };

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
        get: function() { return elem._id; },
        set: function(v) {
          elem._id = v;
          elements[v] = elem;
        }
      });

      /// Define a remove method to clean up the elements map when an element is removed
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

    document.body.appendChild = function(elem) {
      return elem;
    };
  });
}
~SetupMockDOM(){
    EM_ASM({
       delete globalThis.document;
       delete globalThis.window;
       delete globalThis.Image;
    });
}
};


TEST_CASE("WebImage constructor creates image with correct properties", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-1", "assets/test.png");

  REQUIRE(img.GetId() == "test-img-1");
  REQUIRE(img.GetSource() == "assets/test.png");
  REQUIRE(img.GetWidth() == 0);
  REQUIRE(img.GetHeight() == 0);
}

TEST_CASE("WebImage SetSource changes the image source", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-2", "initial.png");

  img.SetSource("new_image.png");

  REQUIRE(img.GetSource() == "new_image.png");
}

TEST_CASE("WebImage SetSize updates dimensions correctly", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-3", "test.png");

  img.SetSize(200, 150);

  REQUIRE(img.GetWidth() == 200);
  REQUIRE(img.GetHeight() == 150);
}

TEST_CASE("WebImage GetSize returns correct dimensions", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-4", "test.png");

  img.SetSize(300, 250);
  auto [width, height] = img.GetSize();

  REQUIRE(width == 300);
  REQUIRE(height == 250);
}

TEST_CASE("WebImage SetPosition updates position correctly", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-5", "test.png");

  img.SetPosition(100, 50);
  auto [x, y] = img.GetPosition();

  REQUIRE(x == 100);
  REQUIRE(y == 50);
}

TEST_CASE("WebImage SetAlt updates alt text", "[WebImage]") {
  SetupMockDOM mock;
  cse498::WebImage img("test-img-6", "test.png");

  img.SetAlt("A test image");

  REQUIRE(img.GetAlt() == "A test image");
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


TEST_CASE("WebImage IsLoaded returns false for unloaded images", "[WebImage]") {
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

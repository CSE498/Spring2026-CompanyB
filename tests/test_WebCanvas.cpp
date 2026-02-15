#include <catch2/catch_test_macros.hpp>
#include <emscripten.h>
#include "interfaces/webUI/WebCanvas.hpp"

// Sets up a minimal mock of the DOM canvas so calls
// in WebCanvas don't crash under Node.js.
static void SetupMockCanvas() {
  EM_ASM({
    if (typeof document === 'undefined') {
      globalThis.document = {};
    }
    document.getElementById = function(id) {
      var ctx = {};
      ctx.clearRect = function() {};
      ctx.fillRect = function() {};
      ctx.strokeRect = function() {};
      ctx.beginPath = function() {};
      ctx.moveTo = function() {};
      ctx.lineTo = function() {};
      ctx.closePath = function() {};
      ctx.arc = function() {};
      ctx.fill = function() {};
      ctx.stroke = function() {};
      ctx.fillText = function() {};
      ctx.drawImage = function() {};
      ctx.translate = function() {};
      ctx.rotate = function() {};
      ctx.scale = function() {};
      ctx.save = function() {};
      ctx.restore = function() {};
      ctx.strokeStyle = "";
      ctx.fillStyle = "";
      ctx.lineWidth = 1;
      ctx.font = "";
      ctx.globalAlpha = 1.0;
      var canvas = {};
      canvas.width = 500;
      canvas.height = 500;
      canvas.style = {};
      canvas.getContext = function() { return ctx; };
      return canvas;
    };
  });
}

TEST_CASE("Initialize Canvas", "[web_canvas]") {
    cse498::WebCanvas canvas(500, 500, "1");

    REQUIRE(canvas.GetWidth() == 500);
    REQUIRE(canvas.GetHeight() == 500);
    REQUIRE(canvas.GetId() == "1");
};

TEST_CASE("DrawRect", "[web_canvas]") {
    SetupMockCanvas();
    cse498::WebCanvas canvas(500, 500, "test");

    SECTION("filled rectangle") {
        REQUIRE_NOTHROW(canvas.DrawRect(10, 20, 100, 50, true));
    }

    SECTION("stroked rectangle") {
        REQUIRE_NOTHROW(canvas.DrawRect(10, 20, 100, 50, false));
    }

    SECTION("zero-dimension rectangle") {
        REQUIRE_NOTHROW(canvas.DrawRect(0, 0, 0, 0));
    }
}

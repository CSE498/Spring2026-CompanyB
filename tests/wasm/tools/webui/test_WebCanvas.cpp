#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebCanvas.hpp"

// Sets up a mock DOM so WebCanvas can create, find, and remove canvas elements
struct SetupMockCanvas {
  SetupMockCanvas() {
    EM_ASM({
      if (typeof document == = 'undefined') {
        globalThis.document = {};
      }

      // Contain DOM elements
      var elements = {};

      // Create a fresh 2D context stub
      function makeCtx() {
        var ctx = {};
        ctx.clearRect = function(){};
        ctx.fillRect = function(){};
        ctx.strokeRect = function(){};
        ctx.beginPath = function(){};
        ctx.moveTo = function(){};
        ctx.lineTo = function(){};
        ctx.closePath = function(){};
        ctx.arc = function(){};
        ctx.fill = function(){};
        ctx.stroke = function(){};
        ctx.fillText = function(){};
        ctx.drawImage = function(){};
        ctx.translate = function(){};
        ctx.rotate = function(){};
        ctx.scale = function(){};
        ctx.save = function(){};
        ctx.restore = function(){};
        return ctx;
      }

      document.getElementById = function(id) { return elements[id] || null; };

      // Creates a mock element. Setting the ID registers the element.
      document.createElement = function() {
        var ctx = makeCtx();
        var elem = {};
        elem.width = 0;
        elem.height = 0;
        elem.style = {};
        elem.getContext = function() { return ctx; };
        elem.remove = function() {
          if (elem._id && elements[elem._id]) {
            delete elements[elem._id];
          }
        };
        elem._id = "";
      Object.defineProperty(elem, 'id', {
        get: function() { return elem._id;
      }
      , set : function(v) {
        elem._id = v;
        elements[v] = elem;
      }
    });
    return elem;
  };

  if (!document.body) {
    document.body = {};
  }
  document.body.appendChild = function(){};

  // Support for LoadImage or DrawImage
  if (typeof window == = 'undefined') {
    globalThis.window = globalThis;
  }
  globalThis.Image = function() { this.complete = false; };
  window._imageCache = {};

  // Support for RequestAnimationFrame
  if (typeof globalThis.requestAnimationFrame == = 'undefined') {
    globalThis.requestAnimationFrame = function(cb) { return 0; };
  }
});
}
~SetupMockCanvas() {
  EM_ASM({
    delete globalThis.document;
    delete globalThis.window;
    delete globalThis.Image;
  });
}
}
;

TEST_CASE("Initialize Canvas", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "1");

  REQUIRE(canvas.GetWidth() == 500);
  REQUIRE(canvas.GetHeight() == 500);
  REQUIRE(canvas.GetId() == "1");
}

TEST_CASE("DrawRect", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "test");

  SECTION("filled rectangle") {
    REQUIRE_NOTHROW(canvas.DrawRect(10, 20, 100, 50, true));
  }

  SECTION("stroked rectangle") {
    REQUIRE_NOTHROW(canvas.DrawRect(10, 20, 100, 50, false));
  }

  SECTION("zero dimension rectangle") {
    REQUIRE_NOTHROW(canvas.DrawRect(0, 0, 0, 0));
  }
}

TEST_CASE("Resize", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "resize_test");

  SECTION("resize updates width and height") {
    canvas.Resize(800, 600);
    REQUIRE(canvas.GetWidth() == 800);
    REQUIRE(canvas.GetHeight() == 600);
  }

  SECTION("resize to minimum 1 by 1") {
    canvas.Resize(1, 1);
    REQUIRE(canvas.GetWidth() == 1);
    REQUIRE(canvas.GetHeight() == 1);
  }

  SECTION("multiple resizes") {
    canvas.Resize(100, 200);
    REQUIRE(canvas.GetWidth() == 100);
    REQUIRE(canvas.GetHeight() == 200);
    canvas.Resize(300, 400);
    REQUIRE(canvas.GetWidth() == 300);
    REQUIRE(canvas.GetHeight() == 400);
  }
}

TEST_CASE("Clear", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "clear_test");

  SECTION("clear does not throw") { REQUIRE_NOTHROW(canvas.Clear()); }

  SECTION("clear after drawing") {
    canvas.DrawRect(10, 10, 100, 100);
    REQUIRE_NOTHROW(canvas.Clear());
  }
}

TEST_CASE("SetBackgroundColor", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "bgcolor_test");

  SECTION("set red background") {
    REQUIRE_NOTHROW(canvas.SetBackgroundColor({255, 0, 0}));
  }

  SECTION("set black background") {
    REQUIRE_NOTHROW(canvas.SetBackgroundColor({0, 0, 0}));
  }

  SECTION("set white background") {
    REQUIRE_NOTHROW(canvas.SetBackgroundColor({255, 255, 255}));
  }

  SECTION("set mid range background") {
    REQUIRE_NOTHROW(canvas.SetBackgroundColor({128, 128, 128}));
  }
}

TEST_CASE("DrawLine", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "line_test");

  SECTION("diagonal line") {
    REQUIRE_NOTHROW(canvas.DrawLine({0.0, 0.0}, {100.0, 100.0}));
  }

  SECTION("horizontal line") {
    REQUIRE_NOTHROW(canvas.DrawLine({10.0, 50.0}, {200.0, 50.0}));
  }

  SECTION("vertical line") {
    REQUIRE_NOTHROW(canvas.DrawLine({50.0, 10.0}, {50.0, 200.0}));
  }

  SECTION("zero length line") {
    REQUIRE_NOTHROW(canvas.DrawLine({50.0, 50.0}, {50.0, 50.0}));
  }

  SECTION("line with negative coordinates") {
    REQUIRE_NOTHROW(canvas.DrawLine({-10.0, -10.0}, {100.0, 100.0}));
  }
}

TEST_CASE("DrawCircle", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "circle_test");

  SECTION("filled circle") {
    REQUIRE_NOTHROW(canvas.DrawCircle(100.0, 100.0, 50.0, true));
  }

  SECTION("stroked circle") {
    REQUIRE_NOTHROW(canvas.DrawCircle(100.0, 100.0, 50.0, false));
  }

  SECTION("zero radius circle") {
    REQUIRE_NOTHROW(canvas.DrawCircle(100.0, 100.0, 0.0, true));
  }

  SECTION("large radius circle") {
    REQUIRE_NOTHROW(canvas.DrawCircle(250.0, 250.0, 500.0));
  }
}

TEST_CASE("DrawPolygon", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "polygon_test");

  SECTION("filled triangle") {
    std::vector<std::pair<double, double>> triangle = {
        {100.0, 100.0}, {200.0, 100.0}, {150.0, 200.0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(triangle, true));
  }

  SECTION("stroked triangle") {
    std::vector<std::pair<double, double>> triangle = {
        {100.0, 100.0}, {200.0, 100.0}, {150.0, 200.0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(triangle, false));
  }

  SECTION("quadrilateral") {
    std::vector<std::pair<double, double>> quad = {
        {10.0, 10.0}, {100.0, 10.0}, {100.0, 100.0}, {10.0, 100.0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(quad));
  }

  SECTION("hexagon") {
    std::vector<std::pair<double, double>> hexagon = {
        {250.0, 200.0}, {300.0, 225.0}, {300.0, 275.0},
        {250.0, 300.0}, {200.0, 275.0}, {200.0, 225.0}};
    REQUIRE_NOTHROW(canvas.DrawPolygon(hexagon));
  }
}

TEST_CASE("DrawText", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "text_test");

  SECTION("basic text") {
    REQUIRE_NOTHROW(canvas.DrawText("Hello, World!", 50.0, 50.0));
  }

  SECTION("empty string") { REQUIRE_NOTHROW(canvas.DrawText("", 0.0, 0.0)); }

  SECTION("text at origin") {
    REQUIRE_NOTHROW(canvas.DrawText("Origin", 0.0, 0.0));
  }
}

TEST_CASE("LoadImage and DrawImage", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "image_test");

  SECTION("load image does not throw") {
    REQUIRE_NOTHROW(canvas.LoadImage("test_image.png"));
  }

  SECTION("draw image does not throw") {
    REQUIRE_NOTHROW(canvas.DrawImage("test_image.png", 0.0, 0.0, 100.0, 100.0));
  }

  SECTION("load then draw image") {
    REQUIRE_NOTHROW(canvas.LoadImage("test_image.png"));
    REQUIRE_NOTHROW(canvas.DrawImage("test_image.png", 10.0, 10.0, 50.0, 50.0));
  }

  SECTION("load nonexistent image does not throw") {
    REQUIRE_NOTHROW(canvas.LoadImage("does_not_exist.png"));
  }
}

TEST_CASE("SetPenColor", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "pen_color_test");

  SECTION("set red") { REQUIRE_NOTHROW(canvas.SetPenColor({255, 0, 0})); }

  SECTION("set black") { REQUIRE_NOTHROW(canvas.SetPenColor({0, 0, 0})); }

  SECTION("set white") { REQUIRE_NOTHROW(canvas.SetPenColor({255, 255, 255})); }
}

TEST_CASE("SetFillColor", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "fill_color_test");

  SECTION("set red") { REQUIRE_NOTHROW(canvas.SetFillColor({255, 0, 0})); }

  SECTION("set black") { REQUIRE_NOTHROW(canvas.SetFillColor({0, 0, 0})); }

  SECTION("set white") {
    REQUIRE_NOTHROW(canvas.SetFillColor({255, 255, 255}));
  }
}

TEST_CASE("SetLineWidth", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "linewidth_test");

  SECTION("set line width to 5") { REQUIRE_NOTHROW(canvas.SetLineWidth(5.0)); }

  SECTION("set line width to zero") {
    REQUIRE_NOTHROW(canvas.SetLineWidth(0.0));
  }

  SECTION("set fractional line width") {
    REQUIRE_NOTHROW(canvas.SetLineWidth(0.5));
  }
}

TEST_CASE("SetFont", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "font_test");

  SECTION("set standard font") {
    REQUIRE_NOTHROW(canvas.SetFont("16px Arial"));
  }

  SECTION("set bold font") {
    REQUIRE_NOTHROW(canvas.SetFont("bold 24px Helvetica"));
  }

  SECTION("set empty font string") { REQUIRE_NOTHROW(canvas.SetFont("")); }
}

TEST_CASE("SetAlpha", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "alpha_test");

  SECTION("fully opaque") { REQUIRE_NOTHROW(canvas.SetAlpha(1.0)); }

  SECTION("fully transparent") { REQUIRE_NOTHROW(canvas.SetAlpha(0.0)); }

  SECTION("half transparent") { REQUIRE_NOTHROW(canvas.SetAlpha(0.5)); }
}

TEST_CASE("Translate", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "translate_test");

  SECTION("translate updates GetLocation") {
    canvas.Translate({100.0, 200.0});
    auto loc = canvas.GetLocation();
    REQUIRE(loc.first == 100.0);
    REQUIRE(loc.second == 200.0);
  }

  SECTION("translate to origin") {
    canvas.Translate({0.0, 0.0});
    auto loc = canvas.GetLocation();
    REQUIRE(loc.first == 0.0);
    REQUIRE(loc.second == 0.0);
  }

  SECTION("translate to negative coordinates") {
    canvas.Translate({-50.0, -75.0});
    auto loc = canvas.GetLocation();
    REQUIRE(loc.first == -50.0);
    REQUIRE(loc.second == -75.0);
  }

  SECTION("second translate overwrites location") {
    canvas.Translate({10.0, 20.0});
    canvas.Translate({30.0, 40.0});
    auto loc = canvas.GetLocation();
    REQUIRE(loc.first == 30.0);
    REQUIRE(loc.second == 40.0);
  }
}

TEST_CASE("GetLocation", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "location_test");

  SECTION("default location is origin") {
    auto loc = canvas.GetLocation();
    REQUIRE(loc.first == 0.0);
    REQUIRE(loc.second == 0.0);
  }
}

TEST_CASE("Rotate", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "rotate_test");

  SECTION("rotate clockwise") { REQUIRE_NOTHROW(canvas.Rotate(1.6782, true)); }

  SECTION("rotate counter clockwise") {
    REQUIRE_NOTHROW(canvas.Rotate(1.6782, false));
  }

  SECTION("rotate by zero") { REQUIRE_NOTHROW(canvas.Rotate(0.0, true)); }
}

TEST_CASE("Scale", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "scale_test");

  SECTION("uniform scale up") { REQUIRE_NOTHROW(canvas.Scale(2.0, 2.0)); }

  SECTION("uniform scale down") { REQUIRE_NOTHROW(canvas.Scale(0.5, 0.5)); }

  SECTION("non uniform scale") { REQUIRE_NOTHROW(canvas.Scale(2.0, 0.5)); }

  SECTION("scale by 1") { REQUIRE_NOTHROW(canvas.Scale(1.0, 1.0)); }
}

TEST_CASE("Save and Restore", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "save_restore_test");

  SECTION("save and restore does not throw") {
    REQUIRE_NOTHROW(canvas.Save());
    REQUIRE_NOTHROW(canvas.Restore());
  }

  SECTION("save and restore preserves location") {
    canvas.Translate({100.0, 200.0});
    canvas.Save();
    canvas.Translate({300.0, 400.0});
    REQUIRE(canvas.GetLocation().first == 300.0);
    REQUIRE(canvas.GetLocation().second == 400.0);
    canvas.Restore();
    REQUIRE(canvas.GetLocation().first == 100.0);
    REQUIRE(canvas.GetLocation().second == 200.0);
  }

  SECTION("multiple save and restores ") {
    canvas.Translate({10.0, 20.0});
    canvas.Save();
    canvas.Translate({30.0, 40.0});
    canvas.Save();
    canvas.Translate({50.0, 60.0});
    REQUIRE(canvas.GetLocation().first == 50.0);
    canvas.Restore();
    REQUIRE(canvas.GetLocation().first == 30.0);
    canvas.Restore();
    REQUIRE(canvas.GetLocation().first == 10.0);
  }
}

TEST_CASE("RequestAnimationFrame", "[web_canvas]") {
  SetupMockCanvas mock;
  cse498::WebCanvas canvas(500, 500, "raf_test");

  SECTION("request animation frame does not throw") {
    REQUIRE_NOTHROW(canvas.RequestAnimationFrame([]() {}));
  }
}

TEST_CASE("Destructor", "[web_canvas]") {
  SetupMockCanvas mock;

  SECTION("destructor removes element from DOM") {
    REQUIRE_NOTHROW(
        []() { cse498::WebCanvas canvas(500, 500, "destructor_test"); }());
  }
}

/**
 * @file WebCanvas.cpp
 * @brief C++ API for the DOM Canvas Element
 * @author Maksim Savich
 * @date 2026-2-2
 **/

/**
 * Canvas API Sources:
 * https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API
 * https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D
 **/

#include "WebCanvas.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <math.h>

#include <cassert>

using emscripten::val;

namespace cse498 {

WebCanvas::WebCanvas(int width, int height, const std::string& id)
    : WebElement(id, true), width(width), height(height) {
  val document = val::global("document");
  val existing = document.call<val>("getElementById", id);

  assert((existing.isNull() || existing.isUndefined()) &&
         "Canvas with this ID already exists in the DOM");

  canvas_element = document.call<val>("createElement", std::string("canvas"));
  canvas_element.set("id", id);
  document["body"].call<void>("appendChild", canvas_element);

  Resize(width, height);
}

WebCanvas::~WebCanvas() {
  if (!canvas_element.isNull() && !canvas_element.isUndefined()) {
    canvas_element.call<void>("remove");
  }
}

void WebCanvas::Resize(int new_width, int new_height) {
  assert(new_width > 0 && "Canvas width must be positive");
  assert(new_height > 0 && "Canvas height must be positive");
  width = new_width;
  height = new_height;
  canvas_element.set("width", width);
  canvas_element.set("height", height);
}

void WebCanvas::Clear() {
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("clearRect", 0, 0, width, height);
}

void WebCanvas::SetBackgroundColor(std::tuple<int, int, int> rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  background_color = rgb;
  std::string color = "rgb(" + std::to_string(r) + "," + std::to_string(g) +
                      "," + std::to_string(b) + ")";
  canvas_element["style"].set("backgroundColor", color);
}

void WebCanvas::DrawLine(std::pair<double, double> x,
                         std::pair<double, double> y) {
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("beginPath");
  ctx.call<void>("moveTo", x.first, x.second);
  ctx.call<void>("lineTo", y.first, y.second);
  ctx.call<void>("stroke");
}

void WebCanvas::DrawRect(double x, double y, double w, double h, bool filled) {
  assert(w >= 0 && "Rectangle width must be non-negative");
  assert(h >= 0 && "Rectangle height must be non-negative");
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  if (filled) {
    ctx.call<void>("fillRect", x, y, w, h);
  } else {
    ctx.call<void>("strokeRect", x, y, w, h);
  }
}

void WebCanvas::DrawCircle(double x, double y, double radius, bool filled) {
  assert(radius >= 0 && "Circle radius must be non-negative");
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("beginPath");
  ctx.call<void>("arc", x, y, radius, 0, 2 * M_PI);
  if (filled) {
    ctx.call<void>("fill");
  } else {
    ctx.call<void>("stroke");
  }
}

void WebCanvas::DrawPolygon(
    const std::vector<std::pair<double, double>>& points, bool filled) {
  assert(points.size() >= 3 && "Polygon must have at least 3 points");

  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("beginPath");
  ctx.call<void>("moveTo", points[0].first, points[0].second);

  for (size_t i = 1; i < points.size(); ++i) {
    ctx.call<void>("lineTo", points[i].first, points[i].second);
  }

  ctx.call<void>("closePath");

  if (filled) {
    ctx.call<void>("fill");
  } else {
    ctx.call<void>("stroke");
  }
}

void WebCanvas::DrawText(const std::string& text, double x, double y) {
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("fillText", text, x, y);
}

void WebCanvas::LoadImage(const std::string& path) {
  val window = val::global("window");
  if (window["_imageCache"].isUndefined()) {
    window.set("_imageCache", val::object());
  }
  val cache = window["_imageCache"];
  if (cache[path].isUndefined()) {
    val Image = val::global("Image");
    val img = Image.new_();
    img.set("src", path);
    cache.set(path, img);
  }
}

// Still requires the macro. So I am keeping this function the same.
void WebCanvas::DrawImage(const std::string& path, double x, double y, double w,
                          double h) {
  EM_ASM(
      {
        if (!window._imageCache) window._imageCache = {};
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        var src = UTF8ToString($1);
        var img = window._imageCache[src];
        if (img && img.complete) {
          ctx.drawImage(img, $2, $3, $4, $5);
        } else {
          img = img || new Image();
          img.src = src;
          window._imageCache[src] = img;
          img.onload = function() {
            ctx.drawImage(img, $2, $3, $4, $5);
          };
        }
      },
      id.c_str(), path.c_str(), x, y, w, h);
}

void WebCanvas::SetPenColor(std::tuple<int, int, int> rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  pen_color = rgb;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  std::string color = "rgb(" + std::to_string(r) + "," + std::to_string(g) +
                      "," + std::to_string(b) + ")";
  ctx.set("strokeStyle", color);
}

void WebCanvas::SetFillColor(std::tuple<int, int, int> rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  fill_color = rgb;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  std::string color = "rgb(" + std::to_string(r) + "," + std::to_string(g) +
                      "," + std::to_string(b) + ")";
  ctx.set("fillStyle", color);
}

void WebCanvas::SetLineWidth(double w) {
  assert(w >= 0 && "Line width must be non-negative");
  line_width = w;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.set("lineWidth", line_width);
}

void WebCanvas::SetFont(const std::string& new_font) {
  font = new_font;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.set("font", font);
}

void WebCanvas::SetAlpha(double new_alpha) {
  assert(new_alpha >= 0.0 && new_alpha <= 1.0 &&
         "Alpha must be between 0 and 1");
  alpha = new_alpha;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.set("globalAlpha", alpha);
}

void WebCanvas::Translate(std::pair<double, double> point) {
  location = point;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("translate", point.first, point.second);
}

void WebCanvas::Rotate(double angle, bool clockwise) {
  double directed_angle = clockwise ? angle : -angle;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("rotate", directed_angle);
}

void WebCanvas::Scale(double x, double y) {
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("scale", x, y);
}

void WebCanvas::Save() {
  saved_states.push_back(
      {pen_color, fill_color, alpha, line_width, font, location});
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("save");
}

void WebCanvas::Restore() {
  assert(!saved_states.empty() && "No saved state to restore");
  CanvasState state = saved_states.back();
  saved_states.pop_back();
  pen_color = state.pen_color;
  fill_color = state.fill_color;
  alpha = state.alpha;
  line_width = state.line_width;
  font = state.font;
  location = state.location;
  val ctx = canvas_element.call<val>("getContext", std::string("2d"));
  ctx.call<void>("restore");
}

int WebCanvas::GetWidth() const { return width; }

int WebCanvas::GetHeight() const { return height; }

// https://emscripten.org/docs/api_reference/html5.h.html#c.emscripten_request_animation_frame
void WebCanvas::RequestAnimationFrame(std::function<void()> callback) {
  void* cb = new std::function<void()>(std::move(callback));
  emscripten_request_animation_frame(
      [](double time, void* user_data) -> EM_BOOL {
        auto* fn = static_cast<std::function<void()>*>(user_data);
        (*fn)();
        delete fn;
        return EM_FALSE;
      },
      cb);
}

std::pair<double, double> WebCanvas::GetLocation() const { return location; }

}  // namespace cse498

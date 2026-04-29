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

WebCanvas::WebCanvas(int width, int height, const WebOptions& options)
    : WebElement("canvas", options), width(width), height(height) {
  ctx = dom_element.call<val>("getContext",
                              std::string("2d"));  // Save the context
  transform_matrix =
      ctx.call<val>("getTransform");  // Save the transform matrix
  Resize(width, height);              // Set the size of the canvas
}

WebCanvas::~WebCanvas() {
  if (!dom_element.isNull() && !dom_element.isUndefined()) {
    dom_element.call<void>("remove");
  }
}

WebCanvas& WebCanvas::Resize(int new_width, int new_height) {
  assert(new_width > 0 && "Canvas width must be positive");
  assert(new_height > 0 && "Canvas height must be positive");
  width = new_width;
  height = new_height;
  dom_element.set("width", width);
  dom_element.set("height", height);
  ApplyState();
  return *this;
}

std::string WebCanvas::RgbString(RGB rgb) {
  auto [r, g, b] = rgb;
  return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," +
         std::to_string(b) + ")";
}

WebCanvas& WebCanvas::ApplyState() {
  ctx.set("strokeStyle", RgbString(pen_color));
  ctx.set("fillStyle", RgbString(fill_color));

  ctx.set("globalAlpha", alpha);
  ctx.set("lineWidth", line_width);

  if (!font.empty()) {
    ctx.set("font", font);
  }

  if (!transform_matrix.isUndefined()) {
    ctx.call<void>("setTransform", transform_matrix);
  }
  return *this;
}

WebCanvas& WebCanvas::Clear() {
  ctx.call<void>("clearRect", 0, 0, width, height);
  return *this;
}

WebCanvas& WebCanvas::SetBackgroundColor(RGB rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  background_color = rgb;
  dom_element["style"].set("backgroundColor", RgbString(rgb));
  return *this;
}

WebCanvas& WebCanvas::DrawLine(std::pair<double, double> start,
                               std::pair<double, double> end) {
  ctx.call<void>("beginPath");
  ctx.call<void>("moveTo", start.first, start.second);
  ctx.call<void>("lineTo", end.first, end.second);
  ctx.call<void>("stroke");
  return *this;
}

WebCanvas& WebCanvas::DrawRect(double x_top_l, double y_top_l, double w,
                               double h, bool filled) {
  assert(w >= 0 && "Rectangle width must be non-negative");
  assert(h >= 0 && "Rectangle height must be non-negative");
  if (filled) {
    ctx.call<void>("fillRect", x_top_l, y_top_l, w, h);
  } else {
    ctx.call<void>("strokeRect", x_top_l, y_top_l, w, h);
  }
  return *this;
}

WebCanvas& WebCanvas::DrawCircle(double x, double y, double radius,
                                 bool filled) {
  assert(radius >= 0 && "Circle radius must be non-negative");
  ctx.call<void>("beginPath");
  ctx.call<void>("arc", x, y, radius, 0,
                 2 * M_PI);  // M_PI is a constant from the <math.h> header
  if (filled) {
    ctx.call<void>("fill");
  } else {
    ctx.call<void>("stroke");
  }
  return *this;
}

WebCanvas& WebCanvas::DrawPolygon(
    const std::vector<std::pair<double, double>>& points, bool filled) {
  assert(points.size() >= 3 && "Polygon must have at least 3 points");

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
  return *this;
}

WebCanvas& WebCanvas::DrawText(const std::string& text, double x, double y) {
  ctx.call<void>("fillText", text, x, y);
  return *this;
}

WebCanvas& WebCanvas::LoadImage(const std::string& path) {
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
    EM_ASM(
        {
          var src = UTF8ToString($0);
          var img = window._imageCache[src];
          if (img) {
            img.onerror = function() {
              console.warn("Failed to load image: " + src);
              delete window._imageCache[src];
            };
          }
        },
        path.c_str());
  }
  return *this;
}

// Still requires the macro. So I am keeping this function the same.
WebCanvas& WebCanvas::DrawImage(const std::string& path, double x, double y,
                                double w, double h) {
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
          img.onload = function() { ctx.drawImage(img, $2, $3, $4, $5); };
          img.onerror = function() {
            console.warn("Failed to load image: " + src);
            delete window._imageCache[src];
          };
        }
      },
      id.c_str(), path.c_str(), x, y, w, h);
  return *this;
}

WebCanvas& WebCanvas::SetPenColor(RGB rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  if (pen_color == rgb) return *this;
  pen_color = rgb;
  ctx.set("strokeStyle", RgbString(rgb));
  return *this;
}

WebCanvas& WebCanvas::SetFillColor(RGB rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  if (fill_color == rgb) return *this;
  fill_color = rgb;
  ctx.set("fillStyle", RgbString(rgb));

  return *this;
}

WebCanvas& WebCanvas::BeginPath() {
  ctx.call<void>("beginPath");
  return *this;
}

WebCanvas& WebCanvas::AddCircle(double x, double y, double radius) {
  // moveTo before each arc so consecutive circles don't get joined into one
  // continuous sub-path
  ctx.call<void>("moveTo", x + radius, y);
  ctx.call<void>("arc", x, y, radius, 0.0, 2 * M_PI);
  return *this;
}

WebCanvas& WebCanvas::Fill() {
  ctx.call<void>("fill");
  return *this;
}

WebCanvas& WebCanvas::Stroke() {
  ctx.call<void>("stroke");
  return *this;
}

WebCanvas& WebCanvas::DrawCanvas(const WebCanvas& src, double x, double y) {
  ctx.call<void>("drawImage", src.GetDOMElement(), x, y,
                 static_cast<double>(width), static_cast<double>(height));
  return *this;
}

WebCanvas& WebCanvas::SetLineWidth(double w) {
  assert(w >= 0 && "Line width must be non-negative");
  line_width = w;

  ctx.set("lineWidth", line_width);
  return *this;
}

WebCanvas& WebCanvas::SetFont(const std::string& new_font) {
  font = new_font;
  ctx.set("font", font);

  return *this;
}

WebCanvas& WebCanvas::SetAlpha(double new_alpha) {
  assert(new_alpha >= 0.0 && new_alpha <= 1.0 &&
         "Alpha must be between 0 and 1");
  alpha = new_alpha;
  ctx.set("globalAlpha", alpha);
  return *this;
}

WebCanvas& WebCanvas::Translate(std::pair<double, double> point) {
  location = point;
  ctx.call<void>("translate", point.first, point.second);
  transform_matrix = ctx.call<val>("getTransform");
  return *this;
}

WebCanvas& WebCanvas::Rotate(double angle, bool clockwise) {
  double directed_angle = clockwise ? angle : -angle;
  ctx.call<void>("rotate", directed_angle);
  transform_matrix = ctx.call<val>("getTransform");
  return *this;
}

WebCanvas& WebCanvas::Scale(double x, double y) {
  ctx.call<void>("scale", x, y);
  transform_matrix = ctx.call<val>("getTransform");
  return *this;
}

WebCanvas& WebCanvas::Save() {
  saved_states.push_back(
      {pen_color, fill_color, alpha, line_width, font, location});
  ctx.call<void>("save");
  return *this;
}

WebCanvas& WebCanvas::Restore() {
  assert(!saved_states.empty() && "No saved state to restore");
  CanvasState state = saved_states.back();
  saved_states.pop_back();
  pen_color = state.pen_color;
  fill_color = state.fill_color;
  alpha = state.alpha;
  line_width = state.line_width;
  font = state.font;
  location = state.location;
  ctx.call<void>("restore");
  transform_matrix = ctx.call<val>("getTransform");
  return *this;
}

int WebCanvas::GetWidth() const { return width; }

int WebCanvas::GetHeight() const { return height; }

// https://emscripten.org/docs/api_reference/html5.h.html#c.emscripten_request_animation_frame
void WebCanvas::RequestAnimationFrame(std::function<void()> callback) {
  void* cb = new std::function<void()>(
      std::move(callback));  // Heap allocate the callback
  // Put the callback on the next frame render
  emscripten_request_animation_frame(
      [](double time, void* user_data) -> EM_BOOL {
        auto* fn = static_cast<std::function<void()>*>(
            user_data);   // Move the callback to be an r-value function
        (*fn)();          // Call the function
        delete fn;        // Delete the function from the heap
        return EM_FALSE;  // Signal the end of the run
      },
      cb);
}

std::pair<double, double> WebCanvas::GetLocation() const { return location; }

}  // namespace cse498

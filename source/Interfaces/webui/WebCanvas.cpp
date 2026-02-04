/**
 * Canvas API Sources:
 * https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API
 * https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D
 **/

#include "WebCanvas.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cassert>

namespace cse498 {

WebCanvas::WebCanvas(int width, int height, const std::string& id)
    : width(width), height(height), id(id) {
  EM_ASM(
      {
        var canvasId = UTF8ToString($0);
        var canvas = document.getElementById(canvasId);
        if (!canvas) {
          canvas = document.createElement('canvas');
          canvas.id = canvasId;
          document.body.appendChild(canvas);
        }
      },
      id.c_str());
  Resize(width, height);
}

void WebCanvas::Resize(int new_width, int new_height) {
  assert(
      new_width > 0 &&
      "Canvas width must be positive");  // Assert with message method:
                                         // https://stackoverflow.com/a/3692961
  assert(new_height > 0 && "Canvas height must be positive");
  width = new_width;
  height = new_height;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        canvas.width = $1;
        canvas.height = $2;
      },
      id.c_str(), width, height);
}

void WebCanvas::Clear() {
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.clearRect(0, 0, canvas.width, canvas.height);
      },
      id.c_str());
}

void WebCanvas::SetBackgroundColor(std::tuple<int, int, int> rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  background_color = rgb;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        canvas.style.backgroundColor = 'rgb(' + $1 + ',' + $2 + ',' + $3 + ')';
      },
      id.c_str(), r, g, b);
}

void WebCanvas::DrawLine(std::pair<double, double> x,
                         std::pair<double, double> y) {
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.moveTo($1, $2);
        ctx.lineTo($3, $4);
        ctx.stroke();
      },
      id.c_str(), x.first, x.second, y.first, y.second);
}

void WebCanvas::DrawRect(double x, double y, double w, double h, bool filled) {
  assert(w >= 0 && "Rectangle width must be non-negative");
  assert(h >= 0 && "Rectangle height must be non-negative");
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        if ($5) {
          ctx.fillRect($1, $2, $3, $4);
        } else {
          ctx.strokeRect($1, $2, $3, $4);
        }
      },
      id.c_str(), x, y, w, h, filled);
}

void WebCanvas::DrawCircle(double x, double y, double radius, bool filled) {
  assert(radius >= 0 && "Circle radius must be non-negative");
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.beginPath();
        ctx.arc($1, $2, $3, 0, 2 * Math.PI);
        if ($4) {
          ctx.fill();
        } else {
          ctx.stroke();
        }
      },
      id.c_str(), x, y, radius, filled);
}

void WebCanvas::DrawPolygon(
    const std::vector<std::pair<double, double>>& points, bool filled) {
  assert(points.size() >= 3 && "Polygon must have at least 3 points");
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        var numPoints = $1;
        var dataPtr = $2;
        ctx.beginPath();
        ctx.moveTo(HEAPF64[dataPtr / 8], HEAPF64[dataPtr / 8 + 1]);
        for (var i = 1; i < numPoints; i++) {
          ctx.lineTo(HEAPF64[dataPtr / 8 + i * 2],
                     HEAPF64[dataPtr / 8 + i * 2 + 1]);
        }
        ctx.closePath();
        if ($3) {
          ctx.fill();
        } else {
          ctx.stroke();
        }
      },
      id.c_str(), static_cast<int>(points.size()), points.data(), filled);
}

void WebCanvas::DrawText(const std::string& text, double x, double y) {
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.fillText(UTF8ToString($1), $2, $3);
      },
      id.c_str(), text.c_str(), x, y);
}

void WebCanvas::LoadImage(const std::string& path) {
  EM_ASM(
      {
        if (!window._imageCache) window._imageCache = {};
        var src = UTF8ToString($0);
        if (!window._imageCache[src]) {
          var img = new Image();
          img.src = src;
          window._imageCache[src] = img;
        }
      },
      path.c_str());
}

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
          img.onload = function() { ctx.drawImage(img, $2, $3, $4, $5); };
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
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.strokeStyle = 'rgb(' + $1 + ',' + $2 + ',' + $3 + ')';
      },
      id.c_str(), r, g, b);
}

void WebCanvas::SetFillColor(std::tuple<int, int, int> rgb) {
  auto [r, g, b] = rgb;
  assert(r >= 0 && r <= 255 && "Red value must be 0-255");
  assert(g >= 0 && g <= 255 && "Green value must be 0-255");
  assert(b >= 0 && b <= 255 && "Blue value must be 0-255");
  fill_color = rgb;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.fillStyle = 'rgb(' + $1 + ',' + $2 + ',' + $3 + ')';
      },
      id.c_str(), r, g, b);
}

void WebCanvas::SetLineWidth(double w) {
  assert(w >= 0 && "Line width must be non-negative");
  line_width = w;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.lineWidth = $1;
      },
      id.c_str(), line_width);
}

void WebCanvas::SetFont(const std::string& new_font) {
  font = new_font;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.font = UTF8ToString($1);
      },
      id.c_str(), font.c_str());
}

void WebCanvas::SetAlpha(double new_alpha) {
  assert(new_alpha >= 0.0 && new_alpha <= 1.0 &&
         "Alpha must be between 0 and 1");
  alpha = new_alpha;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.globalAlpha = $1;
      },
      id.c_str(), alpha);
}

void WebCanvas::Translate(std::pair<double, double> point) {
  location = point;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.translate($1, $2);
      },
      id.c_str(), point.first, point.second);
}

void WebCanvas::Rotate(double angle, bool clockwise) {
  double directed_angle = clockwise ? angle : -angle;
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.rotate($1);
      },
      id.c_str(), directed_angle);
}

void WebCanvas::Scale(double x, double y) {
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.scale($1, $2);
      },
      id.c_str(), x, y);
}

void WebCanvas::Save() {
  saved_states.push_back(
      {pen_color, fill_color, alpha, line_width, font, location});
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.save();
      },
      id.c_str());
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
  EM_ASM(
      {
        var canvas = document.getElementById(UTF8ToString($0));
        var ctx = canvas.getContext('2d');
        ctx.restore();
      },
      id.c_str());
}

int WebCanvas::GetWidth() const { return width; }

int WebCanvas::GetHeight() const { return height; }

std::string WebCanvas::GetCanvasId() const { return id; }

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

/**
 * @file WebCanvas.hpp
 * @brief C++ API for the DOM Canvas Element
 * @author Maksim Savich
 * @date 2026-2-2
 **/

#pragma once

#include <emscripten/val.h>

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "WebElement.hpp"

namespace cse498 {

/**
 * @class WebCanvas
 * @brief A C++ wrapper for the HTML5 Canvas 2D API, compiled to WebAssembly
 * via Emscripten.
 *
 * WebCanvas provides a high-level interface for 2D drawing operations in a
 * browser environment. It manages a DOM canvas element and exposes methods for
 * drawing shapes, text, and images, as well as controlling rendering state
 * (colors, transforms, etc.).
 **/
class WebCanvas : public WebElement {
 public:
  using RGB = std::tuple<int, int, int>;

 private:
  /// A struct for storing the state of a canvas
  struct CanvasState {
    RGB pen_color{255, 255, 255};
    RGB fill_color{255, 255, 255};
    double alpha = 1.0;
    double line_width = 1.0;
    std::string font{};
    std::pair<double, double> location{0.0, 0.0};
  };

  /// Canvas width
  int width = 0;

  /// Canvas height
  int height = 0;

  /// Background color of the Canvas
  RGB background_color{0, 0, 0};

  /// The pen color the canvas sues
  RGB pen_color{255, 255, 255};

  /// The fill color the canvas uses for shapes
  RGB fill_color{255, 255, 255};

  /// The opacity of the shapes, images, and text drawn onto the canvas.
  double alpha = 1.0;

  /// The width of the line that the pen draws.
  double line_width = 1.0;

  /// The font that the canvas writes text in.
  std::string font{};

  /// The current translation offset of the 2D context's transform origin.
  std::pair<double, double> location{0.0, 0.0};

  /// Vector of saved states
  std::vector<CanvasState> saved_states;

  /// The cached 2D rendering context
  emscripten::val ctx = emscripten::val::null();

  /// The current transform matrix
  emscripten::val transform_matrix = emscripten::val::undefined();

  /**
   * @brief Reapplies all tracked C++ state to the 2D context
   **/
  WebCanvas& ApplyState();

  /**
   * @brief Converts an RGB tuple to a CSS rgb() string
   **/
  static std::string RgbString(RGB rgb);

 public:
  /**
   * @brief Construct a WebCanvas, creating the canvas element in the DOM if
   * it does not already exist.
   * @param width The width of the canvas in pixels.
   * @param height The height of the canvas in pixels.
   * @param options Optional WebOptions to apply to the canvas.
   **/
  WebCanvas(int width, int height, const WebOptions& options = {});

  /// Disable copy constructor since each instance is tied to a specific DOM
  /// element
  WebCanvas(const WebCanvas&) = delete;

  /// Disable copy assignment operator
  WebCanvas& operator=(const WebCanvas&) = delete;

  /// Destructor that removes the canvas element from the DOM if this instance
  /// created it
  ~WebCanvas();

  /**
   * @brief Resize the canvas to the given dimensions.
   * @param width The new width in pixels. Must be positive.
   * @param height The new height in pixels. Must be positive.
   **/
  WebCanvas& Resize(int width, int height);

  /**
   * @brief Clear the entire canvas, removing all drawn content.
   **/
  WebCanvas& Clear();

  /**
   * @brief Set the CSS background color of the canvas element.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   **/
  WebCanvas& SetBackgroundColor(RGB rgb);

  /**
   * @brief Draw a straight line between two points.
   * @param x The starting point as (x, y).
   * @param y The ending point as (x, y).
   **/
  WebCanvas& DrawLine(std::pair<double, double> start,
                      std::pair<double, double> end);

  /**
   * @brief Draw a rectangle on the canvas.
   * @param x The x-coordinate of the top-left corner.
   * @param y The y-coordinate of the top-left corner.
   * @param width The width of the rectangle. Must be non-negative.
   * @param height The height of the rectangle. Must be non-negative.
   * @param filled If true, fill the rectangle; otherwise, stroke the outline.
   **/
  WebCanvas& DrawRect(double x_top_l, double y_top_l, double width,
                      double height, bool filled = true);

  /**
   * @brief Draw a circle on the canvas.
   * @param x The x-coordinate of the center.
   * @param y The y-coordinate of the center.
   * @param radius The radius of the circle. Must be non-negative.
   * @param filled If true, fill the circle; otherwise, stroke the outline.
   **/
  WebCanvas& DrawCircle(double x, double y, double radius, bool filled = true);

  /**
   * @brief Draw a closed polygon from a series of points.
   * @param points A vector of (x, y) pairs defining the polygon vertices.
   *               Must contain at least 3 points.
   * @param filled If true, fill the polygon; otherwise, stroke the outline.
   **/
  WebCanvas& DrawPolygon(const std::vector<std::pair<double, double>>& points,
                         bool filled = true);

  /**
   * @brief Draw text on the canvas at the given position using the current
   * fill color and font.
   * @param text The string to render.
   * @param x The x-coordinate of the text origin.
   * @param y The y-coordinate of the text baseline.
   **/
  WebCanvas& DrawText(const std::string& text, double x, double y);

  /**
   * @brief Preload an image into the cache so that subsequent DrawImage
   * calls with the same path can draw synchronously.
   * @param path The file path or URL of the image to preload.
   **/
  WebCanvas& LoadImage(const std::string& path);

  /**
   * @brief Draw an image on the canvas. If the image has been preloaded via
   * LoadImage, it is drawn synchronously; otherwise, it is drawn
   * asynchronously once loaded.
   * @param path The file path or URL of the image.
   * @param x The x-coordinate of the top-left corner.
   * @param y The y-coordinate of the top-left corner.
   * @param width The width to draw the image.
   * @param height The height to draw the image.
   **/
  WebCanvas& DrawImage(const std::string& path, double x, double y,
                       double width, double height);

  /**
   * @brief Set the stroke color used for lines and shape outlines.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   **/
  WebCanvas& SetPenColor(RGB rgb);

  /**
   * @brief Set the fill color used for filled shapes and text.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   */
  WebCanvas& SetFillColor(RGB rgb);

  /**
   * @brief Set the line width for stroke operations.
   * @param width The line width in pixels. Must be non-negative.
   **/
  WebCanvas& SetLineWidth(double width);

  /**
   * @brief Set the font used for text rendering.
   * @param font A CSS font string (e.g., "16px Arial").
   **/
  WebCanvas& SetFont(const std::string& font);

  /**
   * @brief Set the global alpha (opacity) for all subsequent draw operations.
   * @param alpha A value in the range [0.0, 1.0] where 0.0 is fully
   *              transparent and 1.0 is fully opaque.
   **/
  WebCanvas& SetAlpha(double alpha);

  /**
   * @brief Translate the canvas origin by the given offset.
   * @param point The (x, y) offset to translate by.
   **/
  WebCanvas& Translate(std::pair<double, double> point);

  /**
   * @brief Rotate the canvas around the current origin.
   * @param angle The rotation angle in radians.
   * @param clockwise If true, rotate clockwise; otherwise, counter-clockwise.
   **/
  WebCanvas& Rotate(double angle, bool clockwise);

  /**
   * @brief Scale the canvas coordinate system.
   * @param x The horizontal scale factor.
   * @param y The vertical scale factor.
   **/
  WebCanvas& Scale(double x, double y);

  /**
   * @brief Save the current canvas state (colors, alpha, line width, font,
   * location, and transform) onto the state stack.
   **/
  WebCanvas& Save();

  /**
   * @brief Restore the most recently saved canvas state from the state stack.
   * @pre Save() must have been called at least once prior.
   **/
  WebCanvas& Restore();

  /**
   * @brief Get the current width of the canvas in pixels.
   * @return The canvas width.
   **/
  [[nodiscard]] int GetWidth() const;

  /**
   * @brief Get the current height of the canvas in pixels.
   * @return The canvas height.
   **/
  [[nodiscard]] int GetHeight() const;

  /**
   * @brief Schedule a callback to run on the next browser repaint frame.
   * The callback is invoked once; call again from within the callback
   * to create an animation loop.
   * @param callback The function to invoke on the next frame.
   **/
  void RequestAnimationFrame(std::function<void()> callback);

  /**
   * @brief Get the current translation offset of the canvas.
   * @return The (x, y) location set by the most recent Translate call.
   **/
  [[nodiscard]] std::pair<double, double> GetLocation() const;
};

}  // namespace cse498

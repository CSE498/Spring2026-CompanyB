/**
 * @file WebCanvas.hpp
 * @brief C++ API for the DOM Canvas Element
 * @author Maksim Savich
 * @date 2026-2-2
 **/

#pragma once

#include <emscripten/val.h>

#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

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
class WebCanvas {
 private:
  /// A struct for storing the state of a canvas
  struct CanvasState {
    std::tuple<int, int, int> pen_color;
    std::tuple<int, int, int> fill_color;
    double alpha;
    double line_width;
    std::string font;
    std::pair<double, double> location;
  };

  /// Canvas width
  int width = 0;

  /// Canvas height
  int height = 0;

  /// Canvas ID
  std::string id;

  /// Background color of the Canvas
  std::tuple<int, int, int> background_color{0, 0, 0};

  /// The pen color the canvas sues
  std::tuple<int, int, int> pen_color{0, 0, 0};

  /// The fill color the canvas uses for shapes
  std::tuple<int, int, int> fill_color{0, 0, 0};

  /// The opacity of the shapes, images, and text drawn onto the canvas.
  double alpha = 1.0;

  /// The width of the line that the pen draws.
  double line_width = 1.0;

  /// The font that the canvas writes text in.
  std::string font;

  /// The current translation offset of the 2D context's transform origin.
  std::pair<double, double> location{0.0, 0.0};

  /// Vector of saved states
  std::vector<CanvasState> saved_states;

  /// The DOM canvas element
  emscripten::val canvas_element = emscripten::val::null();

 public:
  /**
   * @brief Construct a WebCanvas, creating the canvas element in the DOM if
   * it does not already exist.
   * @param width The width of the canvas in pixels.
   * @param height The height of the canvas in pixels.
   * @param id The DOM element ID for the canvas.
   **/
  WebCanvas(int width, int height, const std::string& id);

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
  void Resize(int width, int height);

  /**
   * @brief Clear the entire canvas, removing all drawn content.
   **/
  void Clear();

  /**
   * @brief Set the CSS background color of the canvas element.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   **/
  void SetBackgroundColor(std::tuple<int, int, int> rgb);

  /**
   * @brief Draw a straight line between two points.
   * @param x The starting point as (x, y).
   * @param y The ending point as (x, y).
   **/
  void DrawLine(std::pair<double, double> x, std::pair<double, double> y);

  /**
   * @brief Draw a rectangle on the canvas.
   * @param x The x-coordinate of the top-left corner.
   * @param y The y-coordinate of the top-left corner.
   * @param width The width of the rectangle. Must be non-negative.
   * @param height The height of the rectangle. Must be non-negative.
   * @param filled If true, fill the rectangle; otherwise, stroke the outline.
   **/
  void DrawRect(double x, double y, double width, double height,
                bool filled = true);

  /**
   * @brief Draw a circle on the canvas.
   * @param x The x-coordinate of the center.
   * @param y The y-coordinate of the center.
   * @param radius The radius of the circle. Must be non-negative.
   * @param filled If true, fill the circle; otherwise, stroke the outline.
   **/
  void DrawCircle(double x, double y, double radius, bool filled = true);

  /**
   * @brief Draw a closed polygon from a series of points.
   * @param points A vector of (x, y) pairs defining the polygon vertices.
   *               Must contain at least 3 points.
   * @param filled If true, fill the polygon; otherwise, stroke the outline.
   **/
  void DrawPolygon(const std::vector<std::pair<double, double>>& points,
                   bool filled = true);

  /**
   * @brief Draw text on the canvas at the given position using the current
   * fill color and font.
   * @param text The string to render.
   * @param x The x-coordinate of the text origin.
   * @param y The y-coordinate of the text baseline.
   **/
  void DrawText(const std::string& text, double x, double y);

  /**
   * @brief Preload an image into the cache so that subsequent DrawImage
   * calls with the same path can draw synchronously.
   * @param path The file path or URL of the image to preload.
   **/
  void LoadImage(const std::string& path);

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
  void DrawImage(const std::string& path, double x, double y, double width,
                 double height);

  /**
   * @brief Set the stroke color used for lines and shape outlines.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   **/
  void SetPenColor(std::tuple<int, int, int> rgb);

  /**
   * @brief Set the fill color used for filled shapes and text.
   * @param rgb An (r, g, b) tuple with values in the range [0, 255].
   */
  void SetFillColor(std::tuple<int, int, int> rgb);

  /**
   * @brief Set the line width for stroke operations.
   * @param width The line width in pixels. Must be non-negative.
   **/
  void SetLineWidth(double width);

  /**
   * @brief Set the font used for text rendering.
   * @param font A CSS font string (e.g., "16px Arial").
   **/
  void SetFont(const std::string& font);

  /**
   * @brief Set the global alpha (opacity) for all subsequent draw operations.
   * @param alpha A value in the range [0.0, 1.0] where 0.0 is fully
   *              transparent and 1.0 is fully opaque.
   **/
  void SetAlpha(double alpha);

  /**
   * @brief Translate the canvas origin by the given offset.
   * @param point The (x, y) offset to translate by.
   **/
  void Translate(std::pair<double, double> point);

  /**
   * @brief Rotate the canvas around the current origin.
   * @param angle The rotation angle in radians.
   * @param clockwise If true, rotate clockwise; otherwise, counter-clockwise.
   **/
  void Rotate(double angle, bool clockwise);

  /**
   * @brief Scale the canvas coordinate system.
   * @param x The horizontal scale factor.
   * @param y The vertical scale factor.
   **/
  void Scale(double x, double y);

  /**
   * @brief Save the current canvas state (colors, alpha, line width, font,
   * location, and transform) onto the state stack.
   **/
  void Save();

  /**
   * @brief Restore the most recently saved canvas state from the state stack.
   * @pre Save() must have been called at least once prior.
   **/
  void Restore();

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
   * @brief Get the DOM element ID of the canvas.
   * @return The canvas ID string.
   **/
  [[nodiscard]] std::string GetCanvasId() const;

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

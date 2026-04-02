/**
 * @file WebImage.hpp
 * @brief C++ wrapper for HTML image elements
 * @author Mariana Vangelov
 **/

#pragma once

#include <emscripten/val.h>

#include <expected>
#include <string>

#include "WebElement.hpp"

namespace cse498 {

/// CSS units supported for image sizing (to help avoid wrong unit inputs)
enum class SizeUnit { px, em, rem, percent, vw, vh };

/**
 * @class WebImage
 * @brief A C++ wrapper for HTML <img> elements
 *
 * WebImage provides a C++ interface for creating and managing image elements
 * in a browser environment. It handles DOM manipulation through Emscripten,
 * allowing programmers to control image source, position, size, alt text,
 * and visibility using C++ functions.
 **/
class WebImage : public WebElement {
 private:
  /// Image's source
  std::string src{};

  /// Image's alt text
  std::string alt{};

  /// Image's Width
  double width = 0;

  /// Image's Height
  double height = 0;

  /// Image's X position
  int x_pos = 0;

  /// Image's Y position
  int y_pos = 0;

 public:
  /**
   * @brief Construct a WebImage, creating the <img> element in the DOM.
   * @param src The image source URL or file path.
   * @param alt_text Alternative text for accessibility
   * @param options Optional WebOptions to apply to the image.
   **/
  WebImage(const std::string& src,
           const std::string& alt_text, const WebOptions& options = {});

  /// Destructor that removes the image element from the DOM
  ~WebImage();

  /// Disable copy constructor since each instance is tied to a specific DOM
  /// element
  WebImage(const WebImage&) = delete;

  /// Disable copy assignment operator
  WebImage& operator=(const WebImage&) = delete;

  /**
   * @brief Check if the image is loaded.
   * @return Bool of loaded status.
   **/
  [[nodiscard]] bool IsLoaded() const;

  /**
   * @brief Check if the image has had an error during loading.
   * @return An empty expected if no error occurred, or an unexpected containing
   *         an error message if the image failed to load.
   **/
  [[nodiscard]] std::expected<void, std::string> HasError() const;

  /**
   * @brief Change the image source to a new file or URL.
   * @param source The new image source path or URL.
   **/
  void SetSource(const std::string& source);

  /**
   * @brief Set the alt text for accessibility and screen readers.
   * @param alt_text The alternative text describing the image.
   **/
  void SetAlt(const std::string& alt_text);

  /**
   * @brief Set the position of the image on the page.
   * @param x The x-coordinate in pixels from the left edge.
   * @param y The y-coordinate in pixels from the top edge.
   **/
  void SetPosition(int x, int y);

  /**
   * @brief Set the display size of the image.
   * @param w The width value. Must be numeric.
   * @param h The height value. Must be numeric.
   * @param unit The CSS unit enum with defaults to SizeUnit::px.
   **/
  template <typename T>
  void SetSize(T w, T h, SizeUnit unit = SizeUnit::px) {
    static_assert(std::is_arithmetic_v<T>, "Dimensions must be numeric");

    width = w;
    height = h;

    std::string unit_str;

    switch (unit) {
      case SizeUnit::px:
        unit_str = "px";
        break;
      case SizeUnit::em:
        unit_str = "em";
        break;
      case SizeUnit::rem:
        unit_str = "rem";
        break;
      case SizeUnit::percent:
        unit_str = "%";
        break;
      case SizeUnit::vw:
        unit_str = "vw";
        break;
      case SizeUnit::vh:
        unit_str = "vh";
        break;
    }

    dom_element["style"].set("width", std::to_string(w) + unit_str);
    dom_element["style"].set("height", std::to_string(h) + unit_str);
  }

  /**
   * @brief Show or hide the image element.
   * @param is_visible If true, display the image; if false, hide it.
   **/
  void SetVisible(bool is_visible);

  /**
   * @brief Get the current width and height of the image.
   * @return A pair containing (width, height) in pixels.
   **/
  [[nodiscard]] std::pair<double, double> GetSize() const {
    return {width, height};
  }

  /**
   * @brief Get the current position of the image.
   * @return A pair containing (x, y) coordinates in pixels.
   **/
  [[nodiscard]] std::pair<int, int> GetPosition() const {
    return {x_pos, y_pos};
  }

  /**
   * @brief Get the alt text of the image.
   * @return The alternative text string.
   **/
  [[nodiscard]] std::string GetAlt() const { return alt; }

  /**
   * @brief Get the current image source path or URL.
   * @return The source string.
   **/
  [[nodiscard]] std::string GetSource() const { return src; }

  /**
   * @brief Get the current width of the image in pixels.
   * @return The image width.
   **/
  [[nodiscard]] double GetWidth() const { return width; }

  /**
   * @brief Get the current height of the image in pixels.
   * @return The image height.
   **/
  [[nodiscard]] double GetHeight() const { return height; }
};

}  // namespace cse498
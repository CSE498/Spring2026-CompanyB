/**
 * @file WebImage.hpp
 * @brief C++ wrapper for HTML image elements
 * @author Mariana Vangelov
 **/

#pragma once

#include <emscripten/val.h>

#include <string>

namespace cse498 {

/**
 * @class WebImage
 * @brief A C++ wrapper for HTML <img> elements
 *
 * WebImage provides a C++ interface for creating and managing image elements
 * in a browser environment. It handles DOM manipulation through Emscripten,
 * allowing programmers to control image source, position, size, alt text,
 * and visibility using C++ functions.
 **/
class WebImage {
 private:
  /// Image's alt text
  std::string alt;

  /// Image's DOM element ID
  std::string id;

  /// Image's source
  std::string src;

  /// Image's Width
  int width = 0;

  /// Image's Height
  int height = 0;

  /// Image's X position
  int x_pos = 0;

  /// Image's Y position
  int y_pos = 0;

  /// DOM image element
  emscripten::val img_element = emscripten::val::null();

 public:
  /**
   * @brief Construct a WebImage, creating the <img> element in the DOM.
   * @param img_id The DOM element ID for the image. Must be unique.
   * @param src The image source URL or file path.
   **/
  WebImage(const std::string& img_id, const std::string& src);

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
  bool IsLoaded() const;

  /**
   * @brief Check if the image has had an error during loading.
   * @return True if error has occured, false otherwise.
   **/
  bool HasError() const;

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
   * @param w The width in pixels. Must be non-negative.
   * @param h The height in pixels. Must be non-negative.
   **/
  void SetSize(int w, int h);

  /**
   * @brief Show or hide the image element.
   * @param is_visible If true, display the image; if false, hide it.
   **/
  void SetVisible(bool is_visible);

  /**
   * @brief Get the current width and height of the image.
   * @return A pair containing (width, height) in pixels.
   **/
  std::pair<int, int> GetSize() const { return {width, height}; }

  /**
   * @brief Get the current position of the image.
   * @return A pair containing (x, y) coordinates in pixels.
   **/
  std::pair<int, int> GetPosition() const { return {x_pos, y_pos}; }

  /**
   * @brief Get the alt text of the image.
   * @return The alternative text string.
   **/
  std::string GetAlt() const { return alt; }

  /**
   * @brief Get the DOM element ID of the image.
   * @return The image element's unique identifier.
   **/
  std::string GetId() const { return id; }

  /**
   * @brief Get the current image source path or URL.
   * @return The source string.
   **/
  std::string GetSource() const { return src; }

  /**
   * @brief Get the current width of the image in pixels.
   * @return The image width.
   **/
  int GetWidth() const { return width; }

  /**
   * @brief Get the current height of the image in pixels.
   * @return The image height.
   **/
  int GetHeight() const { return height; }
};

}  // namespace cse498
#pragma once

#include <functional>
#include <string>
#include <stdexcept>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {
/**
 * @brief C++ wrapper for a web UI button (HTML <button>) in the Emscripten
 * interface.
 *
 * This class stores button state (label, visibility, enabled) and a click
 * callback
 */
class WebButton {
 public:
  /// Construct a WebButton with a label
  explicit WebButton(const std::string& label, const std::string& id);

  /// Get/Set the label
  const std::string& GetLabel() const;
  void SetLabel(const std::string& label);

  /// Get/Set the DOM id
  const std::string& GetId() const;
  void SetId(const std::string& id);

  /// Get/Set the color
  const std::string& GetColor() const;
  void SetColor(const std::string& color);

  /// Get/Set the size
  int GetWidth() const;
  int GetHeight() const;
  void SetSize(int width, int height);

  /// Show/Hide controls whether the button is visible.
  void Show();
  void Hide();
  bool IsVisible() const;

  /// Enable/Disable controls whether the button can be clicked.
  void Enable();
  void Disable();
  bool IsEnabled() const;

  /// Register a callback to run when the button is clicked.
  void SetOnClick(std::function<void()> callback);

  /**
   * @brief Simulate a click for testing.
   *
   * Runs the callback only if visible and enabled and a callback is set.
   * (In the browser build, this will be triggered by a DOM event.)
   */
  void Click();

 private:
  std::string label_;
  std::string id_;
  std::string color_{"black"};
  int width_{100};
  int height_{50};
  bool visible_{true};
  bool enabled_{true};
  std::function<void()> on_click_{};
};

}
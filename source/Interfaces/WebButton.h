#pragma once

#include <functional>
#include <string>

/**
 * @brief C++ wrapper for a web UI button (HTML <button>) in the Emscripten interface.
 *
 * This class stores button state (label, visibility, enabled) and a click callback
 */
class WebButton {
 public:
  /// Construct a WebButton with a label
  explicit WebButton(const std::string& label, const std::string& id = "");

  /// Get/Set the label 
  const std::string& GetLabel() const;
  void SetLabel(const std::string& label);

  /// Get/Set an optional DOM id
  const std::string& GetId() const;
  void SetId(const std::string& id);

  /// Show/Hide  button is visible.
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
  bool visible_{true};
  bool enabled_{true};
  std::function<void()> on_click_{};
};

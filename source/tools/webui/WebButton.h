#pragma once

#include <emscripten/val.h>

#include <functional>
#include <string>
#include <memory>

#include "WebElement.hpp"

/**
 * @brief C++ wrapper for a web UI button (HTML <button>) in the Emscripten
 * interface.
 *
 * This class stores button state (label, visibility, enabled) and a click
 * callback
 */

namespace cse498 {

class WebButton : public WebElement {
 public:
  explicit WebButton(const std::string& label, const WebOptions& options = {});
  ~WebButton();

  const std::string& GetLabel() const;
  void SetLabel(const std::string& label);

  void Show();
  void Hide();
  bool IsVisible() const;

  void Enable();
  void Disable();
  bool IsEnabled() const;

  WebButton& SetOnClick(std::function<void()> callback);
  void Click(emscripten::val arg);

 private:
  std::string label_;
  bool visible_{true};
  bool enabled_{true};
  std::function<void()> on_click_{};
  /// @brief Allows us to remove event listeners on destruction. 
  emscripten::val abort_controller_;
};

}  // namespace cse498

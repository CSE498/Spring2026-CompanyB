#pragma once

#include <emscripten/val.h>

#include <functional>
#include <memory>
#include <string>

#include "WebElement.hpp"

/**
 * @brief C++ wrapper for a web UI button (HTML <button>) in the Emscripten
 * interface.
 *
 * This class stores button state (label, visibility, enabled), click
 * callbacks, and handles secure file uploads via hidden DOM inputs.
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

  // NEW FILE UPLOAD METHODS
  WebButton& SetOnFileUpload(std::function<void(const std::string&)> callback);
  WebButton& SetOnFileUploadWithName(
      std::function<void(const std::string&, const std::string&)> callback);

  // Embind callbacks (must be public for JS to call them)
  void Click(emscripten::val arg);
  void OnFileChange(emscripten::val event);
  void OnFileReadCompleteInternal(emscripten::val event);

 private:
  std::string label_;
  bool visible_{true};
  bool enabled_{true};
  std::function<void()> on_click_{};

  // File Upload variables
  std::function<void(const std::string&)> on_file_upload_{};
  std::function<void(const std::string&, const std::string&)>
      on_file_upload_with_name_{};
  std::string last_uploaded_filename_{};
  emscripten::val file_input_ = emscripten::val::null();

  /// @brief Allows us to remove event listeners on destruction.
  emscripten::val abort_controller_;
};

}  // namespace cse498
#pragma once

#include <functional>
#include <string>

class WebButton {
public:
  WebButton(const std::string& label);
  WebButton(const std::string& label, const std::string& id);

  void SetLabel(const std::string& label);
  std::string GetLabel() const;

  void Show();
  void Hide();
  bool IsVisible() const;

  void Enable();
  void Disable();
  bool IsEnabled() const;

  void SetOnClick(std::function<void()> callback);

  std::string GetId() const;

  // For testing/demo
  void Click();

private:
  std::string label_;
  std::string id_;
  bool visible_{true};
  bool enabled_{true};
  std::function<void()> on_click_{};
};

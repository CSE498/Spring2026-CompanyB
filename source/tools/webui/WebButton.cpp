#include "WebButton.h"

#include <cassert>
#include <emscripten.h>
#include <utility>

namespace cse498 {

using emscripten::val;

WebButton::WebButton(const std::string& label, const WebOptions& options)
    : WebElement("button", options), label_(label) {
  dom_element.set("innerText", label_);

  // Basic default state.
  dom_element["style"].set("display", std::string("block"));
  dom_element.set("disabled", false);
}

WebButton::~WebButton() {
}

const std::string& WebButton::GetLabel() const { return label_; }

void WebButton::SetLabel(const std::string& label) {
  label_ = label;
  dom_element.set("innerText", label_);
}

void WebButton::Show() {
  visible_ = true;
  dom_element["style"].set("display", std::string("block"));
}

void WebButton::Hide() {
  visible_ = false;
  dom_element["style"].set("display", std::string("none"));
}

bool WebButton::IsVisible() const { return visible_; }

void WebButton::Enable() {
  enabled_ = true;
  dom_element.set("disabled", false);
}

void WebButton::Disable() {
  enabled_ = false;
  dom_element.set("disabled", true);
}

bool WebButton::IsEnabled() const { return enabled_; }

WebButton& WebButton::SetOnClick(std::function<void()> callback) {
  on_click_ = std::move(callback);

  return *this;
}

void WebButton::Click() {
  if (!visible_ || !enabled_) return;
  if (on_click_) on_click_();
}

}

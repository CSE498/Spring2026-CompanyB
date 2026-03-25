#include "WebButton.h"

#include <cassert>
#include <emscripten.h>
#include <utility>

namespace cse498 {

using emscripten::val;

WebButton::WebButton(const std::string& label, const std::string& id)
    : WebElement(id, true), label_(label) {
  assert(!id.empty() && "Button id must not be empty");

  val document = val::global("document");

  // Make sure the ID is unique.
  val existing = document.call<val>("getElementById", id);
  assert((existing.isNull() || existing.isUndefined()) &&
         "Button with this id already exists");

  // Create the <button> element.
  button_element_ = document.call<val>("createElement", std::string("button"));
  button_element_.set("id", id);
  button_element_.set("innerText", label_);

  // Basic default state.
  button_element_["style"].set("display", std::string("block"));
  button_element_.set("disabled", false);

  // Add to page.
  document["body"].call<void>("appendChild", button_element_);
}

WebButton::~WebButton() {
  if (button_element_.isNull() || button_element_.isUndefined()) return;

  val document = val::global("document");
  val body = document["body"];
  body.call<void>("removeChild", button_element_);
}

const std::string& WebButton::GetLabel() const { return label_; }

void WebButton::SetLabel(const std::string& label) {
  label_ = label;
  button_element_.set("innerText", label_);
}

void WebButton::Show() {
  visible_ = true;
  button_element_["style"].set("display", std::string("block"));
}

void WebButton::Hide() {
  visible_ = false;
  button_element_["style"].set("display", std::string("none"));
}

bool WebButton::IsVisible() const { return visible_; }

void WebButton::Enable() {
  enabled_ = true;
  button_element_.set("disabled", false);
}

void WebButton::Disable() {
  enabled_ = false;
  button_element_.set("disabled", true);
}

bool WebButton::IsEnabled() const { return enabled_; }

void WebButton::SetOnClick(std::function<void()> callback) {
  on_click_ = std::move(callback);
}

void WebButton::Click() {
  if (!visible_ || !enabled_) return;
  if (on_click_) on_click_();
}

} 
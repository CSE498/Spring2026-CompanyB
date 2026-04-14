#include "WebButton.h"

#include <cassert>
#include <emscripten.h>
#include <utility>
#include <emscripten/bind.h>

namespace cse498 {

using emscripten::val;

WebButton::WebButton(const std::string& label, const WebOptions& options)
    : WebElement("button", options), label_(label) {
  dom_element.set("innerText", label_);

  // Basic default state.
  dom_element["style"].set("display", std::string("block"));
  dom_element.set("disabled", false);

  abort_controller_ = val::global("AbortController").new_();
}

WebButton::~WebButton() {
  #ifdef DEBUG_LOG_WEB_ELEMENTS
  if (!id.empty()) {
    std::printf("WebButton #%s destructed\n", id.c_str());
  }
  else std::printf("WebButton destructed\n");
  #endif

  // Remove event listener from button
  abort_controller_.call<void>("abort");
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

  auto buttonHandle = emscripten::val(std::dynamic_pointer_cast<cse498::WebButton>(shared_from_this()));

  val config = emscripten::val::object();
  config.set("signal", abort_controller_["signal"]);

  dom_element.call<void>(
    "addEventListener",
    std::string("click"),
    buttonHandle["onClick"].call<val>("bind", buttonHandle),
    config
  );

  return *this;
}

void WebButton::Click(val arg) {
  if (!visible_ || !enabled_) return;
  if (on_click_) on_click_();
}
}

EMSCRIPTEN_BINDINGS(button_bindings) {
  emscripten::class_<cse498::WebButton>("WebButton")
    .smart_ptr<std::shared_ptr<cse498::WebButton>>("std::shared_ptr<WebButton>")
    .function("onClick", &cse498::WebButton::Click);
}
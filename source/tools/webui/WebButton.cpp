#include "WebButton.h"

#include <emscripten.h>
#include <emscripten/bind.h>

#include <cassert>
#include <utility>

namespace cse498 {

using emscripten::val;

WebButton::WebButton(const std::string& label, const WebOptions& options)
    : WebElement("button", options), label_(label) {
  dom_element.set("innerText", label_);

  // Basic default state.
  dom_element["style"].set("display", std::string("block"));
  dom_element.set("disabled", false);

  // Safely initialize AbortController only if it exists in the environment
  val globalThis = val::global("globalThis");
  if (!globalThis["AbortController"].isUndefined()) {
    abort_controller_ = val::global("AbortController").new_();
  }
}

WebButton::~WebButton() {
#ifdef DEBUG_LOG_WEB_ELEMENTS
  if (!id.empty()) {
    std::printf("WebButton #%s destructed\n", id.c_str());
  } else
    std::printf("WebButton destructed\n");
#endif

  // Safely remove event listener from button
  if (!abort_controller_.isNull() && !abort_controller_.isUndefined()) {
    abort_controller_.call<void>("abort");
  }

  // Safely remove the hidden file input if we generated one
  if (!file_input_.isNull() && !file_input_.isUndefined()) {
    file_input_.call<void>("remove");
  }
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

  auto buttonHandle = emscripten::val(
      std::dynamic_pointer_cast<cse498::WebButton>(shared_from_this()));

  val config = emscripten::val::object();

  // Apply the abort signal only if the controller successfully initialized
  if (!abort_controller_.isNull() && !abort_controller_.isUndefined()) {
    config.set("signal", abort_controller_["signal"]);
  }

  dom_element.call<void>(
      "addEventListener", std::string("click"),
      buttonHandle["onClick"].call<val>("bind", buttonHandle), config);

  return *this;
}

// --- FILE UPLOAD IMPLEMENTATION ---

WebButton& WebButton::SetOnFileUpload(
    std::function<void(const std::string&)> callback) {
  on_file_upload_ = std::move(callback);

  // If we haven't created the hidden input yet, do it now.
  if (file_input_.isNull() || file_input_.isUndefined()) {
    val document = val::global("document");

    // Safety check: Don't try to build DOM nodes if there is no document
    // (Headless safety)
    if (!document.isUndefined()) {
      file_input_ = document.call<val>("createElement", std::string("input"));
      file_input_.set("type", "file");
      file_input_["style"].set("display", "none");

      // Attach it safely to the body so it exists in the DOM
      document["body"].call<void>("appendChild", file_input_);

      auto buttonHandle = emscripten::val(
          std::dynamic_pointer_cast<cse498::WebButton>(shared_from_this()));

      val config = emscripten::val::object();

      if (!abort_controller_.isNull() && !abort_controller_.isUndefined()) {
        config.set("signal", abort_controller_["signal"]);
      }

      // Bind the 'change' event (fires when the user selects a file from the OS
      // menu)
      file_input_.call<void>(
          "addEventListener", std::string("change"),
          buttonHandle["onFileChange"].call<val>("bind", buttonHandle), config);
    }
  }

  // Ensure we register the button click itself so it can trigger the hidden
  // input
  if (!on_click_) {
    SetOnClick([]() {});  // Setup baseline click listener if one doesn't exist
  }

  return *this;
}

void WebButton::Click(val arg) {
  if (!visible_ || !enabled_) return;

  // If this button is designated for uploads, proxy the click to the hidden
  // input!
  if (on_file_upload_ && !file_input_.isNull() && !file_input_.isUndefined()) {
    file_input_.call<void>("click");
  } else if (on_click_) {
    on_click_();
  }
}

void WebButton::OnFileChange(emscripten::val event) {
  val target = event["target"];
  val files = target["files"];

  if (files["length"].as<int>() == 0) return;

  // Headless safety check: Ensure FileReader exists before trying to call new()
  val globalFR = val::global("FileReader");
  if (globalFR.isUndefined()) return;

  val file = files[0];
  val reader = globalFR.new_();

  // Bind the async file reader complete event to our internal callback
  auto buttonHandle = emscripten::val(
      std::dynamic_pointer_cast<cse498::WebButton>(shared_from_this()));
  reader.set("onload", buttonHandle["onFileReadCompleteInternal"].call<val>(
                           "bind", buttonHandle));

  reader.call<void>("readAsText", file);
}

void WebButton::OnFileReadCompleteInternal(emscripten::val event) {
  val target = event["target"];
  std::string file_content = target["result"].as<std::string>();

  // Reset the input value so the exact same file can be uploaded again if
  // needed
  if (!file_input_.isNull() && !file_input_.isUndefined()) {
    file_input_.set("value", "");
  }

  // Hand the raw text over to the C++ backend!
  if (on_file_upload_) {
    on_file_upload_(file_content);
  }
}

}  // namespace cse498

EMSCRIPTEN_BINDINGS(button_bindings) {
  emscripten::class_<cse498::WebButton>("WebButton")
      .smart_ptr<std::shared_ptr<cse498::WebButton>>(
          "std::shared_ptr<WebButton>")
      .function("onClick", &cse498::WebButton::Click)
      .function("onFileChange", &cse498::WebButton::OnFileChange)
      .function("onFileReadCompleteInternal",
                &cse498::WebButton::OnFileReadCompleteInternal);
}
#include "WebButton.h"

#include <stdexcept>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

namespace cse498 {

WebButton::WebButton(const std::string& label, const std::string& id)
    : label_(label), id_(id) {
  if (id_.empty()) {
    throw std::invalid_argument("Button id cannot be empty.");
  }
}

void WebButton::SetLabel(const std::string& label) { label_ = label; }

const std::string& WebButton::GetLabel() const { return label_; }

void WebButton::SetId(const std::string& id) {
  if (id.empty()) {
    throw std::invalid_argument("Button id cannot be empty.");
  }
  id_ = id;
}

const std::string& WebButton::GetId() const { return id_; }

void WebButton::SetColor(const std::string& color) { color_ = color; }
const std::string& WebButton::GetColor() const { return color_; }

void WebButton::SetSize(int width, int height) {
  width_ = width;
  height_ = height;
}
void WebButton::Show() { visible_ = true; }
void WebButton::Hide() { visible_ = false; }
bool WebButton::IsVisible() const { return visible_; }

void WebButton::Enable() { enabled_ = true; }
void WebButton::Disable() { enabled_ = false; }
bool WebButton::IsEnabled() const { return enabled_; }

void WebButton::SetOnClick(std::function<void()> callback) {
  on_click_ = std::move(callback);
}

void WebButton::Click() {
  if (!visible_ || !enabled_) return;
  if (on_click_) on_click_();
}

int WebButton::GetWidth() const { return width_; }
int WebButton::GetHeight() const { return height_; }

}
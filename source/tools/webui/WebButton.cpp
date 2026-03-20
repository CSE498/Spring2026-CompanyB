#include "WebButton.h"

#include <cassert>
#include <utility>

WebButton::WebButton(const std::string& label, const std::string& id)
    : label_(label), id_(id) {
  assert(!id_.empty());
}

void WebButton::SetLabel(const std::string& label) { label_ = label; }

const std::string& WebButton::GetLabel() const { return label_; }

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
const std::string& WebButton::GetId() const { return id_; }

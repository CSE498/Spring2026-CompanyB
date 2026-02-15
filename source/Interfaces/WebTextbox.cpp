/**
 * @file WebTextbox.cpp
 * @brief Implementation of WebTextbox using Embind
 */

#include "WebTextbox.hpp"
#include <cassert>
#include <iostream>

using namespace emscripten;

namespace cse498 {

WebTextbox::WebTextbox(const std::string& id, const TextStyle& style)
    : id(id)
{
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) {
        return; // Headless Mode
    }

    val document = val::global("document");
    val existing = document.call<val>("getElementById", id);
    assert((existing.isNull() || existing.isUndefined()) && "WebTextbox ID already exists in DOM");

    div_element = document.call<val>("createElement", std::string("div"));
    div_element.set("id", id);

    div_element["style"].set("position", "absolute");
    div_element["style"].set("border", "1px solid #ccc");
    div_element["style"].set("padding", "5px");
    div_element["style"].set("overflow", "auto");
    div_element["style"].set("zIndex", "9999");

    document["body"].call<void>("appendChild", div_element);
    SetStyle(style);
}

WebTextbox::~WebTextbox() {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;

    if (!div_element.isNull() && !div_element.isUndefined()) {
        div_element.call<void>("remove");
    }
}

    void WebTextbox::SetText(const std::string& text) {
    // 1. Calculate the safe, trimmed text first
    std::string safe_text = text.length() > max_length ? text.substr(text.length() - max_length) : text;

    // 2. Apply it to either the mock (headless) or the real DOM
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) {
        mock_text_content = safe_text;
        return;
    }

    div_element.set("innerText", safe_text);
}

void WebTextbox::AppendText(const std::string& text) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) {
        mock_text_content += text;
        if (mock_text_content.length() > max_length) {
            mock_text_content = mock_text_content.substr(mock_text_content.length() - max_length);
        }
        return;
    }

    std::string current = div_element["innerText"].as<std::string>();
    current += text;

    // Prevent DOM memory overflow during long simulations
    if (current.length() > max_length) {
        current = current.substr(current.length() - max_length);
    }

    div_element.set("innerText", current);
    div_element.set("scrollTop", div_element["scrollHeight"]); // Auto-scroll
}

void WebTextbox::Clear() {
    SetText("");
}

std::string WebTextbox::GetText() const {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) {
        return mock_text_content;
    }

    if (div_element.isNull() || div_element.isUndefined()) return "";
    return div_element["innerText"].as<std::string>();
}

void WebTextbox::SetStyle(const TextStyle& style) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;

    val css = div_element["style"];
    css.set("fontFamily", style.fontFamily);
    css.set("fontSize", std::to_string(style.fontSize) + "px");
    css.set("color", style.color);
    css.set("backgroundColor", style.backgroundColor);
    css.set("fontWeight", style.bold ? "bold" : "normal");
}

void WebTextbox::SetClass(const std::string& css_class) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;
    div_element.set("className", css_class);
}

void WebTextbox::SetVisible(bool visible) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;
    div_element["style"].set("display", visible ? "block" : "none");
}

void WebTextbox::SetPosition(int x, int y) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;

    val css = div_element["style"];
    css.set("left", std::to_string(x) + "px");
    css.set("top", std::to_string(y) + "px");
}

void WebTextbox::SetSize(int width, int height) {
    val global = val::global("globalThis");
    if (global["document"].isUndefined()) return;

    val css = div_element["style"];
    css.set("width", std::to_string(width) + "px");
    css.set("height", std::to_string(height) + "px");
}

void WebTextbox::SetMaxLength(size_t length) {
    max_length = length;
}

}
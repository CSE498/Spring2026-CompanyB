#pragma once

#include <emscripten/val.h>
#include <string>
#include <tuple>

namespace cse498 {

    struct TextStyle {
        std::string fontFamily = "Arial";
        int fontSize = 16;
        std::string color = "black";
        std::string backgroundColor = "white";
        bool bold = false;
    };

    class WebTextbox {
    private:
        std::string id;
        emscripten::val div_element = emscripten::val::null();

        // ADD THIS: For headless testing support
        std::string mock_text_content;

    public:
        WebTextbox(const std::string& id, const TextStyle& style = TextStyle());

        // Delete copy constructors
        WebTextbox(const WebTextbox&) = delete;
        WebTextbox& operator=(const WebTextbox&) = delete;

        ~WebTextbox();

        void SetText(const std::string& text);
        void AppendText(const std::string& text);
        void SetStyle(const TextStyle& style);
        void SetPosition(int x, int y);
        void SetSize(int width, int height);

        // ADD THIS: The missing function!
        [[nodiscard]] std::string GetText() const;
    };

} // namespace cse498
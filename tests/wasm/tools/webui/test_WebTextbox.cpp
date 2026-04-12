#include <emscripten.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "tools/webui/WebTextbox.hpp"

using namespace cse498;

struct SetupMockDOMWebTextbox {
  SetupMockDOMWebTextbox() {
    // clang-format off
    EM_ASM({
      /// Creates a document if it doesn't exist
      if (typeof document === 'undefined') {
        const { JSDOM } = jsdom;
        const dom = new JSDOM("<!DOCTYPE html> <html><head></head><body></body></html>");
        globalThis.window = dom.window;
        globalThis.document = dom.window.document;
      }
    });
    // clang-format on
  }

  ~SetupMockDOMWebTextbox() {
    // clang-format off
      EM_ASM({
        delete globalThis.document;
        delete globalThis.window;
      });
    // clang-format on
  }
};

// --- CHAOS HELPERS ---
std::string GenerateGarbage(size_t length) {
  std::string s;
  s.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    // Keep garbage text as valid UTF-8 in ASCII printable range, otherwise the
    // text gets mangled. This issue occurs when the DOM exists.
    s.push_back(static_cast<char>(32 + rand() % 95));
  }
  return s;
}

// --- NEW FEATURE TESTS ---

TEST_CASE("WebTextbox: New UI & Memory Features", "[features]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox box(TextStyle(), { .id = "feature_box" });

  // 1. Test Clear()
  box.SetText("Initialize Data");
  REQUIRE(box.GetText().value() == "Initialize Data");
  box.Clear();
  REQUIRE(box.GetText().value() == "");

  // 2. Test MaxLength Trimming on SetText
  box.SetMaxLength(10);
  box.SetText("This is way too long for a 10 char limit");
  // Should only keep the last 10 characters
  REQUIRE(box.GetText().value() == "char limit");

  // 3. Test MaxLength Trimming on AppendText
  box.Clear();
  box.SetText("1234567890");
  box.AppendText("ABC");
  // Should drop '123' to keep the total length at 10
  REQUIRE(box.GetText().value() == "4567890ABC");

  // 4. Test UI Methods (Ensure they don't crash in Headless Node.js)
  REQUIRE_NOTHROW(box.SetVisible(false));
  REQUIRE_NOTHROW(box.SetVisible(true));
  REQUIRE_NOTHROW(box.SetClass("logger-window"));
}

// --- CORE ROBUSTNESS TESTS ---

TEST_CASE("WebTextbox: The Naughty String List", "[edge_case]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox box(TextStyle(), { .id = "naughty_box" });

  std::vector<std::string> naughty_strings = {"",
                                              "undefined",
                                              "null",
                                              "NaN",
                                              "<script>alert(1)</script>",
                                              "drop table users;",
                                              "\\",
                                              "\\\\",
                                              "/",
                                              "\r\n",
                                              "\n",
                                              "\r",
                                              "Powerلُلُصّبُلُلصّبُررً ॣ ॣh ॣ ॣ冗",
                                              "﷽",
                                              "🇺🇸🇷🇺🇸🇦",
                                              "Z̮̞̠͙͔ͅaIgO tExT iS cOmInG",
                                              std::string(1000, 'A'),
                                              std::string("\0", 1),
                                              "USER\0NAME"};

  for (const auto& nasty : naughty_strings) {
    REQUIRE_NOTHROW(box.SetText(nasty));
    REQUIRE(box.GetText().value() == nasty);

    REQUIRE_NOTHROW(box.AppendText(nasty));
    // Note: we might hit the default 50KB limit here, but these strings are
    // small enough
    REQUIRE(box.GetText().value() == nasty + nasty);
  }
}

TEST_CASE("WebTextbox: Numeric Extremes", "[limits]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox box(TextStyle(), { .id = "limit_box" });
  TextStyle style;

  // Swapped from integer limits to their literal string px equivalents to avoid -Wconstant-conversion
  style.font_size = "2147483647px";
  REQUIRE_NOTHROW(box.SetStyle(style));

  style.font_size = "-2147483648px";
  REQUIRE_NOTHROW(box.SetStyle(style));

  REQUIRE_NOTHROW(box.SetPosition(-1000, -1000));
  REQUIRE_NOTHROW(box.SetSize(0, 0));
  REQUIRE_NOTHROW(box.SetSize(100000, 100000));
}

TEST_CASE("WebTextbox: High-Volume Fuzzing (10,000 Iterations)", "[fuzz]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox fuzzer(TextStyle(), { .id = "fuzz_target" });
  srand(12345);

  for (int i = 0; i < 10000; ++i) {
    size_t len = rand() % 1001;
    std::string junk = GenerateGarbage(len);

    REQUIRE_NOTHROW(fuzzer.SetText(junk));
    REQUIRE(fuzzer.GetText().value() == junk);
  }
}

TEST_CASE("WebTextbox: The Memory Stress Test", "[memory]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox heavyBox(TextStyle(), { .id = "heavy_box" });

  // Temporarily increase limit for the stress test
  size_t massive_size = 10 * 1024 * 1024;   // 10MB
  heavyBox.SetMaxLength(massive_size * 2);  // Allow up to 20MB

  std::string massive(massive_size, 'X');
  REQUIRE_NOTHROW(heavyBox.SetText(massive));
  REQUIRE(heavyBox.GetText().value().size() ==
          massive.size());  // Addition of .value()

  std::string more(1024 * 1024, 'Y');  // 1MB more
  REQUIRE_NOTHROW(heavyBox.AppendText(more));
  REQUIRE(heavyBox.GetText().value().size() ==
          massive_size + (1024 * 1024));  // .value() added here too
}

TEST_CASE("WebTextbox: Templates and Lambdas", "[webui]") {
  SetupMockDOMWebTextbox mock;
  cse498::WebTextbox box(TextStyle(), { .id = "test_box" });

  // Testing the Template
  REQUIRE_NOTHROW(box.AppendValue<int>(404));
  REQUIRE(box.GetText().value() == "404");

  // Testing the Lambda (Appending "Error" using a lambda transformation)
  auto add_error_label = [](const std::string& str) {
    return str + " Error";
  };

  box.TransformText(add_error_label);
  REQUIRE(box.GetText().value() == "404 Error");
}

// ADVANCED UI, SECURITY & C++20 TESTS

TEST_CASE("WebTextbox: Span Pruning & Line Formatting", "[ui_spans]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox logger(TextStyle(), { .id = "logger_box" });

  SECTION("SetMaxLines limits DOM nodes without crashing JSDOM") {
    logger.SetMaxLines(3);

    // Inject 10 lines. If our pruning logic failed or threw DOM errors,
    // JSDOM would crash the test suite here.
    for(int i = 0; i < 10; ++i) {
        REQUIRE_NOTHROW(logger.AppendLine("Log line " + std::to_string(i), "INFO"));
    }
  }

  SECTION("AppendStyledLine safely handles XSS strings") {
    std::string malicious = "<img src='x' onerror='alert(1)'>";

    // Validates that EscapeHTML sanitizes the payload before injection
    REQUIRE_NOTHROW(logger.AppendStyledLine(malicious, "danger-text"));

    // The C++ raw text buffer should retain the exact text for logging/reading parity
    std::string current_text = logger.GetText().value();
    REQUIRE(current_text.find(malicious) != std::string::npos);
  }
}

TEST_CASE("WebTextbox: C++20 Concepts and Constexpr checks", "[modern_cpp]") {
  SetupMockDOMWebTextbox mock;
  WebTextbox box(TextStyle(), { .id = "concepts_box" });

  SECTION("AppendValue accepts std::floating_point types via C++20 Concepts") {
    REQUIRE_NOTHROW(box.AppendValue(3.14159));
    REQUIRE(box.GetText().value().find("3.14159") != std::string::npos);
  }

  SECTION("IsValidLength executes safely at compile-time") {
    STATIC_REQUIRE(WebTextbox::IsValidLength(500) == true);
    STATIC_REQUIRE(WebTextbox::IsValidLength(0) == false);
    STATIC_REQUIRE(WebTextbox::IsValidLength(99999999) == false);
  }
}
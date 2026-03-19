#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "tools/webui/WebTextbox.hpp"

using namespace cse498;

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
  WebTextbox box("feature_box");

  // "SECTION" is mocked by our MiniTest macro, so this just runs sequentially

  // 1. Test Clear()
  box.SetText("Initialize Data");
  REQUIRE(box.GetText() == "Initialize Data");
  box.Clear();
  REQUIRE(box.GetText() == "");

  // 2. Test MaxLength Trimming on SetText
  box.SetMaxLength(10);
  box.SetText("This is way too long for a 10 char limit");
  // Should only keep the last 10 characters
  REQUIRE(box.GetText() == "char limit");

  // 3. Test MaxLength Trimming on AppendText
  box.Clear();
  box.SetText("1234567890");
  box.AppendText("ABC");
  // Should drop '123' to keep the total length at 10
  REQUIRE(box.GetText() == "4567890ABC");

  // 4. Test UI Methods (Ensure they don't crash in Headless Node.js)
  REQUIRE_NOTHROW(box.SetVisible(false));
  REQUIRE_NOTHROW(box.SetVisible(true));
  REQUIRE_NOTHROW(box.SetClass("logger-window"));
}

// --- CORE ROBUSTNESS TESTS ---

TEST_CASE("WebTextbox: The Naughty String List", "[edge_case]") {
  WebTextbox box("naughty_box");

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
    REQUIRE(box.GetText() == nasty);

    REQUIRE_NOTHROW(box.AppendText(nasty));
    // Note: we might hit the default 50KB limit here, but these strings are
    // small enough
    REQUIRE(box.GetText() == nasty + nasty);
  }
}

TEST_CASE("WebTextbox: Numeric Extremes", "[limits]") {
  WebTextbox box("limit_box");
  TextStyle style;

  style.font_size = std::numeric_limits<int>::max();
  REQUIRE_NOTHROW(box.SetStyle(style));

  style.font_size = std::numeric_limits<int>::min();
  REQUIRE_NOTHROW(box.SetStyle(style));

  REQUIRE_NOTHROW(box.SetPosition(-1000, -1000));
  REQUIRE_NOTHROW(box.SetSize(0, 0));
  REQUIRE_NOTHROW(box.SetSize(100000, 100000));
}

TEST_CASE("WebTextbox: High-Volume Fuzzing (10,000 Iterations)", "[fuzz]") {
  WebTextbox fuzzer("fuzz_target");
  srand(12345);

  for (int i = 0; i < 10000; ++i) {
    size_t len = rand() % 1001;
    std::string junk = GenerateGarbage(len);

    REQUIRE_NOTHROW(fuzzer.SetText(junk));
    REQUIRE(fuzzer.GetText() == junk);
  }
}

TEST_CASE("WebTextbox: The Memory Stress Test", "[memory]") {
  WebTextbox heavyBox("heavy_box");

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

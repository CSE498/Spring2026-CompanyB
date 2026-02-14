#include "MiniTest.hpp"
#include "../Interfaces/WebTextbox.hpp"
#include <random>
#include <iostream>
#include <limits>
#include <vector>

using namespace cse498;

// --- CHAOS HELPERS ---

// Generates absolute garbage (non-printable chars, etc.)
std::string GenerateGarbage(size_t length) {
    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        // Include everything from 0 (NULL) to 255
        s.push_back(static_cast<char>(rand() % 256));
    }
    return s;
}

// --- ROBUSTNESS TESTS ---

TEST_CASE("WebTextbox: The Naughty String List", "[edge_case]") {
    WebTextbox box("naughty_box");

    // A collection of strings known to break systems
    std::vector<std::string> naughty_strings = {
        "", // Empty
        "undefined", "null", "NaN", // JS keywords
        "<script>alert(1)</script>", // XSS
        "drop table users;", // SQL Injection
        "\\", "\\\\", "/", // Path separators
        "\r\n", "\n", "\r", // Newlines
        "Powerلُلُصّبُلُلصّبُررً ॣ ॣh ॣ ॣ冗", // Unicode text rendering crashes
        "﷽", // Massive single-glyph character
        "🇺🇸🇷🇺🇸🇦", // Flag emojis (combining sequences)
        "Z̮̞̠͙͔ͅaIgO tExT iS cOmInG", // Zalgo text (stacking diacritics)
        std::string(1000, 'A'), // Buffer overflow attempt
        std::string("\0", 1), // Null terminator logic error
        "USER\0NAME" // Null in middle of string
    };

    for (const auto& nasty : naughty_strings) {
        // 1. Set the nasty string
        REQUIRE_NOTHROW(box.SetText(nasty));

        // 2. Verify we got it back exactly as provided
        // (Note: C++ std::string handles embedded nulls, unlike C-strings)
        REQUIRE(box.GetText() == nasty);

        // 3. Append the nasty string
        REQUIRE_NOTHROW(box.AppendText(nasty));

        // 4. Verify append worked (original + appended)
        REQUIRE(box.GetText() == nasty + nasty);
    }
}

TEST_CASE("WebTextbox: Numeric Extremes", "[limits]") {
    WebTextbox box("limit_box");
    TextStyle style;

    // 1. Test Integer Overflow / Underflow for styles
    style.fontSize = std::numeric_limits<int>::max();
    REQUIRE_NOTHROW(box.SetStyle(style));

    style.fontSize = std::numeric_limits<int>::min(); // Negative font size?
    REQUIRE_NOTHROW(box.SetStyle(style));

    style.fontSize = 0;
    REQUIRE_NOTHROW(box.SetStyle(style));

    // 2. Test Size/Position Limits
    REQUIRE_NOTHROW(box.SetPosition(-1000, -1000)); // Off screen
    REQUIRE_NOTHROW(box.SetSize(0, 0)); // Invisible
    REQUIRE_NOTHROW(box.SetSize(100000, 100000)); // Massive
}

TEST_CASE("WebTextbox: High-Volume Fuzzing (10,000 Iterations)", "[fuzz]") {
    WebTextbox fuzzer("fuzz_target");

    // Deterministic seed so we can reproduce crashes if they happen
    srand(12345);

    for(int i = 0; i < 10000; ++i) {
        // Generate random length between 0 and 1000
        size_t len = rand() % 1001;
        std::string junk = GenerateGarbage(len);

        // STRESS TEST: SetText
        REQUIRE_NOTHROW(fuzzer.SetText(junk));

        // VERIFY: Did data corruption occur?
        if (fuzzer.GetText() != junk) {
            std::cerr << "FAIL on iteration " << i << " with length " << len << std::endl;
        }
        REQUIRE(fuzzer.GetText() == junk);
    }
}

TEST_CASE("WebTextbox: The Memory Stress Test", "[memory]") {
    WebTextbox heavyBox("heavy_box");

    // 1. Build a 10MB string
    std::string massive(10 * 1024 * 1024, 'X');

    // 2. Shove it into the box
    REQUIRE_NOTHROW(heavyBox.SetText(massive));

    // 3. Verify size
    REQUIRE(heavyBox.GetText().size() == massive.size());

    // 4. Append another 1MB
    std::string more(1024 * 1024, 'Y');
    REQUIRE_NOTHROW(heavyBox.AppendText(more));

    // 5. Check final size (11MB)
    REQUIRE(heavyBox.GetText().size() == (11 * 1024 * 1024));
}
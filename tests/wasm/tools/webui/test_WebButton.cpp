
#include <catch2/catch_test_macros.hpp>

#include "tools/webui/WebButton.h"

TEST_CASE("WebButton basics") {
  WebButton b("Start", "startBtn");

  SECTION("Label set/get works") {
    REQUIRE(b.GetLabel() == "Start");
    b.SetLabel("Pause");
    REQUIRE(b.GetLabel() == "Pause");
  }

  SECTION("Visibility works") {
    REQUIRE(b.IsVisible());
    b.Hide();
    REQUIRE_FALSE(b.IsVisible());
    b.Show();
    REQUIRE(b.IsVisible());
  }

  SECTION("Enable/Disable works") {
    REQUIRE(b.IsEnabled());
    b.Disable();
    REQUIRE_FALSE(b.IsEnabled());
    b.Enable();
    REQUIRE(b.IsEnabled());
  }

  SECTION("Click runs callback only when enabled and visible") {
    int count = 0;
    b.SetOnClick([&count]() { ++count; });

    b.Click();
    REQUIRE(count == 1);

    b.Disable();
    b.Click();
    REQUIRE(count == 1);

    b.Enable();
    b.Hide();
    b.Click();
    REQUIRE(count == 1);

    b.Show();
    b.Click();
    REQUIRE(count == 2);
  }
}

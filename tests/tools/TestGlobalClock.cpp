#include <catch2/catch_test_macros.hpp>
#include "../../source/tools/GlobalClock.hpp"

TEST_CASE("GlobalClock functionality", "[GlobalClock]") {
  cse498::GlobalClock::Reset();

  SECTION("Initial state") {
    REQUIRE(cse498::GlobalClock::GetTime() == 0);
  }

  SECTION("Tick with specific delta time") {
    cse498::GlobalClock::Tick(10);
    REQUIRE(cse498::GlobalClock::GetTime() == 10);

    cse498::GlobalClock::Tick(100);
    REQUIRE(cse498::GlobalClock::GetTime() == 110);

    cse498::GlobalClock::Tick(20);
    REQUIRE(cse498::GlobalClock::GetTime() == 130);
  }

  SECTION("Tick with zero delta time") {
    cse498::GlobalClock::Tick(50);
    REQUIRE(cse498::GlobalClock::GetTime() == 50);

    cse498::GlobalClock::Tick(0);
    REQUIRE(cse498::GlobalClock::GetTime() == 50);

    cse498::GlobalClock::Tick(0);
    REQUIRE(cse498::GlobalClock::GetTime() == 50);

    cse498::GlobalClock::Tick(1);
    REQUIRE(cse498::GlobalClock::GetTime() == 51);
  }

  SECTION("Tick with default delta time (1)") {
    cse498::GlobalClock::Tick();
    REQUIRE(cse498::GlobalClock::GetTime() == 1);

    for (int i = 0; i < 5; ++i) {
      cse498::GlobalClock::Tick();
    }
    REQUIRE(cse498::GlobalClock::GetTime() == 6);
  }

  SECTION("Reset functionality") {
    cse498::GlobalClock::Tick(1000);
    REQUIRE(cse498::GlobalClock::GetTime() == 1000);

    cse498::GlobalClock::Reset();
    REQUIRE(cse498::GlobalClock::GetTime() == 0);

    cse498::GlobalClock::Reset();
    REQUIRE(cse498::GlobalClock::GetTime() == 0);


    cse498::GlobalClock::Tick(5);
    REQUIRE(cse498::GlobalClock::GetTime() == 5);

    cse498::GlobalClock::Tick(15);
    REQUIRE(cse498::GlobalClock::GetTime() == 20);
  }
}

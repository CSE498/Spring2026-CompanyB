#include <catch2/catch_test_macros.hpp>

#include "../../source/tools/GlobalClock.hpp"
#include "../../source/tools/TickTimer.hpp"

TEST_CASE("Ensure we can create a timer with the expected initalized values",
          "[TickTimer]") {
  cse498::GlobalClock::Reset();

  cse498::TickTimer testTimer("TEST");
  REQUIRE(testTimer.GetName() == "TEST");
  REQUIRE(testTimer.GetTotalTime() == 0);
}

TEST_CASE("Test start and stop and assosicated methods of getting the time",
          "[TickTimer]") {
  cse498::TickTimer testTimer("TEST");
  cse498::GlobalClock::Reset();
  testTimer.Start();
  cse498::GlobalClock::Tick(100);
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() == 100);

  // time doesn't increase when stopped
  cse498::GlobalClock::Tick(50);
  REQUIRE(testTimer.GetTotalTime() == 100);
}

TEST_CASE("Test reset and associated methods of getting the time", "[TickTimer]") {
  cse498::GlobalClock::Reset();
  cse498::TickTimer testTimer("TEST");

  testTimer.Start();
  cse498::GlobalClock::Tick(100);
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() == 100);

  testTimer.Reset();
  REQUIRE(testTimer.GetTotalTime() == 0);
}

TEST_CASE("Test pause and resume and assosicated methods of getting the time",
          "[TickTimer]") {
  cse498::GlobalClock::Reset();
  cse498::TickTimer testTimer("TEST");

  testTimer.Start();
  cse498::GlobalClock::Tick(100);
  testTimer.Pause();
  REQUIRE(testTimer.GetTotalTime() == 100);
  // time should not increase while paused
  cse498::GlobalClock::Tick(50);
  REQUIRE(testTimer.GetTotalTime() == 100);

  testTimer.Resume();
  cse498::GlobalClock::Tick(50);
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() == 150);
}

TEST_CASE("Test getting time while running", "[TickTimer]") {
  cse498::GlobalClock::Reset();
  cse498::TickTimer testTimer("test");

  testTimer.Start();
  cse498::GlobalClock::Tick(10);
  REQUIRE(testTimer.GetTotalTime() == 10);

  cse498::GlobalClock::Tick(20);
  REQUIRE(testTimer.GetTotalTime() == 30);

  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() == 30);
}

TEST_CASE("Test accumulation across multiple Start/Stop cycles", "[TickTimer]") {
  cse498::GlobalClock::Reset();
  cse498::TickTimer testTimer("test");

  // Cycle 1
  testTimer.Start();
  cse498::GlobalClock::Tick(50);
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() == 50);

  cse498::GlobalClock::Tick(100);  // Time passes while stopped, shouldn't count

  // Cycle 2
  testTimer.Start();
  cse498::GlobalClock::Tick(25);
  testTimer.Stop();

  REQUIRE(testTimer.GetTotalTime() == 25);
}

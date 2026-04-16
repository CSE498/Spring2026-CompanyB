#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "../../source/tools/ProfilingTimer.hpp"

TEST_CASE("Ensure we can create a timer with the expected initalized values",
          "[ProfilingTimer]") {
  cse498::ProfilingTimer testTimer("TEST");
  REQUIRE(testTimer.GetName() == "TEST");
  REQUIRE(testTimer.GetTotalTime() == 0);
}

TEST_CASE("Test start and stop and assosicated methods of getting the time",
          "[ProfilingTimer]") {
  cse498::ProfilingTimer testTimer("TEST");

  testTimer.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  testTimer.Stop();

  uint64_t stoppedTime = testTimer.GetTotalTime();
  REQUIRE(stoppedTime > 0);

  // time doesn't increase when stopped
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  REQUIRE(testTimer.GetTotalTime() == stoppedTime);
}

TEST_CASE("Test reset and associated methods of getting the time",
          "[ProfilingTimer]") {
  cse498::ProfilingTimer testTimer("TEST");

  testTimer.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() > 0);

  testTimer.Reset();
  REQUIRE(testTimer.GetTotalTime() == 0);
}

TEST_CASE("Test pause and resume and assosicated methods of getting the time",
          "[ProfilingTimer]") {
  cse498::ProfilingTimer testTimer("TEST");

  testTimer.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  testTimer.Pause();

  uint64_t pausedTime = testTimer.GetTotalTime();
  REQUIRE(pausedTime > 0);

  // time should not increase while paused
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  REQUIRE(testTimer.GetTotalTime() == pausedTime);

  testTimer.Resume();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() > pausedTime);
}

TEST_CASE("Test getting time while running", "[ProfilingTimer]") {
  cse498::ProfilingTimer testTimer("test");

  testTimer.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  uint64_t midTime = testTimer.GetTotalTime();
  REQUIRE(midTime > 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  uint64_t laterTime = testTimer.GetTotalTime();
  REQUIRE(laterTime > midTime);

  testTimer.Stop();
  REQUIRE(testTimer.GetTotalTime() >= laterTime);
}

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/Timer.hpp"
#include "../../source/tools/GlobalClock.hpp"


TEST_CASE("Ensure we can create a timer with the expected initalized values", "[Timer]") {
    cse498::GlobalClock::Reset();

    cse498::Timer testTimer("TEST");
    REQUIRE(testTimer.GetName() == "TEST");
    REQUIRE(testTimer.GetTotalTime() == 0.0);
    REQUIRE(testTimer.GetTimeInSeconds() == 0.0);
    REQUIRE(testTimer.GetTimeInMinutes() == 0.0);
    REQUIRE(testTimer.GetTimeInHours() == 0.0);
}


TEST_CASE("Test start and stop and assosicated methods of getting the time", "[Timer]") {
	cse498::Timer testTimer("TEST");
    cse498::GlobalClock::Reset();
	testTimer.Start();
    cse498::GlobalClock::Tick(100);
    testTimer.Stop();
    REQUIRE(testTimer.GetTotalTime() == 100.0);
    REQUIRE(testTimer.GetTimeInSeconds() == 0.1);
    REQUIRE(testTimer.GetTimeInMinutes() == Approx(0.1 / 60.0));
    REQUIRE(testTimer.GetTimeInHours() == Approx(0.1 / 3600.0));

    //time doesn't increase when stopped
    cse498::GlobalClock::Tick(50);
    REQUIRE(testTimer.GetTotalTime() == 100.0);
}

TEST_CASE("Test reset and associated methods of getting the time", "[Timer]") {
    cse498::GlobalClock::Reset();
    cse498::Timer testTimer("TEST");

    testTimer.Start();
    cse498::GlobalClock::Tick(100);
    testTimer.Stop();
    REQUIRE(testTimer.GetTotalTime() == 100.0);

    testTimer.Reset();
    REQUIRE(testTimer.GetTotalTime() == 0.0);
    REQUIRE(testTimer.GetTimeInSeconds() == 0.0);
}

TEST_CASE("Test pause and resume and assosicated methods of getting the time", "[Timer]") {
    cse498::GlobalClock::Reset();
    cse498::Timer testTimer("TEST");

    testTimer.Start();
    cse498::GlobalClock::Tick(100);
    testTimer.Pause();
    REQUIRE(testTimer.GetTotalTime() == 100.0);
    //time should not increase while paused
    cse498::GlobalClock::Tick(50);
    REQUIRE(testTimer.GetTotalTime() == 100.0);

    testTimer.Resume();
    cse498::GlobalClock::Tick(50);
    testTimer.Stop();
    REQUIRE(testTimer.GetTotalTime() == 150.0);
}

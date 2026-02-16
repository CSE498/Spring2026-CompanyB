#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/GlobalClock.hpp"



TEST_CASE("Test initial values of Global Clock", "[GlobalClock]") {
    cse498::GlobalClock::Reset();
    REQUIRE(cse498::GlobalClock::GetTime() == 0);

}
TEST_CASE("Test Tick method", "[GlobalClock]") {
    cse498::GlobalClock::Reset();
    cse498::GlobalClock::Tick(10);
    REQUIRE(cse498::GlobalClock::GetTime() == 10);
    cse498::GlobalClock::Tick(100);
    REQUIRE(cse498::GlobalClock::GetTime() == 110);
    cse498::GlobalClock::Reset();
    REQUIRE(cse498::GlobalClock::GetTime() == 0);
    cse498::GlobalClock::Tick(0);
    REQUIRE(cse498::GlobalClock::GetTime() == 0);

    //Beyond this point, it appears that if I keep continuing using tick method without reset, I will not be able to get the values right.

}
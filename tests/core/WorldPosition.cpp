#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../source/core/WorldPosition.hpp"


TEST_CASE("Test WorldPosition Constructors", "[core]")
{
  cse498::WorldPosition wp1;       // Default Constructor
  cse498::WorldPosition wp2(4,5);  // Direct Constructor
  cse498::WorldPosition wp3(0,0);
  cse498::WorldPosition wp4(wp2);  // Copy Constructor

  // Test basic comparisons.
  CHECK(wp1 == wp1);
  CHECK(wp1 != wp2);
  CHECK(wp1 == wp3);
  CHECK(wp1 != wp4);
  CHECK(wp2 == wp4);

  wp4 = cse498::WorldPosition{3,4};
  CHECK(wp2 != wp4);
  wp2 = wp4;
  CHECK(wp2 == wp4);

  // Test CellX() and CellY()
  CHECK(wp2.CellX() == 3);
  CHECK(wp2.CellY() == 4);

  wp4 = cse498::WorldPosition(1000, 2000);
  CHECK(wp4.CellX() == 1000);
  CHECK(wp4.CellY() == 2000);

  // Test more direct setting of values.
  wp1.Set(7,9.5);
  CHECK(wp1.X() == 7.0);
  CHECK(wp1.Y() == 9.5);
  CHECK(wp1.CellX() == 7);
  CHECK(wp1.CellY() == 9);

  CHECK(wp1.Set(10,10).Up().Y() == 9);
}
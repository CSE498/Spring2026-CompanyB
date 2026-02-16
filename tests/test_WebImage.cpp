//#include "catch2/catch.hpp"
#include <catch2/catch_test_macros.hpp>
#include "Interfaces/webui/WebImage.hpp"

TEST_CASE("WebImage constructor creates image with correct properties", "[WebImage]") {
  cse498::WebImage img("test-img-1", "assets/test.png");
  
  REQUIRE(img.GetId() == "test-img-1");
  REQUIRE(img.GetSource() == "assets/test.png");
  REQUIRE(img.GetWidth() == 0);
  REQUIRE(img.GetHeight() == 0);
}

TEST_CASE("WebImage SetSource changes the image source", "[WebImage]") {
  cse498::WebImage img("test-img-2", "initial.png");
  
  img.SetSource("new_image.png");
  
  REQUIRE(img.GetSource() == "new_image.png");
}

TEST_CASE("WebImage SetSize updates dimensions correctly", "[WebImage]") {
  cse498::WebImage img("test-img-3", "test.png");
  
  img.SetSize(200, 150);
  
  REQUIRE(img.GetWidth() == 200);
  REQUIRE(img.GetHeight() == 150);
}

TEST_CASE("WebImage GetSize returns correct dimensions", "[WebImage]") {
  cse498::WebImage img("test-img-4", "test.png");
  
  img.SetSize(300, 250);
  auto [width, height] = img.GetSize();
  
  REQUIRE(width == 300);
  REQUIRE(height == 250);
}

TEST_CASE("WebImage SetPosition updates position correctly", "[WebImage]") {
  cse498::WebImage img("test-img-5", "test.png");
  
  img.SetPosition(100, 50);
  auto [x, y] = img.GetPosition();
  
  REQUIRE(x == 100);
  REQUIRE(y == 50);
}

TEST_CASE("WebImage SetAlt updates alt text", "[WebImage]") {
  cse498::WebImage img("test-img-6", "test.png");
  
  img.SetAlt("A test image");
  
  REQUIRE(img.GetAlt() == "A test image");
}

TEST_CASE("WebImage handles multiple property changes", "[WebImage]") {
  cse498::WebImage img("test-img-7", "original.png");
  
  img.SetSource("updated.png");
  img.SetSize(400, 300);
  img.SetPosition(50, 75);
  img.SetAlt("Updated image");
  
  REQUIRE(img.GetSource() == "updated.png");
  REQUIRE(img.GetWidth() == 400);
  REQUIRE(img.GetHeight() == 300);
  auto [x, y] = img.GetPosition();
  REQUIRE(x == 50);
  REQUIRE(y == 75);
  REQUIRE(img.GetAlt() == "Updated image");
}
/**
 * @file ImageManagerTest.cpp
 * @author Andreea Danila
 */

#include <QApplication>
#include <catch2/catch_test_macros.hpp>

#include "../../source/Interfaces/gui/ImageManager.hpp"

static int argc = 1;
static char arg0[] = "tests";
static char* argv[] = {arg0, nullptr};
static QApplication app(argc, argv);

static const QString kImg1 = "../../images/test1.png";
static const QString kImg2 = "../../images/test2.png";

TEST_CASE("Test ImageManager Constructor", "[ImageManager]") {
  cse498::ImageManager manager1;
  CHECK(!manager1.HasImage("anyImage"));
  CHECK(!manager1.HasImage(""));
}

TEST_CASE("Test ImageManager Load", "[ImageManager]") {
  cse498::ImageManager manager1;

  CHECK(manager1.Load("image1", kImg1));
  CHECK(manager1.HasImage("image1"));

  CHECK(!manager1.Load("", kImg1));

  CHECK(!manager1.Load("image2", ""));
  CHECK(!manager1.HasImage("image2"));

  CHECK(!manager1.Load("image3", "fake/path.png"));
  CHECK(!manager1.HasImage("image3"));

  CHECK(manager1.Load("image1", kImg2));
  CHECK(manager1.HasImage("image1"));
}

TEST_CASE("Test ImageManager HasImage", "[ImageManager]") {
  cse498::ImageManager manager1;

  CHECK(!manager1.HasImage("image1"));

  CHECK(manager1.Load("image1", kImg1));  // fixed
  CHECK(manager1.HasImage("image1"));

  CHECK(!manager1.HasImage("image2"));
}

TEST_CASE("Test ImageManager GetImage", "[ImageManager]") {
  cse498::ImageManager manager1;
  CHECK(manager1.Load("image1", kImg1));  // fixed

  auto result = manager1.GetImage("image1");
  REQUIRE(result.has_value());
  CHECK(!result->isNull());
  CHECK(result->width() > 0);
  CHECK(result->height() > 0);

  auto missing = manager1.GetImage("nonexistent");
  CHECK(!missing.has_value());
}

TEST_CASE("Test ImageManager Remove", "[ImageManager]") {
  cse498::ImageManager manager1;
  CHECK(manager1.Load("image1", kImg1));  // fixed
  CHECK(manager1.Load("image2", kImg2));  // fixed

  CHECK(manager1.Remove("image2"));
  CHECK(!manager1.HasImage("image2"));
  CHECK(manager1.HasImage("image1"));

  CHECK(manager1.Remove("image1"));
  CHECK(!manager1.HasImage("image1"));

  CHECK(!manager1.Remove("image1"));

  CHECK(manager1.Load("image1", kImg1));  // fixed
  CHECK(manager1.HasImage("image1"));
}

TEST_CASE("Test ImageManager Clear", "[ImageManager]") {
  cse498::ImageManager manager1;
  CHECK(manager1.Load("image1", kImg1));  // fixed
  CHECK(manager1.Load("image2", kImg2));  // fixed

  manager1.Clear();
  CHECK(!manager1.HasImage("image1"));
  CHECK(!manager1.HasImage("image2"));
}
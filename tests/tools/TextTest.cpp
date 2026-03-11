#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

#include "../../source/Interfaces/gui/Text.hpp"


TEST_CASE("Test Text Constructor", "[core]")
{
  // testing getters 
  cse498::Text t1("Hello World", "Arial", "blue", 22, true, false, true, true);
  CHECK(t1.GetContent() == "Hello World");
  CHECK(t1.GetFont().family() == "Arial");
  CHECK(t1.GetColor() == QColor("blue"));
  CHECK(t1.GetSize() == 22);
  CHECK(t1.GetBold() == true);
  CHECK(t1.GetUnderline() == false);
  CHECK(t1.GetItalics() == true);
  CHECK(t1.GetUppercase() == true);

  // testing setters (with getters)
  t1.SetContent("Bye World");
  CHECK(t1.GetContent() == "Bye World");
  t1.SetFont("Arial");
  CHECK(t1.GetFont().family() == "Arial");
  t1.SetColor("Blue");
  CHECK(t1.GetColor()== QColor("blue"));
  t1.SetSize(22);
  CHECK(t1.GetSize() == 22);
  t1.SetBold(true);
  CHECK(t1.GetBold() == true);
  t1.SetUnderline(false);
  CHECK(t1.GetUnderline() == false);
  t1.SetItalics(true);
  CHECK(t1.GetItalics() == true);
  t1.SetUnderline(true);
  CHECK(t1.GetUnderline() == true);
}

TEST_CASE("Test Text Constructor (testing other edge cases)", "[core]")
{
  cse498::Text t1(" ", "Times New Roman", "#6c579c", 555, false, true, false, false);
  CHECK(t1.GetContent() == " ");
  CHECK(t1.GetFont().family() == "Times New Roman");
  CHECK(t1.GetColor() == QColor("#6c579c"));
  CHECK(t1.GetSize() == 555);
  CHECK(t1.GetBold() == false);
  CHECK(t1.GetUnderline() == true);
  CHECK(t1.GetItalics() == false);
  CHECK(t1.GetUppercase() == false);
}

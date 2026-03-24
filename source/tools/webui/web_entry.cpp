/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <memory>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
static std::shared_ptr<WebCanvas> canvas;
static std::unique_ptr<WebLayout> layout;

int main() {
  layout = std::make_unique<WebLayout>("app-layout");
  layout->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");

  auto menu_bar = std::make_shared<WebLayout>("menu-bar");
  menu_bar->SetHeight("80px");

  // auto start_btn = std::make_shared<WebButton>("Start", "start-btn");
  // start_btn->SetOnClick([]() {
  //   canvas->Clear();
  //   canvas->SetFillColor({0, 200, 100});
  //   canvas->DrawText("Running...", 10, 30);
  // });
  // menu_bar->AddChild(start_btn);

  layout->AddChild(menu_bar);

  auto main_layout = std::make_shared<WebLayout>("main-layout");
  main_layout->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start");
  layout->AddChild(main_layout);

  auto left_column = std::make_shared<WebLayout>("left-column");
  left_column->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch");
  main_layout->AddChild(left_column);

  // auto text = std::make_shared<WebTextbox>("test-button");
  // text->SetText("Example text");
  // left_column->AddChild(text);

  canvas = std::make_shared<WebCanvas>(800, 600, "game-canvas");
  canvas->SetBackgroundColor({0, 0, 0});
  canvas->SetFillColor({255, 255, 255});
  canvas->SetFont("48px arial");
  canvas->DrawText("Hello from WebAssembly!", 10, 50);

  main_layout->AddChild(canvas);

  EM_ASM({
    document.querySelector("body").style.background = "#85B790";

    Object.assign(document.getElementById("main-layout").style, {
      width: "100%",
    });

    Object.assign(document.getElementById("left-column").style, {
      flex: "1 0 150px",
      background: "#555",
      borderRadius: "20px",
    });

    Object.assign(document.getElementById("menu-bar").style, {
      width: "100%",
      background: "#363636",
      borderRadius: "20px",
    });
  });

  return 0;
}

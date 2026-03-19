/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <memory>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebLayout.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
static std::shared_ptr<WebCanvas> canvas;
static std::unique_ptr<WebButton> start_btn;
static std::unique_ptr<WebLayout> layout;

int main() {
  layout = std::make_unique<WebLayout>("app-layout");
  layout->SetDirection("column").SetAlignItems("center").SetGap("10px");

  canvas = std::make_shared<WebCanvas>(800, 600, "game-canvas");
  canvas->SetBackgroundColor({0, 0, 0});
  canvas->SetFillColor({255, 255, 255});
  canvas->SetFont("48px arial");
  canvas->DrawText("Hello from WebAssembly!", 10, 50);

  start_btn = std::make_unique<WebButton>("Start", "start-btn");
  start_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  layout->AddChild(canvas);

  return 0;
}

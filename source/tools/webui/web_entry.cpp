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
  menu_bar->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px");

  auto start_btn = std::make_shared<WebButton>("", "start-btn");
  start_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  auto pause_btn = std::make_shared<WebButton>("", "pause-btn");
  pause_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  auto stop_btn = std::make_shared<WebButton>("", "stop-btn");
  stop_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30); 
  });

  auto upload_btn = std::make_shared<WebButton>("", "upload-btn");
  upload_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  auto save_btn = std::make_shared<WebButton>("", "save-btn");
  save_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  auto exit_btn = std::make_shared<WebButton>("", "exit-btn");
  exit_btn->SetOnClick([]() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  });

  menu_bar->AddChild(start_btn);
  menu_bar->AddChild(pause_btn);
  menu_bar->AddChild(stop_btn);
  menu_bar->AddChild(upload_btn);
  menu_bar->AddChild(save_btn);
  menu_bar->AddChild(exit_btn);

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
  canvas->DrawImage("assets/images/map.svg", 0, 0, 800, 600);

  main_layout->AddChild(canvas);

  return 0;
}

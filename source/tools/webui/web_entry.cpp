/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <memory>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
static std::shared_ptr<WebCanvas> canvas;
static std::shared_ptr<WebImage> map_image;
static std::unique_ptr<WebLayout> layout;
static std::shared_ptr<WebTextbox> top_textbox;
static std::shared_ptr<WebTextbox> bottom_textbox;

int main() {
  layout = std::make_unique<WebLayout>(WebOptions{ .id = "app-layout" });
  layout->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");

  auto menu_bar = std::make_shared<WebLayout>(WebOptions{ 
    .id = "menu-bar",
    .style = {{
      {"width", "100%"},
      {"background", "#363636"},
      {"border-radius", "20px"},
    }}
  });
  menu_bar->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px");

  auto btn_lambda = []() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  };

  auto start_btn = std::make_shared<WebButton>("", WebOptions{ .id = "start-btn", .classes={"button"} });
  start_btn->SetOnClick(btn_lambda);

  auto pause_btn = std::make_shared<WebButton>("", WebOptions{ .id = "pause-btn", .classes={"button"} });
  pause_btn->SetOnClick(btn_lambda);

  auto stop_btn = std::make_shared<WebButton>("", WebOptions{ .id = "stop-btn", .classes={"button"} });
  stop_btn->SetOnClick(btn_lambda);

  auto upload_btn = std::make_shared<WebButton>("", WebOptions{ .id = "upload-btn", .classes={"button"} });
  upload_btn->SetOnClick(btn_lambda);

  auto save_btn = std::make_shared<WebButton>("", WebOptions{ .id = "save-btn", .classes={"button"} });
  save_btn->SetOnClick(btn_lambda);

  auto exit_btn = std::make_shared<WebButton>("", WebOptions{ .id = "exit-btn", .classes={"button"} });
  exit_btn->SetOnClick(btn_lambda);

  menu_bar->AddChild(start_btn);
  menu_bar->AddChild(pause_btn);
  menu_bar->AddChild(stop_btn);
  menu_bar->AddChild(upload_btn);
  menu_bar->AddChild(save_btn);
  menu_bar->AddChild(exit_btn);

  layout->AddChild(menu_bar);

  auto main_layout = std::make_shared<WebLayout>(WebOptions{ .id = "main-layout" });
  main_layout->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start");
  layout->AddChild(main_layout);

  auto left_column = std::make_shared<WebLayout>(WebOptions{ 
    .id = "left-column",
    .style = {{
      {"flex", "3"}, 
    }}
  });
  left_column->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch");
  main_layout->AddChild(left_column);

  top_textbox = std::make_shared<WebTextbox>(TextStyle(), WebOptions{
    .id = "top-textbox",
    .classes = { "textbox" },
    .style = {{ "flex", "7" }}
  });
  top_textbox->SetText("Info");
  left_column->AddChild(top_textbox);

  bottom_textbox = std::make_shared<WebTextbox>(TextStyle(), WebOptions{
    .id = "bottom-textbox",
    .classes = { "textbox" },
    .style = {{ "flex", "3" }}
  });
  bottom_textbox->SetText("Log");
  left_column->AddChild(bottom_textbox);

  auto game_area = std::make_shared<WebLayout>(WebOptions{ .id = "game-area" });
  main_layout->AddChild(game_area);

  map_image = std::make_shared<WebImage>("assets/images/map2.svg", "Game map", WebOptions{ .id = "map-image" });
  map_image->SetPosition(0, 0);
  game_area->AddChild(map_image);

  canvas = std::make_shared<WebCanvas>(1000, 500, WebOptions{ .id = "game-canvas" });
  canvas->SetFillColor({255, 255, 255});
  canvas->SetFont("48px arial");
  game_area->AddChild(canvas);

  return 0;
}

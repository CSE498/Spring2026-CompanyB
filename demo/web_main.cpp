/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <memory>
#include <unordered_map>
#include <functional>
#include <print>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
static std::shared_ptr<WebCanvas> canvas;

static std::unordered_map<std::string, std::shared_ptr<WebElement>> elements{};

void set_active_layout(std::shared_ptr<WebElement>&& layout) {
  elements["active_layout"] = layout;
}

std::shared_ptr<WebElement> GameInfoCanvas(WebOptions options) {
  auto gameInfo = UIItem<WebCanvas>(
    500,
    500,
    options);

  gameInfo->SetBackgroundColor({85, 85, 85})
    .SetFillColor({255, 255, 255})
    .DrawRect(10, 10, 480, 480, true)
    .SetPenColor({0, 0, 0})
    .DrawLine({40, 40}, {40, 450})
    .DrawLine({40, 450}, {450, 450});

  // Draw our data points
  for (int i = 0; i < 41; i++) {
    gameInfo->SetFillColor({0, 0, 100})
        .SetFont("8px Arial")
        .DrawLine({(i * 10) + 40, 445}, {(i * 10) + 40, 455})
        .DrawText(std::to_string(i), (i * 10) + 37, 470);
    gameInfo->SetFillColor({0, 0, 100})
        .SetFont("8px Arial")
        .DrawLine({35, 450 - (i * 10)}, {45, 450 - (i * 10)})
        .DrawText(std::to_string(i), 20, 453 - (i * 10));

    gameInfo->SetFillColor({150, 0, 0})
        .DrawLine({(i * 10) + 40, 450}, {(i * 10) + 40, 450 - (i * 10)});
  };

  return gameInfo;
}

std::shared_ptr<WebElement> SimulationLayout(std::function<void()> btn_lambda) {
  auto gameInfo = GameInfoCanvas(WebOptions{
      .id = "game-info",
      .classes = {"textbox"},
      .style = {{"flex", "7"}}
    });

  return UIItem<WebLayout>(WebOptions{
    .id = "app-layout",
    .children = {

      UIItem<WebLayout>(WebOptions{
        .id = "menu-bar",
        .style = {
          {"width", "100%"},
          {"background", "#363636"},
          {"border-radius", "20px"},
        },
        .children = {
          UIItem<WebButton>("", WebOptions{ .id = "start-btn", .classes={"button"} })->SetOnClick(btn_lambda),
          UIItem<WebButton>("", WebOptions{ .id = "pause-btn", .classes={"button"} })->SetOnClick(btn_lambda),
          UIItem<WebButton>("", WebOptions{ .id = "stop-btn", .classes={"button"} })->SetOnClick(btn_lambda),
          UIItem<WebButton>("", WebOptions{ .id = "upload-btn", .classes={"button"} })->SetOnClick(btn_lambda),
          UIItem<WebButton>("", WebOptions{ .id = "save-btn", .classes={"button"} })->SetOnClick(btn_lambda),
          UIItem<WebButton>("", WebOptions{ .id = "exit-btn", .classes={"button"} })->SetOnClick(btn_lambda)
        }
      })->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px"),

      UIItem<WebLayout>(WebOptions{
        .id = "main-layout",
        .children = {
          UIItem<WebLayout>(WebOptions{ 
            .id = "left-column",
            .style = {
              {"flex", "3"}, 
            },

            .children = {
              // Our graph canvas
              gameInfo,

              UIItem<WebTextbox>(TextStyle(), WebOptions{
                .id = "top-textbox",
                .classes = { "textbox" },
                .style = {{ "flex", "3" }}
              })->SetText("Info"),
            }
          })->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch"),

          UIItem<WebLayout>(WebOptions{
            .id = "game-area",
            .children = {
              UIItem<WebImage>("assets/images/map2.svg", "Game map", WebOptions{ .id = "map-image" })->SetPosition(0, 0),
              UIItem<WebCanvas>(1000, 500, WebOptions{ .id = "game-canvas" })->SetFillColor({255, 255, 255}).SetFont("48px arial")
            }
          })
        }
      })
        ->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start")
    }
  })->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");
}

std::shared_ptr<WebElement> MenuLayout(std::function<void()> button_lambda) {
  // This code works
  return UIItem<WebLayout>(WebOptions{
    .id = "menu-screen",
    .style = {
      {"width", "100vw"},
      {"height", "100vh"},
    },
    .children = {
      UIItem<WebButton>("Start Simulation")->SetOnClick(button_lambda),
    }
  })->SetJustifyContent("center").SetAlignItems("center");
}

void load_menu_layout();
void load_simulation_layout();

void load_menu_layout() {
  std::println("Loading menu layout");
  set_active_layout(MenuLayout(load_simulation_layout));
}

void load_simulation_layout() {
  std::println("Loading simulation layout");
  set_active_layout(SimulationLayout(load_menu_layout));
};

int main() {
  load_menu_layout();

  return 0;
}

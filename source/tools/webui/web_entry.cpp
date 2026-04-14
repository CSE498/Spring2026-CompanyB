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

std::shared_ptr<WebElement> simulation_layout(std::function<void()> btn_lambda) {
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
              UIItem<WebTextbox>(TextStyle(), WebOptions{
                .id = "top-textbox",
                .classes = { "textbox" },
                .style = {{ "flex", "7" }}
              })->SetText("Info"),

              UIItem<WebTextbox>(TextStyle(), WebOptions{
                .id = "bottom-textbox",
                .classes = { "textbox" },
                .style = {{ "flex", "3" }}
              })->SetText("Log")
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

std::shared_ptr<WebElement> menu_layout(std::function<void()> button_lambda) {
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
  set_active_layout(menu_layout(load_simulation_layout));
}

void load_simulation_layout() {
  std::println("Loading simulation layout");
  set_active_layout(simulation_layout(load_menu_layout));
};

int main() {
  load_menu_layout();

  return 0;
}

/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <cmath>
#include <memory>
#include <vector>
#include "InfoGraph.hpp"
#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"
using namespace cse498;

static std::unordered_map<std::string, std::shared_ptr<WebElement>> elements{};
static std::shared_ptr<InfoGraph> live_graph = nullptr;

static double t = 0.0;
static std::vector<double> bar_data{8, 4, 10, 6, 12};

void set_active_layout(std::shared_ptr<WebElement>&& layout) {
  elements["active_layout"] = layout;
}

void UpdateGraphs() {
  static int count = 0;
  if (live_graph) {
    live_graph->AddDataPoint(count, "Counting Test");
    count++;
  }
}

std::shared_ptr<WebElement> GameInfoCanvas(WebOptions options) {
  auto gameInfo = UIItem<InfoGraph>(500, 500, options);
  live_graph = gameInfo;
  live_graph->SetAutoScroll(true);
  return gameInfo;
}

std::shared_ptr<WebElement> SimulationLayout(std::function<void()> btn_lambda,
                                             std::function<void()> exit_lambda) {
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
          UIItem<WebButton>("", WebOptions{ .id = "exit-btn", .classes={"button"} })->SetOnClick(exit_lambda)
        }
      })->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px"),

      UIItem<WebLayout>(WebOptions{
        .id = "main-layout",
        .children = {
          UIItem<WebLayout>(WebOptions{
            .id = "left-column",
            .style = {{"flex", "3"}},
            .children = {
              gameInfo,
              UIItem<WebLayout>(WebOptions{
                .id = "scroll-controls",
                .style = {{"flex", "0"}},
                .children = {
                  UIItem<WebButton>("<", WebOptions{ .id = "scroll-left-btn", .classes={"button"} })
                    ->SetOnClick([](){ 
                      if (live_graph) {
                        live_graph->SetAutoScroll(false);
                        live_graph->ScrollLeft();
                      }
                    }),
                  UIItem<WebButton>(">", WebOptions{ .id = "scroll-right-btn", .classes={"button"} })
                    ->SetOnClick([](){ 
                      if (live_graph) live_graph->ScrollRight();
                    }),
                }
              })->SetDirection("row").SetGap("10px").SetJustifyContent("center"),
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
      })->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start")
    }
  })->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");
}

std::shared_ptr<WebElement> MenuLayout(std::function<void()> button_lambda) {
  return UIItem<WebLayout>(WebOptions{
    .id = "menu-screen",
    .style = {
      {"width", "100vw"},
      {"height", "100vh"},
      {"position", "relative"},
    },
    .children = {
      UIItem<WebImage>("assets/images/TopBorder.png", "top decoration", WebOptions{ .id = "menu-img-top" }),
      UIItem<WebTextbox>(TextStyle{ .font_family = "Gugi, sans-serif", .font_size = "75px", .color = "white" }, WebOptions{
        .id = "menu-title",
        .style = {{"border", "none"}, {"background", "transparent"}, {"white-space", "nowrap"}},
      })->SetText("Simulation Tool"),
      UIItem<WebLayout>(WebOptions{
        .id = "world-buttons",
        .style = {{"font-family", "Gugi, sans-serif"}},
        .children = {
          UIItem<WebButton>("Traffic", WebOptions{ .id = "traffic-btn" })->SetOnClick(button_lambda),
          UIItem<WebButton>("Virus", WebOptions{ .id = "virus-btn" })->SetOnClick(button_lambda),
        }
      })->SetDirection("row").SetJustifyContent("center").SetAlignItems("center").SetGap("50px"),
      UIItem<WebImage>("assets/images/BottomBorder.png", "bottom decoration", WebOptions{ .id = "menu-img-bottom" }),
    }
  })->SetDirection("column").SetJustifyContent("flex-start").SetAlignItems("center").SetGap("100px");
}

void load_menu_layout();
void load_simulation_layout();

void load_menu_layout() {
  std::println("Loading menu layout");
  live_graph = nullptr;
  set_active_layout(MenuLayout(load_simulation_layout));
}

void load_simulation_layout() {
  std::println("Loading simulation layout");
  set_active_layout(SimulationLayout(load_simulation_layout, load_menu_layout));
}

int main() {
  load_menu_layout();
  emscripten_set_main_loop(UpdateGraphs, 2, true);
  return 0;
}
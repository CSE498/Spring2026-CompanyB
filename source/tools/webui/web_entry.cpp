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
static std::unique_ptr<WebLayout> layout;

int main() {
  auto btn_lambda = []() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  };

  layout = std::make_unique<WebLayout>(WebOptions{
    .id = "app-layout",
    .children = {

      UIItem<WebLayout>(WebOptions{ 
        .id = "menu-bar",
        .style = {{
          {"width", "100%"},
          {"background", "#363636"},
          {"border-radius", "20px"},
        }},
        .children = {
          UIItem<WebButton>("", WebOptions{ .id = "start-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr(),
          UIItem<WebButton>("", WebOptions{ .id = "pause-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr(),
          UIItem<WebButton>("", WebOptions{ .id = "stop-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr(),
          UIItem<WebButton>("", WebOptions{ .id = "upload-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr(),
          UIItem<WebButton>("", WebOptions{ .id = "save-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr(),
          UIItem<WebButton>("", WebOptions{ .id = "exit-btn", .classes={"button"} })->SetOnClick(btn_lambda).GetSharedPtr()
        }
      })->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px").GetSharedPtr(),

      UIItem<WebLayout>(WebOptions{
        .id = "main-layout",
        .children = {
          UIItem<WebLayout>(WebOptions{ 
            .id = "left-column",
            .style = {{
              {"flex", "3"}, 
            }},

            .children = {
              UIItem<WebTextbox>(TextStyle(), WebOptions{
                .id = "top-textbox",
                .classes = { "textbox" },
                .style = {{ "flex", "7" }}
              })->SetText("Info").GetSharedPtr(),

              UIItem<WebTextbox>(TextStyle(), WebOptions{
                .id = "bottom-textbox",
                .classes = { "textbox" },
                .style = {{ "flex", "3" }}
              })->SetText("Log").GetSharedPtr()
            }
          })->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch").GetSharedPtr(),

          UIItem<WebLayout>(WebOptions{
            .id = "game-area",
            .children = {
              UIItem<WebImage>("assets/images/map2.svg", "Game map", WebOptions{ .id = "map-image" })->SetPosition(0, 0).GetSharedPtr(),
              UIItem<WebCanvas>(1000, 500, WebOptions{ .id = "game-canvas" })->SetFillColor({255, 255, 255}).SetFont("48px arial").GetSharedPtr()
            }
          })
        }
      })
        ->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start").GetSharedPtr()
    }
  });
  
  layout->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");

  return 0;
}

/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <memory>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"

#include "Agents/PacingAgent.hpp"
#include "Worlds/InfectiousWorld.hpp"

namespace ansi {
constexpr const char* CLEAR_SCREEN = "\033[2J\033[H";
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* RED = "\033[31m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* GREY = "\033[90m";
}  // namespace ansi

using namespace cse498;

// Globals kept alive for the duration of the page
static std::shared_ptr<WebCanvas> canvas;
static std::unique_ptr<WebLayout> layout;

std::shared_ptr<WebCanvas> GameInfo;
std::shared_ptr<WebCanvas> GameCanvas;

static std::unique_ptr<InfectiousWorld> world;

// rebuild display and draw each frame.
void drawSim() {
    GameCanvas->Clear();
  const WorldGrid& grid = world->GetGrid();

  // Build a character grid from the world tiles.
  std::vector<std::string> display(grid.GetHeight());
  for (size_t y = 0; y < grid.GetHeight(); ++y) {
    display[y].resize(grid.GetWidth());
    for (size_t x = 0; x < grid.GetWidth(); ++x) {
      display[y][x] = grid.GetSymbol(WorldPosition{x, y});
    }
  }

  // Stamp agents onto the grid.
  for (size_t id = 0; id < world->GetNumAgents(); ++id) {
    const AgentBase& agent = world->GetAgent(id);
    WorldPosition pos = agent.GetLocation().AsWorldPosition();
    char sym;
    switch (world->GetAgentHealth(id)) {
      case InfectiousWorld::HealthState::SUSCEPTIBLE:
        sym = 'S';
        break;
      case InfectiousWorld::HealthState::INFECTED:
        sym = 'I';
        break;
      case InfectiousWorld::HealthState::RECOVERED:
        sym = 'R';
        break;
    }
    display[pos.CellY()][pos.CellX()] = sym;
  }

  // Scale grid to fill the entire canvas
  double cell_w = 1000.0 / grid.GetWidth();
  double cell_h = 500.0 / grid.GetHeight();
  double radius = std::min(cell_w, cell_h) * 0.35;

  // Draw the grid to the canvas
  for (size_t i = 0; i < grid.GetHeight(); i++) {
    for (size_t j = 0; j < grid.GetWidth(); j++) {
      double cx = j * cell_w + cell_w / 2.0;
      double cy = i * cell_h + cell_h / 2.0;
      char c = display.at(i).at(j);
      switch (c) {
        case 'S':
          GameCanvas->SetFillColor({0, 255, 0}).DrawCircle(cx, cy, radius, true);
          break;
        case 'I':
          GameCanvas->SetFillColor({255, 0, 0}).DrawCircle(cx, cy, radius, true);
          break;
        case 'R':
          GameCanvas->SetFillColor({0, 0, 255}).DrawCircle(cx, cy, radius, true);
          break;
        case '#':
          GameCanvas->SetFillColor({0, 0, 0}).DrawRect(cx - cell_w / 2.0, cy - cell_h / 2.0, cell_w, cell_h, true);
          break;
        default:
          break;
      }
    }
  }

  // Step the simulation forward.
  world->RunAgents();
  world->UpdateWorld();
}

/// Set up the world, configure disease, add agents.
void SetupWorld() {
  world = std::make_unique<InfectiousWorld>(20, 10);

  WorldGrid& grid = world->GetGrid();
  // Add some walls
  for (size_t y = 0; y < 10; ++y) {
    grid[0, y] = world->GetWallID();
    grid[19, y] = world->GetWallID();
  }
  for (size_t x = 0; x < 20; ++x) {
    grid[x, 0] = world->GetWallID();
    grid[x, 9] = world->GetWallID();
  }
  // Internal wall
  for (size_t y = 1; y < 6; ++y) {
    grid[10, y] = world->GetWallID();
  }

  // Configure disease.
  world->SetTransmissionRate(0.4);
  world->SetInfectionRadius(1.5);
  world->SetInfectionDuration(8);
  world->SetImmunityDuration(15);

  // Add agents scattered across the grid.
  world->AddAgent<PacingAgent>("Agent-1").SetLocation(WorldPosition{3, 2});
  world->AddAgent<PacingAgent>("Agent-2").SetLocation(WorldPosition{5, 4});
  world->AddAgent<PacingAgent>("Agent-3").SetLocation(WorldPosition{7, 3});
  world->AddAgent<PacingAgent>("Agent-4").SetLocation(WorldPosition{4, 6});
  world->AddAgent<PacingAgent>("Agent-5").SetLocation(WorldPosition{8, 7});
  world->AddAgent<PacingAgent>("Agent-6").SetLocation(WorldPosition{12, 3});
  world->AddAgent<PacingAgent>("Agent-7").SetLocation(WorldPosition{15, 5});
  world->AddAgent<PacingAgent>("Agent-8").SetLocation(WorldPosition{14, 2});
  world->AddAgent<PacingAgent>("Agent-9").SetLocation(WorldPosition{16, 7});
  world->AddAgent<PacingAgent>("Agent-10").SetLocation(WorldPosition{13, 7});

  // Patient zero!
  world->InfectAgent(0);
}

int main() {
  SetupWorld();

  auto btn_lambda = []() {
    canvas->Clear();
    canvas->SetFillColor({0, 200, 100});
    canvas->DrawText("Running...", 10, 30);
  };

  layout = std::make_unique<WebLayout>(WebOptions{
      .id = "app-layout",
      .children = {

          UIItem<WebLayout>(
              WebOptions{
                  .id = "menu-bar",
                  .style = {{
                      {"width", "100%"},
                      {"background", "#363636"},
                      {"border-radius", "20px"},
                  }},
                  .children =
                      {UIItem<WebButton>("", WebOptions{.id = "start-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda),
                       UIItem<WebButton>("", WebOptions{.id = "pause-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda),
                       UIItem<WebButton>("", WebOptions{.id = "stop-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda),
                       UIItem<WebButton>("", WebOptions{.id = "upload-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda),
                       UIItem<WebButton>("", WebOptions{.id = "save-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda),
                       UIItem<WebButton>("", WebOptions{.id = "exit-btn",
                                                        .classes = {"button"}})
                           ->SetOnClick(btn_lambda)}})
              ->SetHeight("80px")
              .SetDirection("row")
              .SetAlignItems("center")
              .SetGap("10px"),

          UIItem<WebLayout>(
              WebOptions{
                  .id = "main-layout",
                  .children =
                      {UIItem<WebLayout>(
                           WebOptions{
                               .id = "left-column",
                               .style = {{
                                   {"flex", "3"},
                               }},

                               .children =
                                   {(GameInfo = UIItem<WebCanvas>(
                                         500,
                                         500,
                                         WebOptions{
                                             .id = "game-info",
                                             .classes =
                                                 {"textbox"},
                                             .style =
                                                 {{"flex", "7"}}}))
                                        ->SetBackgroundColor({85, 85, 85})
                                        .SetFillColor({255, 255, 255})
                                        .DrawRect(10, 10, 480, 480, true)
                                        .SetPenColor({0, 0, 0})
                                        .DrawLine({40, 40}, {40, 450})
                                        .DrawLine({40, 450}, {450, 450}),
                                    UIItem<WebTextbox>(
                                        TextStyle(),
                                        WebOptions{
                                            .id = "bottom-textbox",
                                            .classes =
                                                {"textbox"},
                                            .style =
                                                {{"flex", "3"}}})
                                        ->SetText("Log")}})
                           ->SetHeight("100%")
                           .SetDirection("column")
                           .SetGap("10px")
                           .SetAlignItems("stretch"),

                       UIItem<WebLayout>(WebOptions{
                           .id = "game-area",
                           .children =
                               {UIItem<WebImage>(
                                    "assets/images/map2.svg",
                                    "Game map",
                                    WebOptions{.id = "map-image"})
                                    ->SetPosition(0, 0),
                                (GameCanvas = UIItem<WebCanvas>(
                                    1000,
                                    500, WebOptions{.id = "game-canvas"}))
                                    ->SetFillColor({255, 255, 255})
                                    .SetFont("48px arial")}})}})
              ->SetHeight("80vh")
              .SetDirection("row")
              .SetGap("20px")
              .SetAlignItems("stretch")
              .SetJustifyContent("flex-start")}});

  layout->SetDirection("column")
      .SetAlignItems("center")
      .SetGap("10px")
      .SetHeight("100vh")
      .SetJustifyContent("flex-start");

  for (int i = 0; i < 41; i++) {
    GameInfo->SetFillColor({0, 0, 100})
        .SetFont("8px Arial")
        .DrawLine({(i * 10) + 40, 445}, {(i * 10) + 40, 455})
        .DrawText(std::to_string(i), (i * 10) + 37, 470);
    GameInfo->SetFillColor({0, 0, 100})
        .SetFont("8px Arial")
        .DrawLine({35, 450 - (i * 10)}, {45, 450 - (i * 10)})
        .DrawText(std::to_string(i), 20, 453 - (i * 10));

    GameInfo->SetFillColor({150, 0, 0})
        .DrawLine({(i * 10) + 40, 450}, {(i * 10) + 40, 450 - (i * 10)});
  }

  emscripten_set_main_loop(drawSim, 5, true);
  return 0;
}

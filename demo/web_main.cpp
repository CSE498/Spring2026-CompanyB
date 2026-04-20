/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>

#include <functional>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"
#include "Interfaces/AutoInterface.hpp"
#include "Worlds/TrafficWorld.hpp"
#include "Worlds/InfectiousWorld.hpp"
#include "core/ItemBase.hpp"
#include "Agents/DrivingAgent.hpp"
#include "Agents/PacingAgent.hpp"
#include "InfoGraph.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
enum class ActiveSim { NONE, TRAFFIC, VIRUS };
enum class SimState  { STOPPED, PLAYING, PAUSED };

static ActiveSim active_sim = ActiveSim::NONE;
static SimState  sim_state  = SimState::STOPPED;
static std::shared_ptr<WebCanvas> GameCanvas;
static std::unique_ptr<TrafficWorld> traffic_world;
static std::unique_ptr<InfectiousWorld> virus_world;

// static std::shared_ptr<InfoGraph> bar_graph = std::make_shared<InfoGraph>(700, 280, "bar-graph");
static std::shared_ptr<InfoGraph> line_graph = nullptr;
static std::unordered_map<std::string, std::shared_ptr<WebElement>> elements{};

static double t = 0.0;
static int count = 0;
// static std::vector<double> bar_data{8, 4, 10, 6, 12};

void UpdateGraphs() {
  if (line_graph) {
    line_graph->AddDataPoint(count, "Counting Test");
    count++;
  }
}

std::shared_ptr<WebElement> GameInfoCanvas(WebOptions options) {
  auto gameInfo = UIItem<InfoGraph>(500, 500, options);
  line_graph = gameInfo;
  line_graph->SetAutoScroll(true);
  return gameInfo;
}

void SetupVirusWorld();

auto handle_sim_state = [](SimState next) {
    switch (next) {
        case SimState::PLAYING:
            sim_state = SimState::PLAYING;
            break;
        case SimState::PAUSED:
            if (sim_state == SimState::PLAYING) sim_state = SimState::PAUSED;
            break;
        case SimState::STOPPED:
            sim_state = SimState::STOPPED;
            // Reset the active world to its initial state
            if (active_sim == ActiveSim::TRAFFIC) {
                traffic_world = std::make_unique<TrafficWorld>("assets/grids/DemoWorld.grid");
            } else if (active_sim == ActiveSim::VIRUS) {
                SetupVirusWorld();
            }
            count = 0;
            line_graph->ClearData();
            break;
    }
};

void DrawTrafficSim() {
    GameCanvas->Clear();
    const WorldGrid& grid = traffic_world->GetGrid();

    const size_t W = grid.GetWidth();
    const size_t H = grid.GetHeight();

    const double cell_w = static_cast<double>(GameCanvas->GetWidth())  / W;
    const double cell_h = static_cast<double>(GameCanvas->GetHeight()) / H;

    for (size_t y = 0; y < H; ++y) {
      for (size_t x = 0; x < W; ++x) {
          double cx = x * cell_w;
          double cy = y * cell_h;
          char sym = grid.GetSymbol(WorldPosition{x, y});
          switch(sym) {
              case '.':
                  GameCanvas->SetFillColor({40, 40, 40}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              case '|':
                  GameCanvas->SetFillColor({0, 100, 100}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              case '-':
                  GameCanvas->SetFillColor({100, 0, 100}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              case 'S':
                  GameCanvas->SetFillColor({100, 100, 100}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              case 'D':
                  GameCanvas->SetFillColor({200, 200, 200}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              default:
                  break;
          }
      }
    }

    const double radius = std::max(std::min(cell_w, cell_h) * 0.4, 8.0);
    for (size_t id = 0; id < traffic_world->GetNumAgents(); ++id) {
        const AgentBase& agent = traffic_world->GetAgent(id);
        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        double cx = pos.CellX() * cell_w + cell_w / 2.0;
        double cy = pos.CellY() * cell_h + cell_h / 2.0;
        GameCanvas->SetFillColor({255, 80, 80}).DrawCircle(cx, cy, radius, true);
    }

    if (sim_state == SimState::PLAYING) {
        traffic_world->RunAgents();
        traffic_world->UpdateWorld();
        UpdateGraphs();
    }
}

void DrawVirusSim() {
    GameCanvas->Clear();
    const WorldGrid& grid = virus_world->GetGrid();

    const size_t W = grid.GetWidth();
    const size_t H = grid.GetHeight();

    std::vector<std::string> display(H);
    for (size_t y = 0; y < H; ++y) {
        display[y].resize(W);
        for (size_t x = 0; x < W; ++x) {
            display[y][x] = grid.GetSymbol(WorldPosition{x, y});
        }
    }

    for (size_t id = 0; id < virus_world->GetNumAgents(); ++id) {
        const AgentBase& agent = virus_world->GetAgent(id);
        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        char sym;
        switch (virus_world->GetAgentHealth(id)) {
            case InfectiousWorld::HealthState::SUSCEPTIBLE: sym = 'S'; break;
            case InfectiousWorld::HealthState::INFECTED:    sym = 'I'; break;
            case InfectiousWorld::HealthState::RECOVERED:   sym = 'R'; break;
        }
        display[pos.CellY()][pos.CellX()] = sym;
    }

    const double cell_w = static_cast<double>(GameCanvas->GetWidth()) / W;
    const double cell_h = static_cast<double>(GameCanvas->GetHeight()) / H;
    const double radius = std::min(cell_w, cell_h) * 0.35;

    for (size_t y = 0; y < H; ++y) {
        for (size_t x = 0; x < W; ++x) {
            double cx = x * cell_w + cell_w / 2.0;
            double cy = y * cell_h + cell_h / 2.0;
            char c = display[y][x];
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

    if (sim_state == SimState::PLAYING) {
        virus_world->RunAgents();
        virus_world->UpdateWorld();
        UpdateGraphs();
    }
}

void SetupVirusWorld() {
    virus_world = std::make_unique<InfectiousWorld>(20, 10);
    WorldGrid& grid = virus_world->GetGrid();

    for (size_t y = 0; y < 10; ++y) {
        grid[0, y] = virus_world->GetWallID();
        grid[19, y] = virus_world->GetWallID();
    }
    for (size_t x = 0; x < 20; ++x) {
        grid[x, 0] = virus_world->GetWallID();
        grid[x, 9] = virus_world->GetWallID();
    }
    for (size_t y = 1; y < 6; ++y) {
        grid[10, y] = virus_world->GetWallID();
    }

    virus_world->SetTransmissionRate(0.4);
    virus_world->SetInfectionRadius(1.5);
    virus_world->SetInfectionDuration(8);
    virus_world->SetImmunityDuration(15);

    virus_world->AddAgent<PacingAgent>("Agent-1").SetLocation(WorldPosition{3, 2});
    virus_world->AddAgent<PacingAgent>("Agent-2").SetLocation(WorldPosition{5, 4});
    virus_world->AddAgent<PacingAgent>("Agent-3").SetLocation(WorldPosition{7, 3});
    virus_world->AddAgent<PacingAgent>("Agent-4").SetLocation(WorldPosition{4, 6});
    virus_world->AddAgent<PacingAgent>("Agent-5").SetLocation(WorldPosition{8, 7});
    virus_world->AddAgent<PacingAgent>("Agent-6").SetLocation(WorldPosition{12, 3});
    virus_world->AddAgent<PacingAgent>("Agent-7").SetLocation(WorldPosition{15, 5});
    virus_world->AddAgent<PacingAgent>("Agent-8").SetLocation(WorldPosition{14, 2});
    virus_world->AddAgent<PacingAgent>("Agent-9").SetLocation(WorldPosition{16, 7});
    virus_world->AddAgent<PacingAgent>("Agent-10").SetLocation(WorldPosition{13, 7});

    virus_world->InfectAgent(0);
}

// We switch screens by holding a shared pointer to the active screen's
// WebElement in a static map
// Note: This requires that there not be other shared pointers to the WebElement around,
// Or the screen won't be removed as it won't be destructed when we replace it!
void set_active_layout(std::shared_ptr<WebElement>&& layout) {
    elements["active_layout"] = layout;
}

// This is a web component that returns a canvas
// Here, we choose to take our options for the canvas as a parameter
// So the function can be treated as a component with the caller specifying
// attributes like ID, styles, CSS classes etc.
// std::shared_ptr<WebElement> GameInfoCanvas(WebOptions options) {
//     // UIItem is just syntax sugar for make_shared that creates a shared pointer
//     // auto gameInfo = UIItem<WebCanvas>(500, 500, options);


//     return gameInfo;
// }

// Here we create a function that creates the layout for the simulation screen
// It takes a lambda as a parameter
// This is a function we want to run when a button is clicked
// We could take more lambdas if we want more types of button handlers
// btn_lambda: generic handler for start/pause/stop/upload/save which can be updated later
// exit_lambda: always goes back to the main menu for reselection of world or exit
std::shared_ptr<WebElement> SimulationLayout(ActiveSim sim, std::function<void()> btn_lambda, std::function<void()> exit_lambda) {
    // Here we initialize our game info canvas with id, classes, and style properties.
    auto gameInfo = GameInfoCanvas(WebOptions{
        .id = "game-info",
        .classes = {"textbox"},
        .style = {{"flex", "7"}}
    });

    std::vector<std::shared_ptr<WebElement>> game_children;

    if (sim == ActiveSim::TRAFFIC) {
        game_children.push_back(
            UIItem<WebImage>("assets/images/full_map.svg", "Game map", WebOptions{
                .id = "map-image",
            })
        );
        game_children.push_back(
            (GameCanvas = UIItem<WebCanvas>(1000, 1000, WebOptions{
                .id = "game-canvas",
            }))->SetFont("48px arial")
        );
    } else {
        game_children.push_back(
            (GameCanvas = UIItem<WebCanvas>(1000, 500, WebOptions{
                .id = "game-canvas",
            }))->SetFillColor({255, 255, 255}).SetFont("48px arial")
        );
    }

    auto game_area = UIItem<WebLayout>(WebOptions{
        .id = "game-area",
        .children = game_children,
    });

    return UIItem<WebLayout>(WebOptions{
        .id = "app-layout",
        .children = {
            // We can use the .children property to declare elements inside WebLayouts
            // Which can include other WebLayouts - allowing us to create arbitrary layouts
            UIItem<WebLayout>(WebOptions{
                .id = "menu-bar",
                .style = {
                    {"width", "100%"},
                    {"background", "#363636"},
                    {"border-radius", "20px"},
                },
                .children = {
                    UIItem<WebButton>("", WebOptions{ .id = "start-btn", .classes={"button"} })->SetOnClick([]{ handle_sim_state(SimState::PLAYING); }),
                    UIItem<WebButton>("", WebOptions{ .id = "pause-btn", .classes={"button"} })->SetOnClick([]{ handle_sim_state(SimState::PAUSED); }),
                    UIItem<WebButton>("", WebOptions{ .id = "stop-btn",  .classes={"button"} })->SetOnClick([]{ handle_sim_state(SimState::STOPPED); }),
                    // Using SetOnFileUpload with a custom lambda
                    UIItem<WebButton>("", WebOptions{ .id = "upload-btn", .classes={"button"} })->SetOnFileUpload([](const std::string& file_content) {
                        std::println("SUCCESS! File uploaded to C++ backend. Length: {} characters.", file_content.length());
                        // TODO: When the World object is available, do: world_ptr->LoadScript(file_content);
                    }),
                    UIItem<WebButton>("", WebOptions{ .id = "save-btn", .classes={"button"} })->SetOnClick(btn_lambda),
                    UIItem<WebButton>("", WebOptions{ .id = "exit-btn", .classes={"button"} })->SetOnClick(exit_lambda)
                }
            })->SetHeight("80px").SetDirection("row").SetAlignItems("center").SetGap("10px"),

            UIItem<WebLayout>(WebOptions{
                .id = "main-layout",
                .children = {
                    UIItem<WebLayout>(WebOptions{
                        .id = "left-column",
                        .style = {
                            {"flex", "1"},
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
                        // Notice that the fluent API style also lets us call WebLayout's methods
                        // while including it as a parameter to the function call
                        // This is possible because they return a reference to the WebElement,
                        // Which in turn is implicitly casted to the shared pointer with an operator overload
                    })->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch"),
                    game_area
                }
            })
                ->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("flex-start")
        }
    })->SetDirection("column").SetAlignItems("center").SetGap("10px").SetHeight("100vh").SetJustifyContent("flex-start");
}

void load_menu_layout();
void load_traffic_layout();
void load_virus_layout();

// We create another component for our menu screen layout
// button_lambda is called when either world is selected.
// When the seperate worlds are ready we can replace the lambdas.
std::shared_ptr<WebElement> MenuLayout() {
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
                    UIItem<WebButton>("Traffic", WebOptions{ .id = "traffic-btn" })->SetOnClick(load_traffic_layout),
                    UIItem<WebButton>("Virus", WebOptions{ .id = "virus-btn" })->SetOnClick(load_virus_layout),
                }
            })->SetDirection("row").SetJustifyContent("center").SetAlignItems("center").SetGap("50px"),
            UIItem<WebImage>("assets/images/BottomBorder.png", "bottom decoration", WebOptions{ .id = "menu-img-bottom" }),
        }
    })->SetDirection("column").SetJustifyContent("flex-start").SetAlignItems("center").SetGap("100px");
}

// These are our two button click handlers
void load_menu_layout() {
    std::println("Loading menu layout");
    active_sim = ActiveSim::NONE;
    GameCanvas.reset();
    set_active_layout(MenuLayout());
}

void load_traffic_layout() {
    std::println("Loading traffic simulation");
    active_sim = ActiveSim::TRAFFIC;
    sim_state  = SimState::STOPPED;
    traffic_world = std::make_unique<TrafficWorld>("assets/grids/DemoWorld.grid");
    set_active_layout(SimulationLayout(ActiveSim::TRAFFIC, load_traffic_layout, load_menu_layout));
}

void load_virus_layout() {
    std::println("Loading virus simulation");
    active_sim = ActiveSim::VIRUS;
    sim_state  = SimState::STOPPED;
    SetupVirusWorld();
    set_active_layout(SimulationLayout(ActiveSim::VIRUS, load_virus_layout, load_menu_layout));
}

void MainLoop() {
    if (!GameCanvas) return;
    switch (active_sim) {
        case ActiveSim::TRAFFIC: DrawTrafficSim(); break;
        case ActiveSim::VIRUS:   DrawVirusSim();   break;
        case ActiveSim::NONE:    break;
    }
}

int main() {
    load_menu_layout();
    emscripten_set_main_loop(MainLoop, 0, 1);
    return 0;
}

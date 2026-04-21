/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>
#include <emscripten/val.h>

#include <charconv>
#include <functional>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"
#include "Worlds/StepTrafficWorld.hpp"
#include "Worlds/InfectiousWorld.hpp"
#include "core/AgentData.hpp"
#include "core/ItemBase.hpp"
#include "Agents/SwarmingAgent.hpp"
#include "InfoGraph.hpp"
#include "WebTextboxOutputManager.hpp"

using namespace cse498;

// Subclass adapter that exposes StepTrafficWorld's protected members so
// the web canvas drawer can read grid and per agent state each frame
class WebTrafficWorld : public StepTrafficWorld<SwarmingAgent<TrafficData>> {
 public:
  using Base = StepTrafficWorld<SwarmingAgent<TrafficData>>;
  using Base::Base;  // inherit constructors

  [[nodiscard]] const WorldGrid& GetGrid() const { return main_grid; }
  [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }
  [[nodiscard]] TrafficData GetAgentState(size_t id) const {
    return agent_set[id]->GetState();
  }
};

// Globals kept alive for the duration of the page
enum class ActiveSim { NONE, TRAFFIC, VIRUS };
enum class SimState  { STOPPED, PLAYING, PAUSED };

static ActiveSim active_sim = ActiveSim::NONE;
static SimState  sim_state  = SimState::STOPPED;
static std::shared_ptr<WebCanvas> GameCanvas;
static std::unique_ptr<WebTrafficWorld> traffic_world;
static std::unique_ptr<InfectiousWorld> virus_world;
static std::shared_ptr<WebTextbox> log_textbox;
static std::unique_ptr<WebTextboxOutputManager> logger;

// Per-sim event tracking so we can emit logs when things change
static size_t last_traffic_agent_count = 0;
static std::vector<bool> traffic_reached_log;
static std::vector<int> last_virus_health;
static size_t sim_tick = 0;  // incremented each advance step

// Tick-rate control
static constexpr int tickRateMin = 1;
static constexpr int tickRateMax = 1000;
static int sim_tps = 20;             // target simulation ticks per second
static int sim_tps_last_valid = 20;  // revert target on bad input
static double last_step_ms = 0.0;    // emscripten_get_now() at last sim step

// How many simulation steps are due this frame based on elapsed wall-clock
// time and the target tps. Supports tps > framerate by running multiple steps
// per MainLoop call. Capped to avoid CPU overconsumption and browser lag.
static int StepsThisFrame() {
    int tps = sim_tps < 1 ? 1 : sim_tps;
    double interval = 1000.0 / static_cast<double>(tps);
    double now = emscripten_get_now();
    if (last_step_ms == 0.0) last_step_ms = now;
    double elapsed = now - last_step_ms;
    int steps = static_cast<int>(elapsed / interval);
    constexpr int maxStepsPerFrame = 20;
    if (steps < 0) steps = 0;
    if (steps > maxStepsPerFrame) {
        steps = maxStepsPerFrame;
        last_step_ms = now;  // resync to wall clock after a long pause
    } else if (steps > 0) {
        last_step_ms += steps * interval;
    }
    return steps;
}

// Parse a non-throwing int from a string. Returns true on full successful parse.
static bool TryParseInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    const char* begin = s.data();
    const char* end = begin + s.size();
    auto res = std::from_chars(begin, end, out);
    return res.ec == std::errc{} && res.ptr == end;
}

// Read the current user-set tps from the DOM input, if present.
// Only updates live when the value is within range; out-of-range values are
// handled on submission.
static void PollTickRateInput() {
    emscripten::val input = emscripten::val::global("document")
        .call<emscripten::val>("getElementById", std::string("tick-rate-input"));
    if (input.isNull() || input.isUndefined()) return;
    std::string i = input["value"].as<std::string>();
    int v = 0;
    if (TryParseInt(i, v) && v >= tickRateMin && v <= tickRateMax) {
        sim_tps = v;
        sim_tps_last_valid = v;
    }
}

void LogSim(const std::string& entity, const std::string& msg, const std::string& tag);

// Called by JS when the tick-rate input submits. Validates and
// reverts to the last valid value if out of range.
extern "C" EMSCRIPTEN_KEEPALIVE void validate_tick_rate() {
    emscripten::val input = emscripten::val::global("document")
        .call<emscripten::val>("getElementById", std::string("tick-rate-input"));
    if (input.isNull() || input.isUndefined()) return;
    std::string i = input["value"].as<std::string>();
    int v = 0;
    bool valid = TryParseInt(i, v);
    if (!valid || v < tickRateMin || v > tickRateMax) {
        LogSim("Input",
               "Tick rate must be between " + std::to_string(tickRateMin) +
               " and " + std::to_string(tickRateMax) +
               ". Reset to " + std::to_string(sim_tps_last_valid) + ".",
               "error");
        input.set("value", std::to_string(sim_tps_last_valid));
        sim_tps = sim_tps_last_valid;
    }
}

static size_t GetActiveTick() { return sim_tick; }

void LogSim(const std::string& entity, const std::string& msg, const std::string& tag = "info") {
    if (!logger) return;
    std::string line = "Tick " + std::to_string(GetActiveTick()) + ": [" + entity + "] " + msg;
    logger->LogTagged(tag, line);
}

void LogInfo(const std::string& entity, const std::string& msg)  { LogSim(entity, msg, "info"); }
void LogError(const std::string& entity, const std::string& msg) { LogSim(entity, msg, "error"); }

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
            LogSim("World", "Simulation started", "start");
            break;
        case SimState::PAUSED:
            if (sim_state == SimState::PLAYING) {
                sim_state = SimState::PAUSED;
                LogSim("World", "Simulation paused", "pause");
            }
            break;
        case SimState::STOPPED:
            sim_state = SimState::STOPPED;
            sim_tick  = 0;
            // Reset the active world to its initial state
            if (active_sim == ActiveSim::TRAFFIC) {
                traffic_world = std::make_unique<WebTrafficWorld>("assets/grids/TrafficWorld_Web.grid");
                last_traffic_agent_count = 0;
                traffic_reached_log.clear();
            } else if (active_sim == ActiveSim::VIRUS) {
                SetupVirusWorld();
                last_virus_health.clear();
            }
            count = 0;
            line_graph->ClearData();
            line_graph->DrawLineChart(std::vector<double> {}, "Cleared");
            LogSim("World", "Simulation stopped and reset", "stop");
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
          int line_space = 10;
          char sym = grid.GetSymbol(WorldPosition{x, y});
          switch(sym) {
              case '.':
                  GameCanvas->SetFillColor({40, 40, 40}).DrawRect(cx, cy, cell_w, cell_h, true);
                  break;
              case '|':
                  GameCanvas->SetPenColor({0, 0, 0}).DrawLine({cx - line_space, cy - line_space}, {cx - line_space, cy + line_space}).DrawLine({cx + line_space, cy - line_space}, {cx + line_space, cy + line_space});
                  break;
              case '-':
                  GameCanvas->SetPenColor({0, 0, 0}).DrawLine({cx - line_space, cy - line_space}, {cx + line_space, cy - line_space}).DrawLine({cx - line_space, cy + line_space}, {cx + line_space, cy + line_space});
                  break;
              case 'S':
                  GameCanvas->SetFillColor({100, 100, 100}).DrawRect(cx - (10*cell_w / 2), cy - (10*cell_h / 2), 10*cell_w, 10*cell_h, true);
                  break;
              case 'D':
                  GameCanvas->SetFillColor({200, 200, 200}).DrawRect(cx - (10*cell_w / 2), cy - (10*cell_h / 2), 10*cell_w, 10*cell_h, true);
                  break;
              default:
                  break;
          }
      }
    }

    const double radius = std::max(std::min(cell_w, cell_h) * 0.4, 8.0);
    for (size_t id = 0; id < traffic_world->GetNumAgents(); ++id) {
        TrafficData state = traffic_world->GetAgentState(id);
        if (!state.is_active) continue;  // skip despawned agents
        WorldPosition pos = state.position;
        double cx = pos.CellX() * cell_w + cell_w / 2.0;
        double cy = pos.CellY() * cell_h + cell_h / 2.0;
        GameCanvas->SetFillColor({255, 80, 80}).DrawCircle(cx, cy, radius * 0.75, true);
    }

    if (sim_state == SimState::PLAYING) {
        int steps = StepsThisFrame();
        for (int s = 0; s < steps; ++s) {
            traffic_world->RunAgents();
            traffic_world->UpdateWorld();
            sim_tick++;
            UpdateGraphs();

            size_t n = traffic_world->GetNumAgents();
            if (traffic_reached_log.size() < n) {
                // New slots default to "was active" so the next-frame transition
                // detection treats a fresh agent as active rather than just-spawned.
                traffic_reached_log.resize(n, true);
            }
            for (size_t id = last_traffic_agent_count; id < n; ++id) {
                LogSim("Agent " + std::to_string(id), "Spawned", "info");
            }
            last_traffic_agent_count = n;

            for (size_t id = 0; id < n; ++id) {
                bool active = traffic_world->GetAgentState(id).is_active;
                bool was_active = traffic_reached_log[id];
                if (was_active && !active) {
                    LogSim("Agent " + std::to_string(id), "Reached destination", "start");
                } else if (!was_active && active) {
                    // Slot was inactive (despawned) and is now active again — recycled spawn.
                    LogSim("Agent " + std::to_string(id), "Spawned", "info");
                }
                traffic_reached_log[id] = active;
            }
        }
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
        DiseaseData state = virus_world->GetAgentState(id);
        WorldPosition pos = state.position;
        char sym = '?';
        switch (state.health) {
            case HealthState::SUSCEPTIBLE: sym = 'S'; break;
            case HealthState::INFECTED:    sym = 'I'; break;
            case HealthState::RECOVERED:   sym = 'R'; break;
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
        int steps = StepsThisFrame();
        for (int s = 0; s < steps; ++s) {
            virus_world->RunAgents();
            virus_world->UpdateWorld();
            sim_tick++;
            UpdateGraphs();

            size_t n = virus_world->GetNumAgents();
            if (last_virus_health.size() != n) last_virus_health.assign(n, -1);
            for (size_t id = 0; id < n; ++id) {
                HealthState health = virus_world->GetAgentHealth(id);
                int cur = static_cast<int>(health);
                if (cur != last_virus_health[id]) {
                    const char* label = "";
                    switch (health) {
                        case HealthState::SUSCEPTIBLE: label = "became susceptible"; break;
                        case HealthState::INFECTED:    label = "infected"; break;
                        case HealthState::RECOVERED:   label = "recovered"; break;
                    }
                    LogSim("Agent " + std::to_string(id), label,
                           health == HealthState::INFECTED ? "error" : "info");
                    last_virus_health[id] = cur;
                }
            }
        }
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

    using SwarmAgent = SwarmingAgent<DiseaseData>;
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{3, 2}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{5, 4}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{7, 3}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{4, 6}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{8, 7}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{12, 3}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{15, 5}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{14, 2}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{16, 7}});
    virus_world->AddAgent<SwarmAgent>(DiseaseData{WorldPosition{13, 7}});

    virus_world->InfectAgent(0);
}

// We switch screens by holding a shared pointer to the active screen's
// WebElement in a static map
// Note: This requires that there not be other shared pointers to the WebElement around,
// Or the screen won't be removed as it won't be destructed when we replace it!
void set_active_layout(std::shared_ptr<WebElement>&& layout) {
    elements["active_layout"] = layout;
}

// Builds the tickrate control input
std::shared_ptr<WebElement> TickRateControl() {
    auto layout = UIItem<WebLayout>(WebOptions{
        .id = "tick-rate-control",
    });

    emscripten::val document = emscripten::val::global("document");
    emscripten::val input = document.call<emscripten::val>("createElement", std::string("input"));
    input.set("id", "tick-rate-input");
    input.set("type", "number");
    input.set("value", std::to_string(sim_tps));

    emscripten::val span = document.call<emscripten::val>("createElement", std::string("span"));
    span.set("innerText", std::string(" ticks per second"));

    layout->GetDOMElement().call<void>("appendChild", input);
    layout->GetDOMElement().call<void>("appendChild", span);

    // Validate
    EM_ASM({
        var inp = document.getElementById('tick-rate-input');
        if (!inp) return;
        inp.addEventListener('change', function() { Module._validate_tick_rate(); });
    });

    return layout->SetDirection("row").SetAlignItems("center").SetGap("6px");
}

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
            (GameCanvas = UIItem<WebCanvas>(2000, 2000, WebOptions{
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
        .classes = sim == ActiveSim::TRAFFIC ? std::vector<std::string>{"traffic"} : std::vector<std::string>{},
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
                    TickRateControl(),
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
                            (log_textbox = UIItem<WebTextbox>(TextStyle(), WebOptions{
                                .id = "top-textbox",
                                .classes = { "textbox" },
                                .style = {{ "flex", "3" }}
                            })),
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
    logger.reset();
    log_textbox.reset();
    set_active_layout(MenuLayout());
}

void load_traffic_layout() {
    std::println("Loading traffic simulation");
    active_sim = ActiveSim::TRAFFIC;
    sim_state  = SimState::STOPPED;
    sim_tick   = 0;
    traffic_world = std::make_unique<WebTrafficWorld>("assets/grids/TrafficWorld_Web.grid");
    last_traffic_agent_count = 0;
    traffic_reached_log.clear();
    set_active_layout(SimulationLayout(ActiveSim::TRAFFIC, load_traffic_layout, load_menu_layout));
    logger = std::make_unique<WebTextboxOutputManager>(log_textbox);
    LogSim("World", "Traffic simulation loaded. Press start to begin.", "info");
}

void load_virus_layout() {
    std::println("Loading virus simulation");
    active_sim = ActiveSim::VIRUS;
    sim_state  = SimState::STOPPED;
    sim_tick   = 0;
    SetupVirusWorld();
    last_virus_health.clear();
    set_active_layout(SimulationLayout(ActiveSim::VIRUS, load_virus_layout, load_menu_layout));
    logger = std::make_unique<WebTextboxOutputManager>(log_textbox);
    LogSim("World", "Virus simulation loaded. Press start to begin.", "info");
}

void MainLoop() {
    if (!GameCanvas) return;
    PollTickRateInput();
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

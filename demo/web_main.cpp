/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <emscripten.h>
#include <emscripten/val.h>

#include <charconv>
#include <fstream>
#include <functional>
#include <random>
#include <sstream>

#include "WebButton.h"
#include "WebCanvas.hpp"
#include "WebImage.hpp"
#include "WebLayout.hpp"
#include "WebTextbox.hpp"
#include "Worlds/StepTrafficWorld.hpp"
#include "Worlds/InfectiousWorld.hpp"
#include "Worlds/InfectiousBuildings.hpp"
#include "tools/DataLog.hpp"
#include "core/AgentData.hpp"
#include "core/ItemBase.hpp"
#include "Agents/ScriptedAgent.hpp"
#include "Agents/SwarmingAgent.hpp"
#include "tools/Box.hpp"
#include "tools/Point.hpp"
#include "InfoGraph.hpp"
#include "WebTextboxOutputManager.hpp"
#include "Interpreter/Parser.hpp"

using namespace cse498;

// Virus / InfectiousWorld constants
constexpr size_t kGridW         = 1000;
constexpr size_t kGridH         = 1000;
constexpr size_t kNumPacers     = 6;     // ScriptedAgent residents
constexpr size_t kSpawnInterval = 10;    // new infected visitor every N ticks
constexpr size_t kMaxAgents     = 500;
constexpr size_t kStatsInterval = 50;    // log S/I/R counts every N ticks

constexpr size_t kRiverY1 = 395;
constexpr size_t kRiverY2 = 425;

// Spawn entry points
constexpr size_t kSpawnSources = 3;
constexpr size_t kSpawnX[kSpawnSources] = {200, 500, 800};
constexpr size_t kSpawnY[kSpawnSources] = {350, 350, 350};

struct Label { size_t x, y; const char* text; };
constexpr Label kLabels[] = {
    {500, 405, "RED CEDAR RIVER"},
};

// Draw a building's perimeter walls, then cut a single cell door opening at
// the center of each side. The Python extractor guarantees building boxes are
// pairwise disjoint, so we don't need to clip against neighbors here
static void PlaceBuilding(WorldGrid& grid, size_t wall_id, size_t floor_id,
                         size_t x1, size_t y1, size_t x2, size_t y2) {
    if (x2 <= x1 + 1 || y2 <= y1 + 1) return;
    for (size_t x = x1; x <= x2; ++x) {
        grid[x, y1] = wall_id;
        grid[x, y2] = wall_id;
    }
    for (size_t y = y1 + 1; y < y2; ++y) {
        grid[x1, y] = wall_id;
        grid[x2, y] = wall_id;
    }
    size_t cx = (x1 + x2) / 2;
    size_t cy = (y1 + y2) / 2;
    grid[cx, y1] = floor_id;
    grid[cx, y2] = floor_id;
    grid[x1, cy] = floor_id;
    grid[x2, cy] = floor_id;
}

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

// DataLog instances aggregate per-tick stats from the live agent set. They
// must be recreated alongside the world so their time-series start at tick 0.
static std::unique_ptr<DataLog<TrafficData>> traffic_log;
static std::unique_ptr<DataLog<DiseaseData>> virus_log;
static std::shared_ptr<WebTextbox> log_textbox;
static std::unique_ptr<WebTextboxOutputManager> logger;

// Per-sim event tracking so we can emit logs when things change
static size_t last_traffic_agent_count = 0;
static std::vector<bool> traffic_reached_log;

// IDs of scripted cars in the traffic world
static std::unordered_set<size_t> scripted_traffic_ids;

// Forward declaration
static const std::string& LoadAgentScript(const char* vfs_path);

// User-uploaded agentlang script
static std::string uploaded_script;
static std::string uploaded_script_name = "none";

// Number of agent definitions in the uploaded script. Used to render exactly
// that many spawn buttons in the navbar.
static size_t uploaded_script_def_count = 0;

// Used to detect a switch between Traffic and Virus so we can
// clear the uploaded script when crossing sim boundaries.
static ActiveSim last_loaded_sim = ActiveSim::NONE;

// Traffic light cell positions captured at world build.
static std::vector<WorldPosition> traffic_light_cells;

static std::vector<WorldPosition> traffic_destination_cells;
static std::vector<WorldPosition> traffic_spawn_cells;

// Add the agentlang-defined cars into traffic_world and seed
// scripted_traffic_ids
static void ScanTrafficCells() {
    traffic_light_cells.clear();
    traffic_destination_cells.clear();
    traffic_spawn_cells.clear();
    if (!traffic_world) return;
    const WorldGrid& grid = traffic_world->GetGrid();
    const size_t gw = grid.GetWidth();
    const size_t gh = grid.GetHeight();
    for (size_t y = 0; y < gh; ++y) {
        for (size_t x = 0; x < gw; ++x) {
            char sym = grid.GetSymbol(WorldPosition{x, y});
            if (sym == '|' || sym == '-') {
                traffic_light_cells.push_back(WorldPosition{x, y});
            } else if (sym == 'D') {
                traffic_destination_cells.push_back(WorldPosition{x, y});
            } else if (sym == 'S') {
                traffic_spawn_cells.push_back(WorldPosition{x, y});
            }
        }
    }
}

// Build a TrafficData seeded with a random spawn cell and random destination
// cell, if any are available in the current world
static TrafficData BuildRandomTrafficData() {
    static std::mt19937 rng{std::random_device{}()};
    TrafficData data{.is_active = true};
    if (!traffic_spawn_cells.empty()) {
        std::uniform_int_distribution<size_t> pick(
            0, traffic_spawn_cells.size() - 1);
        data.position = traffic_spawn_cells[pick(rng)];
    }
    if (!traffic_destination_cells.empty()) {
        std::uniform_int_distribution<size_t> pick(
            0, traffic_destination_cells.size() - 1);
        data.destination = traffic_destination_cells[pick(rng)];
    }
    return data;
}

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

// Read an agentlang script from the wasm virtual filesystem. Caches the script.
static const std::string& LoadAgentScript(const char* vfs_path) {
    static std::unordered_map<std::string, std::string> cache;
    auto it = cache.find(vfs_path);
    if (it != cache.end()) return it->second;
    std::ifstream file(vfs_path);
    if (!file) {
        std::println("Failed to open agent script '{}'", vfs_path);
        return cache.emplace(vfs_path, std::string{}).first->second;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return cache.emplace(vfs_path, ss.str()).first->second;
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

// Per-agent positions from the previous UpdateGraphs() call. Used to
// distinguish cars that actually moved this tick from those stalled at a
// light or wall, since is_active alone counts both as "active".
static std::vector<WorldPosition> prev_traffic_positions;

void UpdateGraphs() {
  if (!line_graph) return;

  // Safely read .back().sum from a DataLog field, 0 if missing/empty.
  auto latest_sum =
      [](const std::unordered_map<std::string, std::vector<TickStats>>& s,
         const std::string& field) -> double {
    auto it = s.find(field);
    if (it == s.end() || it->second.empty()) return 0.0;
    return it->second.back().sum;
  };

  if (active_sim == ActiveSim::TRAFFIC && traffic_world && traffic_log) {
    // active_count comes from DataLog. "Moving" is
    // computed locally from position deltas since DataLog doesn't expose
    // position-change.
    line_graph->SetChartType(InfoGraph::ChartType::Line);
    traffic_log->AggregateData(traffic_world->GetAgents());
    const auto& series = traffic_log->GetAggregationData();
    const double active = latest_sum(series, "active_count");

    size_t n = traffic_world->GetNumAgents();
    if (prev_traffic_positions.size() < n) prev_traffic_positions.resize(n);
    size_t moving = 0;
    for (size_t id = 0; id < n; ++id) {
      TrafficData state = traffic_world->GetAgentState(id);
      if (state.is_active && state.position != prev_traffic_positions[id]) {
        ++moving;
      }
      prev_traffic_positions[id] = state.position;
    }
    const double moving_d = static_cast<double>(moving);
    const double idle = std::max(0.0, active - moving_d);
    line_graph->AddDataPoints(
        {moving_d, idle},
        {InfoGraph::Color{80, 200, 120}, InfoGraph::Color{220, 90, 90}},
        {"Moving", "Idle"},
        "Cars / tick");
  } else if (active_sim == ActiveSim::VIRUS && virus_world && virus_log) {
    line_graph->SetChartType(InfoGraph::ChartType::Bar);
    virus_log->AggregateData(virus_world->GetAgents());
    const auto& series = virus_log->GetAggregationData();
    InfoGraph::DataSeries snapshot = {
        latest_sum(series, "susceptible_count"),
        latest_sum(series, "infection_count"),
        latest_sum(series, "cured_count"),
    };
    std::vector<InfoGraph::Color> bar_colors = {
        {60, 220, 90},    // SUSCEPTIBLE - green
        {230, 60, 60},    // INFECTED    - red
        {90, 130, 240},   // RECOVERED   - blue
    };
    std::vector<std::string> bar_labels = {"S", "I", "R"};
    line_graph->DrawBarChart(snapshot, bar_colors, bar_labels, "S / I / R");
  }
  count++;
}

// Trigger a browser download of JSON as a file.
static void DownloadJson(const std::string& filename, const std::string& json) {
    EM_ASM({
        var name = UTF8ToString($0);
        var text = UTF8ToString($1);
        var blob = new Blob([text], {type: 'application/json'});
        var url = URL.createObjectURL(blob);
        var a = document.createElement('a');
        a.href = url;
        a.download = name;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }, filename.c_str(), json.c_str());
}

std::shared_ptr<WebElement> GameInfoCanvas(WebOptions options) {
  auto gameInfo = UIItem<InfoGraph>(500, 500, options);
  line_graph = gameInfo;
  line_graph->SetAutoScroll(true);
  return gameInfo;
}

void SetupVirusWorld();
static void ResetActiveWorld();
static void BuildTrafficStaticLayer();
static void BuildVirusStaticLayer();

// Offscreen canvases that hold the world's static geometry.
static std::shared_ptr<WebCanvas> traffic_static_layer;
static std::shared_ptr<WebCanvas> virus_static_layer;

auto handle_sim_state = [](SimState next) {
    switch (next) {
        case SimState::PLAYING:
            if (sim_state != SimState::PLAYING) LogSim("World", "Simulation started", "start");
            sim_state = SimState::PLAYING;
            break;
        case SimState::PAUSED:
            if (sim_state == SimState::PLAYING) {
                sim_state = SimState::PAUSED;
                LogSim("World", "Simulation paused", "pause");
            }
            break;
        case SimState::STOPPED:
            sim_state = SimState::STOPPED;
            ResetActiveWorld();
            if (logger) logger->Clear();
            LogSim("World", "Simulation stopped and reset", "stop");
            break;
    }
};

// Log the current script status
static void LogScriptStatus() {
    LogSim("Script",
           "In use: " + uploaded_script_name +
               ". Upload a .al script at the start of a sim (before pressing start).",
           "info");
}

// Rebuild the currently active world so changes take effect
static void ResetActiveWorld() {
    sim_tick = 0;
    if (active_sim == ActiveSim::TRAFFIC) {
        traffic_world = std::make_unique<WebTrafficWorld>("assets/grids/TrafficWorld_Web.grid");
        traffic_log = std::make_unique<DataLog<TrafficData>>(WorldType::Traffic);
        scripted_traffic_ids.clear();
        ScanTrafficCells();
        prev_traffic_positions.clear();
        last_traffic_agent_count = 0;
        traffic_reached_log.clear();
        BuildTrafficStaticLayer();
    } else if (active_sim == ActiveSim::VIRUS) {
        SetupVirusWorld();
        last_virus_health.clear();
    }
    count = 0;
    if (line_graph) {
        line_graph->ClearData();
        line_graph->DrawLineChart(std::vector<double>{}, "Cleared");
    }
}

// Render the traffic world's grid into an offscreen canvas matching the
// visible canvas's pixel size.
static void BuildTrafficStaticLayer() {
    if (!GameCanvas || !traffic_world) return;
    const int W = GameCanvas->GetWidth();
    const int H = GameCanvas->GetHeight();
    traffic_static_layer = std::make_shared<WebCanvas>(W, H);

    const WorldGrid& grid = traffic_world->GetGrid();
    const size_t gw = grid.GetWidth();
    const size_t gh = grid.GetHeight();
    const double cell_w = static_cast<double>(W) / gw;
    const double cell_h = static_cast<double>(H) / gh;

    for (size_t y = 0; y < gh; ++y) {
        for (size_t x = 0; x < gw; ++x) {
            double cx = x * cell_w;
            double cy = y * cell_h;
            char sym = grid.GetSymbol(WorldPosition{x, y});
            switch (sym) {
                case '.':
                    traffic_static_layer->SetFillColor({40, 40, 40})
                        .DrawRect(cx, cy, cell_w, cell_h, true);
                    break;
                case '|':
                case '-':
                    // Traffic lights flip each phase — drawn dynamically.
                    break;
                case 'S':
                    traffic_static_layer->SetFillColor({100, 100, 100})
                        .DrawRect(cx - (10 * cell_w / 2), cy - (10 * cell_h / 2),
                                  10 * cell_w, 10 * cell_h, true);
                    break;
                case 'D':
                    traffic_static_layer->SetFillColor({200, 200, 200})
                        .DrawRect(cx - (10 * cell_w / 2), cy - (10 * cell_h / 2),
                                  10 * cell_w, 10 * cell_h, true);
                    break;
                default:
                    break;
            }
        }
    }
}

// Render the virus world's static geometry
static void BuildVirusStaticLayer() {
    if (!GameCanvas || !virus_world) return;
    const int W = GameCanvas->GetWidth();
    const int H = GameCanvas->GetHeight();
    virus_static_layer = std::make_shared<WebCanvas>(W, H);

    const WorldGrid& grid = virus_world->GetGrid();
    const size_t gw = grid.GetWidth();
    const size_t gh = grid.GetHeight();
    const double cell_w = static_cast<double>(W) / gw;
    const double cell_h = static_cast<double>(H) / gh;

    // Quarantine overlay
    virus_static_layer->SetFillColor({240, 200, 60})
        .DrawRect(850, 540, 990 - 850, 900 - 540, true);

    // Red Cedar River with bridges
    virus_static_layer->SetFillColor({60, 110, 200});
    const double river_x0 = 0.0;
    const double river_x1 = 1000.0;
    const double river_y = static_cast<double>(kRiverY1);
    const double river_h_px = static_cast<double>(kRiverY2 - kRiverY1);
    struct BridgeSpan { double x_lo, x_hi; };
    constexpr BridgeSpan kBridges[] = {{80, 130}, {470, 520}, {770, 820}};
    double cursor = river_x0;
    for (auto& br : kBridges) {
        if (br.x_lo > cursor) {
            virus_static_layer->DrawRect(cursor, river_y, br.x_lo - cursor,
                                         river_h_px, true);
        }
        cursor = br.x_hi;
    }
    if (cursor < river_x1) {
        virus_static_layer->DrawRect(cursor, river_y, river_x1 - cursor,
                                     river_h_px, true);
    }

    // Building outlines
    virus_static_layer->SetPenColor({80, 80, 80});
    constexpr double kDoorHalf = 4.0;
    for (auto& b : infectious_buildings::kBuildings) {
        double x1 = b.x1, y1 = b.y1, x2 = b.x2, y2 = b.y2;
        double cxd = (x1 + x2) * 0.5, cyd = (y1 + y2) * 0.5;
        virus_static_layer->DrawLine({x1, y1}, {cxd - kDoorHalf, y1});
        virus_static_layer->DrawLine({cxd + kDoorHalf, y1}, {x2, y1});
        virus_static_layer->DrawLine({x1, y2}, {cxd - kDoorHalf, y2});
        virus_static_layer->DrawLine({cxd + kDoorHalf, y2}, {x2, y2});
        virus_static_layer->DrawLine({x1, y1}, {x1, cyd - kDoorHalf});
        virus_static_layer->DrawLine({x1, cyd + kDoorHalf}, {x1, y2});
        virus_static_layer->DrawLine({x2, y1}, {x2, cyd - kDoorHalf});
        virus_static_layer->DrawLine({x2, cyd + kDoorHalf}, {x2, y2});
    }

    // Per-building labels
    constexpr double kBuildingLabelPx = 9.0;
    virus_static_layer
        ->SetFont(std::to_string(static_cast<int>(kBuildingLabelPx)) +
                  "px monospace")
        .SetFillColor({230, 230, 230});
    for (size_t i = 0; i < infectious_buildings::kBuildings.size(); ++i) {
        auto& b = infectious_buildings::kBuildings[i];
        double cxd = (b.x1 + b.x2) * 0.5;
        double cyd = (b.y1 + b.y2) * 0.5;
        std::string txt = std::to_string(i);
        virus_static_layer->DrawText(txt, cxd - txt.size() * 2.5,
                                     cyd + kBuildingLabelPx * 0.35);
    }

    // River label
    constexpr double kLabelFontPx = 18.0;
    virus_static_layer
        ->SetFont(std::to_string(static_cast<int>(kLabelFontPx)) +
                  "px monospace")
        .SetFillColor({0, 220, 220});
    for (auto& lbl : kLabels) {
        double lx = lbl.x * cell_w;
        double ly = lbl.y * cell_h + kLabelFontPx * 0.85;
        virus_static_layer->DrawText(lbl.text, lx, ly);
    }
}

// Number of spawn buttons rendered into the navbar at sim load
constexpr size_t kSpawnButtonCount = 8;

// Push the current script name into the nav-bar display, if it exists, and
// show/hide the Clear button + spawn buttons based on the loaded script.
static void UpdateScriptDisplay() {
    emscripten::val doc = emscripten::val::global("document");
    emscripten::val name_el = doc.call<emscripten::val>(
        "getElementById", std::string("active-script-name"));
    if (!name_el.isNull() && !name_el.isUndefined()) {
        name_el.set("innerText", uploaded_script_name);
    }
    emscripten::val clear_btn = doc.call<emscripten::val>(
        "getElementById", std::string("clear-script-btn"));
    if (!clear_btn.isNull() && !clear_btn.isUndefined()) {
        clear_btn["style"].set("display",
                               uploaded_script.empty() ? std::string("none")
                                                       : std::string("block"));
    }

    emscripten::val spawn_ctrl = doc.call<emscripten::val>(
        "getElementById", std::string("spawn-agent-control"));
    if (!spawn_ctrl.isNull() && !spawn_ctrl.isUndefined()) {
        spawn_ctrl["style"].set(
            "display",
            uploaded_script_def_count == 0 ? std::string("none")
                                           : std::string("flex"));
    }
    for (size_t i = 0; i < kSpawnButtonCount; ++i) {
        std::string btn_id = "spawn-btn-" + std::to_string(i + 1);
        emscripten::val btn = doc.call<emscripten::val>("getElementById", btn_id);
        if (btn.isNull() || btn.isUndefined()) continue;
        bool show = i < uploaded_script_def_count;
        btn["style"].set("display",
                         show ? std::string("inline-block") : std::string("none"));
    }
}

// Reset the uploaded script back to "none".
static void ResetScript() {
    uploaded_script.clear();
    uploaded_script_name = "none";
    uploaded_script_def_count = 0;
    UpdateScriptDisplay();
}

// Handle a user-selected file from the upload button. Per-state behavior:
// PLAYING: pause and tell the user to stop the sim before uploading.
// PAUSED:  tell the user to stop the sim before uploading.
// STOPPED: validate ".al" extension, parse with the agentlang Parser, and
//            install the script on success. Any failure resets to "none".
auto handle_upload = [](const std::string& filename, const std::string& content) {
    if (sim_state == SimState::PLAYING) {
        sim_state = SimState::PAUSED;
        LogSim("Script",
               "Simulation paused. End the simulation (press stop) to upload a script.",
               "warn");
        return;
    }
    if (sim_state == SimState::PAUSED) {
        LogSim("Script",
               "End the simulation (press stop) to upload a script.",
               "warn");
        return;
    }
    // STOPPED: accept upload attempt.
    auto ends_with_al = [](const std::string& s) {
        return s.size() >= 3 && s.compare(s.size() - 3, 3, ".al") == 0;
    };
    if (!ends_with_al(filename)) {
        ResetScript();
        LogSim("Script",
               "Upload rejected (must end in .al). Script in use: none.",
               "error");
        return;
    }
    Parser parser;
    std::stringstream ss{content};
    auto result = parser.parse(ss);
    if (!result.has_value()) {
        ResetScript();
        LogSim("Script",
               "Failed to parse '" + filename + "': " + result.error().ToStr() +
                   ". Script in use: none.",
               "error");
        return;
    }
    uploaded_script = content;
    uploaded_script_name = filename;
    uploaded_script_def_count = result.value().size();
    UpdateScriptDisplay();
    ResetActiveWorld();
    LogSim("Script",
           "Script uploaded: " + filename + " (" +
               std::to_string(uploaded_script_def_count) + " agent type" +
               (uploaded_script_def_count == 1 ? "" : "s") + ")",
           "info");
};

// Clear the uploaded script
auto handle_clear_script = []() {
    if (sim_state == SimState::PLAYING) {
        sim_state = SimState::PAUSED;
        LogSim("Script",
               "Simulation paused. End the simulation (press stop) to clear the script.",
               "warn");
        return;
    }
    if (sim_state == SimState::PAUSED) {
        LogSim("Script",
               "End the simulation (press stop) to clear the script.",
               "warn");
        return;
    }
    if (uploaded_script.empty()) {
        LogSim("Script", "No script to clear.", "info");
        return;
    }
    ResetScript();
    ResetActiveWorld();
    LogSim("Script", "Script cleared. Script in use: none.", "info");
};

// Spawn one scripted traffic agent of the given def_idx with a random spawn
// and destination drawn from the grid's S/D cells.
auto handle_spawn = [](size_t def_idx) {
    if (active_sim != ActiveSim::TRAFFIC || !traffic_world) {
        LogSim("Spawn", "Spawn buttons only work in the traffic sim.", "warn");
        return;
    }
    if (uploaded_script.empty()) {
        LogSim("Spawn", "No script uploaded. Upload a .al script first.", "warn");
        return;
    }
    size_t new_id = traffic_world->GetNumAgents();
    auto* agent = traffic_world->AddScriptedAgent(BuildRandomTrafficData(),
                                                  uploaded_script, def_idx);
    if (!agent) {
        LogSim("Spawn",
               "Type " + std::to_string(def_idx + 1) +
                   " not found in script (def_idx " + std::to_string(def_idx) +
                   " out of range).",
               "error");
        return;
    }
    scripted_traffic_ids.insert(new_id);
    LogSim("Spawn",
           "Spawned scripted agent type " + std::to_string(def_idx + 1),
           "info");
};

// Save the current simulation log to a downloaded JSON file.
auto handle_save = []() {
    if (!logger) return;
    bool was_playing = (sim_state == SimState::PLAYING);
    if (was_playing) {
        sim_state = SimState::PAUSED;
        LogSim("World", "Simulation paused for save", "pause");
    }
    std::string json = logger->GetBufferedLog().dump(2);
    std::string prefix = (active_sim == ActiveSim::TRAFFIC) ? "traffic_log_"
                       : (active_sim == ActiveSim::VIRUS)   ? "virus_log_"
                                                            : "sim_log_";
    std::string fname = prefix + std::to_string(sim_tick) + ".json";
    DownloadJson(fname, json);
    if (was_playing) {
        LogSim("World", "Log downloaded. Press start to resume.", "info");
    } else {
        LogSim("World", "Log downloaded.", "info");
    }
};

void DrawTrafficSim() {
    GameCanvas->Clear();
    if (traffic_static_layer) GameCanvas->DrawCanvas(*traffic_static_layer);

    const WorldGrid& grid = traffic_world->GetGrid();
    const size_t W = grid.GetWidth();
    const size_t H = grid.GetHeight();
    const double cell_w = static_cast<double>(GameCanvas->GetWidth())  / W;
    const double cell_h = static_cast<double>(GameCanvas->GetHeight()) / H;

    // Redraw traffic lights every frame against current grid symbols
    constexpr double kArrowHalf = 10.0;
    constexpr double kArrowHead = 5.0;
    GameCanvas->SetPenColor({50, 100, 255}).SetLineWidth(2.0).BeginPath();
    for (const auto& pos : traffic_light_cells) {
        char sym = grid.GetSymbol(pos);
        double cx = pos.CellX() * cell_w;
        double cy = pos.CellY() * cell_h;
        if (sym == '|') {
            GameCanvas->AddLine({cx, cy - kArrowHalf}, {cx, cy + kArrowHalf})
                       .AddLine({cx, cy - kArrowHalf},
                                {cx - kArrowHead, cy - kArrowHalf + kArrowHead})
                       .AddLine({cx, cy - kArrowHalf},
                                {cx + kArrowHead, cy - kArrowHalf + kArrowHead})
                       .AddLine({cx, cy + kArrowHalf},
                                {cx - kArrowHead, cy + kArrowHalf - kArrowHead})
                       .AddLine({cx, cy + kArrowHalf},
                                {cx + kArrowHead, cy + kArrowHalf - kArrowHead});
        } else if (sym == '-') {
            GameCanvas->AddLine({cx - kArrowHalf, cy}, {cx + kArrowHalf, cy})
                       .AddLine({cx - kArrowHalf, cy},
                                {cx - kArrowHalf + kArrowHead, cy - kArrowHead})
                       .AddLine({cx - kArrowHalf, cy},
                                {cx - kArrowHalf + kArrowHead, cy + kArrowHead})
                       .AddLine({cx + kArrowHalf, cy},
                                {cx + kArrowHalf - kArrowHead, cy - kArrowHead})
                       .AddLine({cx + kArrowHalf, cy},
                                {cx + kArrowHalf - kArrowHead, cy + kArrowHead});
        }
    }
    GameCanvas->Stroke();

    const double radius = std::max(std::min(cell_w, cell_h) * 0.4, 8.0);
    const double agent_r = radius * 0.75;
    const size_t na = traffic_world->GetNumAgents();

    GameCanvas->BeginPath();
    for (size_t id = 0; id < na; ++id) {
        if (scripted_traffic_ids.count(id)) continue;
        TrafficData state = traffic_world->GetAgentState(id);
        if (!state.is_active) continue;
        double cx = state.position.CellX() * cell_w + cell_w / 2.0;
        double cy = state.position.CellY() * cell_h + cell_h / 2.0;
        GameCanvas->AddCircle(cx, cy, agent_r);
    }
    GameCanvas->SetFillColor({255, 80, 80}).Fill();

    GameCanvas->BeginPath();
    for (size_t id : scripted_traffic_ids) {
        if (id >= na) continue;
        TrafficData state = traffic_world->GetAgentState(id);
        if (!state.is_active) continue;
        double cx = state.position.CellX() * cell_w + cell_w / 2.0;
        double cy = state.position.CellY() * cell_h + cell_h / 2.0;
        GameCanvas->AddCircle(cx, cy, agent_r);
    }
    GameCanvas->SetFillColor({80, 120, 255}).Fill();

    if (sim_state == SimState::PLAYING) {
        int steps = StepsThisFrame();
        for (int s = 0; s < steps; ++s) {
            traffic_world->RunAgents();
            traffic_world->UpdateWorld();
            sim_tick++;

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
        if (steps > 0) UpdateGraphs();
    }
}

void DrawVirusSim() {
    GameCanvas->Clear();
    if (virus_static_layer) GameCanvas->DrawCanvas(*virus_static_layer);

    const WorldGrid& grid = virus_world->GetGrid();
    const size_t W = grid.GetWidth();
    const size_t H = grid.GetHeight();
    const double cell_w = static_cast<double>(GameCanvas->GetWidth())  / W;
    const double cell_h = static_cast<double>(GameCanvas->GetHeight()) / H;

    // Agents with a small fixed radius so 1-cell movement is visible.
    constexpr double kResidentR = 5.0;
    constexpr double kVisitorR  = 4.0;
    const bool script_active = !uploaded_script.empty();
    size_t n = virus_world->GetNumAgents();

    // Batch by health color
    auto batch_fill = [&](HealthState target, WebCanvas::RGB color) {
        GameCanvas->BeginPath();
        for (size_t id = 0; id < n; ++id) {
            DiseaseData state = virus_world->GetAgentState(id);
            if (state.health != target) continue;
            bool is_scripted = script_active && id < kNumPacers;
            double r = is_scripted ? kResidentR : kVisitorR;
            double cx = state.position.CellX() * cell_w + cell_w / 2.0;
            double cy = state.position.CellY() * cell_h + cell_h / 2.0;
            GameCanvas->AddCircle(cx, cy, r);
        }
        GameCanvas->SetFillColor(color).Fill();
    };
    batch_fill(HealthState::SUSCEPTIBLE, {60, 220, 90});
    batch_fill(HealthState::INFECTED,    {230, 60, 60});
    batch_fill(HealthState::RECOVERED,   {90, 130, 240});

    // Orange outline for scripted pacers
    if (script_active) {
        GameCanvas->BeginPath();
        size_t lim = n < kNumPacers ? n : kNumPacers;
        for (size_t id = 0; id < lim; ++id) {
            DiseaseData state = virus_world->GetAgentState(id);
            double cx = state.position.CellX() * cell_w + cell_w / 2.0;
            double cy = state.position.CellY() * cell_h + cell_h / 2.0;
            GameCanvas->AddCircle(cx, cy, kResidentR + 1.5);
        }
        GameCanvas->SetPenColor({255, 140, 0}).SetLineWidth(2.0).Stroke();
    }

    if (sim_state == SimState::PLAYING) {
        int steps = StepsThisFrame();
        for (int s = 0; s < steps; ++s) {
            // Periodic infected visitor spawn
            if (sim_tick > 0 && sim_tick % kSpawnInterval == 0 &&
                virus_world->GetNumAgents() < kMaxAgents) {
                size_t src = (sim_tick / kSpawnInterval) % kSpawnSources;
                virus_world->AddAgent<SwarmingAgent<DiseaseData>>(
                    DiseaseData{WorldPosition{kSpawnX[src], kSpawnY[src]}});
                virus_world->InfectAgent(virus_world->GetNumAgents() - 1);
                LogSim("World",
                       "Visitor spawned at (" + std::to_string(kSpawnX[src]) + "," +
                           std::to_string(kSpawnY[src]) + ")",
                       "warn");
            }

            // Each visible step advances the world several times so agents
            constexpr int kVirusStepsPerTick = 4;
            for (int sub = 0; sub < kVirusStepsPerTick; ++sub) {
                virus_world->RunAgents();
                virus_world->UpdateWorld();
            }
            sim_tick++;

            size_t cur_n = virus_world->GetNumAgents();
            if (last_virus_health.size() < cur_n) last_virus_health.resize(cur_n, -1);
            for (size_t id = 0; id < cur_n; ++id) {
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

            // Periodic stats summary
            if (sim_tick > 0 && sim_tick % kStatsInterval == 0) {
                LogSim("World",
                       "S=" + std::to_string(virus_world->GetSusceptibleCount()) +
                           "  I=" + std::to_string(virus_world->GetInfectedCount()) +
                           "  R=" + std::to_string(virus_world->GetRecoveredCount()) +
                           "  total=" + std::to_string(virus_world->GetNumAgents()),
                       "info");
            }
        }
        if (steps > 0) UpdateGraphs();
    }
}

void SetupVirusWorld() {
    virus_world = std::make_unique<InfectiousWorld>(kGridW, kGridH);
    virus_log = std::make_unique<DataLog<DiseaseData>>(WorldType::Infection);
    WorldGrid& grid = virus_world->GetGrid();
    size_t wall  = virus_world->GetWallID();
    size_t floor = virus_world->GetFloorID();

    // Outer border
    for (size_t y = 0; y < kGridH; ++y) {
        grid[0,         y] = wall;
        grid[kGridW - 1, y] = wall;
    }
    for (size_t x = 0; x < kGridW; ++x) {
        grid[x, 0          ] = wall;
        grid[x, kGridH - 1] = wall;
    }

    // Auto generated building outlines from full_map
    for (auto& b : infectious_buildings::kBuildings) {
        PlaceBuilding(grid, wall, floor, b.x1, b.y1, b.x2, b.y2);
    }

    // Red Cedar River across the middle band, with three bridges.
    for (size_t y = kRiverY1; y <= kRiverY2; ++y) {
        for (size_t x = 1; x <= kGridW - 2; ++x) grid[x, y] = wall;
    }
    auto bridge = [&](size_t x_lo, size_t x_hi) {
        for (size_t y = kRiverY1; y <= kRiverY2; ++y) {
            for (size_t x = x_lo; x <= x_hi; ++x) grid[x, y] = floor;
        }
    };
    bridge( 80, 130);
    bridge(470, 520);
    bridge(770, 820);

    // Quarantine zone
    virus_world->AddQuarantineZone(
        Box::FromCorners(Point(850.0, 540.0), Point(990.0, 900.0)));

    // Disease parameters
    virus_world->SetTransmissionRate(0.50);
    virus_world->SetInfectionRadius(25.0);
    virus_world->SetTreatmentDuration(40);
    virus_world->SetImmunityDuration(50);
    virus_world->SetFallbackRecoveryTicks(100);
    virus_world->SetClinicEntrance(WorldPosition{900, 700});
    virus_world->SetRecoveryExit(WorldPosition{780, 470});

    if (!uploaded_script.empty()) {
        struct PacerSpec { WorldPosition pos; size_t def_idx; };
        const PacerSpec kPacers[] = {
            {{ 80, 200}, 0},   // h_pacer
            {{220, 200}, 0},   // h_pacer
            {{420, 200}, 1},   // v_pacer
            {{ 60, 700}, 1},   // v_pacer
            {{260, 700}, 2},   // square_walker
            {{600, 200}, 3},   // stander
        };
        for (const auto& p : kPacers) {
          virus_world->AddScriptedAgent(DiseaseData{p.pos}, uploaded_script, p.def_idx);
        }
    }

    // Initial swarming visitors
    using Swarm = SwarmingAgent<DiseaseData>;
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{160, 360}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{490, 360}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{760, 360}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{180, 460}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{490, 460}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{690, 460}});
    virus_world->AddAgent<Swarm>(DiseaseData{WorldPosition{520, 580}});

    // Patient zero: first swarming agent.
    virus_world->InfectAgent(kNumPacers);

    BuildVirusStaticLayer();
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

// Builds a nav-bar element showing the active script name and a clear button.
std::shared_ptr<WebElement> ScriptStatusControl() {
    auto layout = UIItem<WebLayout>(WebOptions{
        .id = "script-status-control",
    });

    emscripten::val document = emscripten::val::global("document");
    emscripten::val label = document.call<emscripten::val>("createElement", std::string("span"));
    label.set("innerText", std::string("Script: "));

    emscripten::val name = document.call<emscripten::val>("createElement", std::string("span"));
    name.set("id", "active-script-name");
    name.set("innerText", uploaded_script_name);

    layout->GetDOMElement().call<void>("appendChild", label);
    layout->GetDOMElement().call<void>("appendChild", name);

    return layout->SetDirection("row").SetAlignItems("center").SetGap("6px");
}

std::shared_ptr<WebElement> SpawnAgentControl() {
    std::vector<std::shared_ptr<WebElement>> buttons;
    for (size_t i = 0; i < kSpawnButtonCount; ++i) {
        size_t def_idx = i;
        buttons.push_back(
            UIItem<WebButton>(std::to_string(i + 1), WebOptions{
                .id = "spawn-btn-" + std::to_string(i + 1),
                .classes = {"spawn-btn"},
            })->SetOnClick([def_idx]() { handle_spawn(def_idx); }));
    }
    auto layout = UIItem<WebLayout>(WebOptions{
        .id = "spawn-agent-control",
        .children = buttons,
    });

    emscripten::val document = emscripten::val::global("document");
    emscripten::val label = document.call<emscripten::val>("createElement", std::string("span"));
    label.set("innerText", std::string("Spawn Agent Type"));
    layout->GetDOMElement().call<void>("prepend", label);

    return layout->SetDirection("row").SetAlignItems("center").SetGap("4px");
}

// Here we create a function that creates the layout for the simulation screen
// It takes a lambda as a parameter
// This is a function we want to run when a button is clicked
// We could take more lambdas if we want more types of button handlers
// exit_lambda: always goes back to the main menu for reselection of world or exit
std::shared_ptr<WebElement> SimulationLayout(ActiveSim sim, std::function<void()> exit_lambda) {
    // Here we initialize our game info canvas with id, classes, and style properties.
    auto gameInfo = GameInfoCanvas(WebOptions{
        .id = "game-info",
        .classes = {"textbox"},
        .style = {{"flex", "7"}}
    });

    std::vector<std::shared_ptr<WebElement>> game_children;

    if (sim == ActiveSim::TRAFFIC) {
        game_children.push_back(
            UIItem<WebImage>("assets/images/full_map_no_road.svg", "Game map", WebOptions{
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
            (GameCanvas = UIItem<WebCanvas>(1000, 1000, WebOptions{
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
                    UIItem<WebButton>("", WebOptions{ .id = "upload-btn", .classes={"button"} })->SetOnFileUploadWithName(handle_upload),
                    UIItem<WebButton>("", WebOptions{ .id = "save-btn", .classes={"button"} })->SetOnClick(handle_save),
                    TickRateControl(),
                    SpawnAgentControl(),
                    ScriptStatusControl(),
                    UIItem<WebButton>("", WebOptions{ .id = "clear-script-btn", .classes={"button"} })->SetOnClick(handle_clear_script),
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
    traffic_static_layer.reset();
    virus_static_layer.reset();
    logger.reset();
    log_textbox.reset();
    set_active_layout(MenuLayout());
}

void load_traffic_layout() {
    std::println("Loading traffic simulation");
    if (last_loaded_sim == ActiveSim::VIRUS) ResetScript();
    last_loaded_sim = ActiveSim::TRAFFIC;
    active_sim = ActiveSim::TRAFFIC;
    sim_state  = SimState::STOPPED;
    sim_tick   = 0;
    traffic_world = std::make_unique<WebTrafficWorld>("assets/grids/TrafficWorld_Web.grid");
    traffic_log = std::make_unique<DataLog<TrafficData>>(WorldType::Traffic);
    scripted_traffic_ids.clear();
    ScanTrafficCells();
    prev_traffic_positions.clear();

    last_traffic_agent_count = 0;
    traffic_reached_log.clear();
    set_active_layout(SimulationLayout(ActiveSim::TRAFFIC, load_menu_layout));
    BuildTrafficStaticLayer();
    logger = std::make_unique<WebTextboxOutputManager>(log_textbox);
    LogSim("World", "Traffic simulation loaded. Press start to begin.", "info");
    LogScriptStatus();
    UpdateScriptDisplay();
}

void load_virus_layout() {
    std::println("Loading virus simulation");
    if (last_loaded_sim == ActiveSim::TRAFFIC) ResetScript();
    last_loaded_sim = ActiveSim::VIRUS;
    active_sim = ActiveSim::VIRUS;
    sim_state  = SimState::STOPPED;
    sim_tick   = 0;
    SetupVirusWorld();
    last_virus_health.clear();
    set_active_layout(SimulationLayout(ActiveSim::VIRUS, load_menu_layout));
    BuildVirusStaticLayer();
    logger = std::make_unique<WebTextboxOutputManager>(log_textbox);
    LogSim("World", "Virus simulation loaded. Press start to begin.", "info");
    LogScriptStatus();
    UpdateScriptDisplay();
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

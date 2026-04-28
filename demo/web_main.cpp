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
#include "Worlds/InfectiousBuildings.hpp"
#include "Worlds/WebTrafficWorld.hpp"
#include "core/AgentData.hpp"
#include "core/ItemBase.hpp"
#include "Agents/ScriptedAgent.hpp"
#include "Agents/SwarmingAgent.hpp"
#include "tools/Box.hpp"
#include "tools/Point.hpp"
#include "InfoGraph.hpp"
#include "WebTextboxOutputManager.hpp"
#include "tools/DataLog.hpp"

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

/// Used for logging traffic data for graphs
auto kTrafficLog = DataLog<WebTrafficWorld>(WorldType::Traffic);
/// Used for logging virus data for graphs
auto kVirusLog   = DataLog<InfectiousWorld>(WorldType::Infection);

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
    if (active_sim == ActiveSim::TRAFFIC) {
      auto& stats = kTrafficLog.GetAggregationData();
      if (stats.contains("driving_count") && !stats.at("driving_count").empty()) {
        line_graph->SetChartType(InfoGraph::ChartType::Line);
        line_graph->AddDataPoint(stats.at("driving_count").back().current, "Driving Agents");
      }
    } else if (active_sim == ActiveSim::VIRUS) {
      auto& stats = kVirusLog.GetAggregationData();
      bool has_i = stats.contains("infected_count") && !stats.at("infected_count").empty();
      bool has_s = stats.contains("susceptible_count") && !stats.at("susceptible_count").empty();
      bool has_r = stats.contains("recovered_count") && !stats.at("recovered_count").empty();

      if (has_i && has_s && has_r) {
        line_graph->SetChartType(InfoGraph::ChartType::Bar);
        std::vector<double> bar_data = {
            stats.at("infected_count").back().current,
            stats.at("susceptible_count").back().current,
            stats.at("recovered_count").back().current
        };
        line_graph->DrawBarChart(bar_data, "I / S / R Population");
      }
    } else {
      line_graph->SetChartType(InfoGraph::ChartType::Line);
      line_graph->AddDataPoint(count, "Counting Test");
      count++;
    }
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

            // Aggregate data
            kTrafficLog.AggregateData(*traffic_world);

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
    const double cell_w = static_cast<double>(GameCanvas->GetWidth())  / W;
    const double cell_h = static_cast<double>(GameCanvas->GetHeight()) / H;

    // Quarantine overlay
    GameCanvas->SetFillColor({240, 200, 60})
        .DrawRect(850, 540, 990 - 850, 900 - 540, true);

    // Red Cedar River with bridges
    GameCanvas->SetFillColor({60, 110, 200});
    const double river_x0 = 0.0;
    const double river_x1 = 1000.0;
    const double river_y = static_cast<double>(kRiverY1);
    const double river_h = static_cast<double>(kRiverY2 - kRiverY1);
    struct BridgeSpan { double x_lo, x_hi; };
    constexpr BridgeSpan kBridges[] = {{80, 130}, {470, 520}, {770, 820}};
    double cursor = river_x0;
    for (auto& br : kBridges) {
        if (br.x_lo > cursor) {
            GameCanvas->DrawRect(cursor, river_y, br.x_lo - cursor, river_h, true);
        }
        cursor = br.x_hi;
    }
    if (cursor < river_x1) {
        GameCanvas->DrawRect(cursor, river_y, river_x1 - cursor, river_h, true);
    }

    // Building outlines
    GameCanvas->SetPenColor({80, 80, 80});
    constexpr double kDoorHalf = 4.0;  // half-width of door gap, in canvas px
    for (auto& b : infectious_buildings::kBuildings) {
        double x1 = b.x1, y1 = b.y1, x2 = b.x2, y2 = b.y2;
        double cxd = (x1 + x2) * 0.5, cyd = (y1 + y2) * 0.5;
        // Top edge
        GameCanvas->DrawLine({x1, y1}, {cxd - kDoorHalf, y1});
        GameCanvas->DrawLine({cxd + kDoorHalf, y1}, {x2, y1});
        // Bottom edge
        GameCanvas->DrawLine({x1, y2}, {cxd - kDoorHalf, y2});
        GameCanvas->DrawLine({cxd + kDoorHalf, y2}, {x2, y2});
        // Left edge
        GameCanvas->DrawLine({x1, y1}, {x1, cyd - kDoorHalf});
        GameCanvas->DrawLine({x1, cyd + kDoorHalf}, {x1, y2});
        // Right edge
        GameCanvas->DrawLine({x2, y1}, {x2, cyd - kDoorHalf});
        GameCanvas->DrawLine({x2, cyd + kDoorHalf}, {x2, y2});
    }

    // Per building labels at the center of each box. Using the building index
    constexpr double kBuildingLabelPx = 9.0;
    GameCanvas->SetFont(std::to_string(static_cast<int>(kBuildingLabelPx)) + "px monospace")
              .SetFillColor({230, 230, 230});
    for (size_t i = 0; i < infectious_buildings::kBuildings.size(); ++i) {
        auto& b = infectious_buildings::kBuildings[i];
        double cxd = (b.x1 + b.x2) * 0.5;
        double cyd = (b.y1 + b.y2) * 0.5;
        std::string txt = std::to_string(i);
        // Horizontal centering
        GameCanvas->DrawText(txt, cxd - txt.size() * 2.5, cyd + kBuildingLabelPx * 0.35);
    }

    // Special location labels, river
    constexpr double kLabelFontPx = 18.0;
    GameCanvas->SetFont(std::to_string(static_cast<int>(kLabelFontPx)) + "px monospace")
              .SetFillColor({0, 220, 220});
    for (auto& lbl : kLabels) {
        double lx = lbl.x * cell_w;
        double ly = lbl.y * cell_h + kLabelFontPx * 0.85;
        GameCanvas->DrawText(lbl.text, lx, ly);
    }

    // Agents with a small fixed radius so 1-cell movement is visible.
    constexpr double kResidentR = 5.0;
    constexpr double kVisitorR  = 4.0;
    size_t n = virus_world->GetNumAgents();
    for (size_t id = 0; id < n; ++id) {
        DiseaseData state = virus_world->GetAgentState(id);
        WorldPosition pos = state.position;
        double cx = pos.CellX() * cell_w + cell_w / 2.0;
        double cy = pos.CellY() * cell_h + cell_h / 2.0;
        bool is_resident = id < kNumPacers;
        double r = is_resident ? kResidentR : kVisitorR;
        switch (state.health) {
            case HealthState::SUSCEPTIBLE: GameCanvas->SetFillColor({60, 220, 90});  break;
            case HealthState::INFECTED:    GameCanvas->SetFillColor({230, 60, 60});  break;
            case HealthState::RECOVERED:   GameCanvas->SetFillColor({90, 130, 240}); break;
        }
        GameCanvas->DrawCircle(cx, cy, r, true);
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

            // Aggregate data
            kVirusLog.AggregateData(*virus_world);

            UpdateGraphs();

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
    }
}

void SetupVirusWorld() {
    virus_world = std::make_unique<InfectiousWorld>(kGridW, kGridH);
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

    // Resident pacers
    using Pacer = ScriptedAgent<DiseaseData>;
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{ 80, 200}});
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{220, 200}});
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{420, 200}});
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{ 60, 700}});
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{260, 700}});
    virus_world->AddAgent<Pacer>(DiseaseData{WorldPosition{600, 200}});

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
        .style = {
          {"margin", "1rem"},
        },
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
                          // Allow left column to shrink down to a reasonable width (smaller viewport will overflow)
                            {"min-width", "200px"},
                            {"flex", "0 10 500px"},
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
                    })->SetHeight("100%").SetDirection("column").SetGap("10px").SetAlignItems("stretch").SetJustifyContent("center"),
                    game_area
                }
            })
                ->SetHeight("80vh").SetDirection("row").SetGap("20px").SetAlignItems("stretch").SetJustifyContent("center")
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

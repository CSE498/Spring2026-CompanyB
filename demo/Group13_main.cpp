/**
 * @file Group13_main.cpp
 * @brief Demo of InfectiousWorld using the step-based agent system.
 *
 * compile (from demo/ directory):
 * g++ -std=c++23 -DDEMO_GROUP13 -I../source \
 * Group13_main.cpp -o infectious_demo \
 * ../source/tools/Point.cpp ../source/tools/Box.cpp
 * run:
 * ./infectious_demo
 **/

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../source/Agents/ScriptedAgent.hpp"
#include "../source/Agents/SwarmingAgent.hpp"
#include "../source/Worlds/InfectiousWorld.hpp"

using namespace cse498;

// ---------------------------------------------------------------------------
// Demo constants
// ---------------------------------------------------------------------------
namespace demo {
constexpr int    kFrameDelayMs  = 200;
constexpr size_t kSimTicks      = 400;
constexpr size_t kNumPacers      = 6;    // IDs 0..5: building residents
constexpr size_t kGridW          = 90;
constexpr size_t kGridH          = 33;
constexpr size_t kSpawnInterval = 12;   // new infected visitor every N ticks
constexpr size_t kMaxAgents      = 60;

// Red Cedar River rows
constexpr size_t kRiverY1 = 14;
constexpr size_t kRiverY2 = 16;

// Quarantine ward inside Olin Health Center
constexpr double kQX1 = 77.0, kQY1 = 18.0;
constexpr double kQX2 = 89.0, kQY2 = 30.0;

// Three spawn entry points
constexpr size_t kSpawnSources = 3;
constexpr size_t kSpawnX[kSpawnSources] = {8, 44, 73};
constexpr size_t kSpawnY[kSpawnSources] = {12, 12, 12};
}  // namespace demo

// ---------------------------------------------------------------------------
// Visualization Constants
// ---------------------------------------------------------------------------

namespace ansi {
constexpr const char* RESET        = "\033[0m";
constexpr const char* BOLD         = "\033[1m";
constexpr const char* WHITE        = "\033[97m";
constexpr const char* GREEN        = "\033[32m";
constexpr const char* RED          = "\033[31m";
constexpr const char* BLUE         = "\033[34m";
constexpr const char* CYAN         = "\033[36m";
constexpr const char* YELLOW       = "\033[33m";
constexpr const char* GREY         = "\033[90m";
constexpr const char* BG_BLUE      = "\033[44m";
constexpr const char* BG_YELLOW    = "\033[43m";
constexpr const char* CLEAR_SCREEN = "\033[2J\033[H";
}  // namespace ansi

namespace map_sym {
constexpr char WALL       = '#';
constexpr char FLOOR      = '.';
constexpr char RIVER      = '~';
constexpr char QUARANTINE = '~';
constexpr char PACER      = 'P';
constexpr char VISITOR    = 'W';
} // namespace map_sym

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------

/**
 * @brief Generates a patrol script string for ScriptedAgents.
 * * Creates a sequence of movement commands (u, d, l, r) that makes an
 * agent walk in a rectangular pattern.
 * * @param steps_v Number of steps to move vertically (Up then Down).
 * @param steps_h Number of steps to move horizontally (Left then Right).
 * @return std::string A script string.
 */
std::string CreatePatrolScript(size_t steps_v, size_t steps_h) {
    std::string script = "";
    script += std::string(steps_v, 'u');
    script += std::string(steps_h, 'l');
    script += std::string(steps_v, 'd');
    script += std::string(steps_h, 'r');
    return script;
}

/**
 * @brief Renders the current state of the InfectiousWorld to the terminal.
 * * Handles background tiles (river, quarantine, walls), building labels,
 * and agent positions with color coding based on health status.
 * * @param world The InfectiousWorld instance to visualize.
 */
void DrawWorld(const InfectiousWorld& world) {
    const WorldGrid& grid = world.GetGrid();
    size_t W = grid.GetWidth(), H = grid.GetHeight();

    struct Cell { char sym; const char* fg; const char* bg; };
    std::vector<std::vector<Cell>> display(H,
        std::vector<Cell>(W, {map_sym::FLOOR, ansi::GREY, ""}));

    // 1. Background tiles
    for (size_t y = 0; y < H; ++y) {
        for (size_t x = 0; x < W; ++x) {
            char s = grid.GetSymbol(WorldPosition{x, y});
            if (s == '#' && y >= demo::kRiverY1 && y <= demo::kRiverY2) {
                display[y][x] = {map_sym::RIVER, ansi::BLUE, ansi::BG_BLUE};
            } else if (s == '#') {
                display[y][x] = {map_sym::WALL, ansi::GREY, ""};
            } else if (world.IsInQuarantine(WorldPosition{x, y})) {
                display[y][x] = {map_sym::QUARANTINE, ansi::YELLOW, ansi::BG_YELLOW};
            } else {
                display[y][x] = {map_sym::FLOOR, ansi::GREY, ""};
            }
        }
    }

    // 2. Building names
    struct Label { size_t x, y; const char* text; };
    static constexpr Label kLabels[] = {
        { 3,  6, "WELLS HALL"}, {18,  6, "MSU UNION"}, {34,  6, "BERKEY"},
        {49,  6, "SHAW HALL"},  {65,  6, "LIBRARY"},   {81,  6, "NAT.SCI"},
        {20, 15, "RED CEDAR RIVER"},
        { 3, 23, "ENGINEERING"}, {20, 22, "BRODY HALL"},
        {42, 24, "STADIUM"},     {80, 23, "OLIN HEALTH"},
    };
    for (auto& lbl : kLabels) {
        for (size_t i = 0; lbl.text[i] != '\0' && lbl.x + i < W; ++i) {
            auto& cell = display[lbl.y][lbl.x + i];
            if (cell.sym == map_sym::FLOOR || cell.sym == map_sym::RIVER) {
                cell.sym = lbl.text[i];
                cell.fg  = ansi::CYAN;
            }
        }
    }

    // 3. Agent overlay
    for (size_t id = 0; id < world.GetNumAgents(); ++id) {
        DiseaseData   st = world.GetAgentState(id);
        WorldPosition p = st.position;
        bool is_resident = (id < demo::kNumPacers);

        const char* col = ansi::GREEN;
        if (st.health == HealthState::INFECTED)  col = ansi::RED;
        if (st.health == HealthState::RECOVERED) col = ansi::BLUE;

        char sym = is_resident ? map_sym::PACER : map_sym::VISITOR;
        display[p.CellY()][p.CellX()] = {sym, col, ""};
    }

    // 4. Render to terminal
    std::cout << ansi::CLEAR_SCREEN;
    std::cout << ansi::BOLD
              << "=== MSU Campus Infectious Disease Simulation (Group 13) ===\n"
              << ansi::RESET;
    std::cout << "  Tick: " << std::setw(4) << world.GetTickCount()
              << "   |   "
              << ansi::GREEN  << "Susceptible: " << std::setw(3)
              << world.GetSusceptibleCount() << ansi::RESET << "   "
              << ansi::RED    << "Infected: "    << std::setw(3)
              << world.GetInfectedCount()    << ansi::RESET << "   "
              << ansi::BLUE   << "Recovered: "   << std::setw(3)
              << world.GetRecoveredCount()   << ansi::RESET << "\n\n";

    std::cout << "  +" << std::string(W, '-') << "+\n";
    for (size_t y = 0; y < H; ++y) {
        std::cout << "  |";
        for (size_t x = 0; x < W; ++x) {
            const auto& c = display[y][x];
            std::cout << c.bg << c.fg << c.sym << ansi::RESET;
        }
        std::cout << "|\n";
    }
    std::cout << "  +" << std::string(W, '-') << "+\n\n";

    // Legend
    std::cout
        << "  Agents:  "
        << ansi::GREEN  << map_sym::PACER << ansi::RESET << " Scripted Agent "
        << ansi::CYAN   << map_sym::VISITOR << ansi::RESET << " Swarming Agent\n"
        << "  Health:  "
        << ansi::GREEN  << "green"  << ansi::RESET << "=susceptible  "
        << ansi::RED    << "red"    << ansi::RESET << "=infected  "
        << ansi::BLUE   << "blue"   << ansi::RESET << "=recovered\n"
        << "  "
        << ansi::YELLOW << ansi::BG_YELLOW << map_sym::QUARANTINE << ansi::RESET
        << ansi::YELLOW << " Olin Health (enter infected → recover → leave)" << ansi::RESET
        << "   agents: " << world.GetNumAgents()
        << " / " << demo::kMaxAgents << "\n";
}

/**
 * @brief Draws four walls of a rectangular building and cuts door openings.
 * * @param grid The grid to modify.
 * @param wall_id The value to use for wall cells.
 * @param floor_id The value to use for door/floor cells.
 * @param x1 Top-left X coordinate.
 * @param y1 Top-left Y coordinate.
 * @param x2 Bottom-right X coordinate.
 * @param y2 Bottom-right Y coordinate.
 * @param doors List of {x, y} pairs to be set as floor_id (entrances).
 */
static void DrawBuilding(
    WorldGrid& grid, size_t wall_id, size_t floor_id,
    size_t x1, size_t y1, size_t x2, size_t y2,
    std::initializer_list<std::pair<size_t, size_t>> doors)
{
    for (size_t x = x1; x <= x2; ++x) {
        grid[x, y1] = wall_id;
        grid[x, y2] = wall_id;
    }
    for (size_t y = y1 + 1; y < y2; ++y) {
        grid[x1, y] = wall_id;
        grid[x2, y] = wall_id;
    }
    for (auto [dx, dy] : doors)
        grid[dx, dy] = floor_id;
}

/**
 * @brief Main execution function for the campus simulation.
 * * Initializes the world layout, sets disease parameters, populates initial
 * agents, and runs the simulation loop.
 * * @return int Status code (0 for success).
 */
int RunInfectiousDemo() {
    InfectiousWorld world(demo::kGridW, demo::kGridH);
    WorldGrid& grid  = world.GetGrid();
    size_t wall      = world.GetWallID();
    size_t floor     = world.GetFloorID();

    // --- Outer border ---
    for (size_t y = 0; y < demo::kGridH; ++y) {
        grid[0, y] = wall;
        grid[demo::kGridW - 1, y] = wall;
    }
    for (size_t x = 0; x < demo::kGridW; ++x) {
        grid[x, 0] = wall;
        grid[x, demo::kGridH - 1] = wall;
    }

    // --- Build Campus ---
    DrawBuilding(grid, wall, floor,  2,  2, 13, 11, {{7,11}, {13,6}});   // Wells
    DrawBuilding(grid, wall, floor, 17,  2, 29, 11, {{17,6}, {23,11}}); // Union
    DrawBuilding(grid, wall, floor, 33,  2, 42, 11, {{33,6}, {38,11}}); // Berkey
    DrawBuilding(grid, wall, floor, 48,  2, 60, 11, {{48,6}, {54,11}}); // Shaw
    DrawBuilding(grid, wall, floor, 64,  2, 76, 11, {{64,6}, {70,11}}); // Library
    DrawBuilding(grid, wall, floor, 80,  2, 88, 11, {{80,6}, {84,11}}); // NatSci

    // Red Cedar River & Bridges
    for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
        for (size_t x = 1; x <= demo::kGridW - 2; ++x)
            grid[x, y] = wall;

    auto make_bridge = [&](size_t x_start) {
        for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y)
            for (size_t x = x_start; x <= x_start + 3; ++x)
                grid[x, y] = floor;
    };
    make_bridge(7); make_bridge(43); make_bridge(69);

    DrawBuilding(grid, wall, floor,  2, 19, 15, 28, {{9,19}, {15,24}}); // EB
    DrawBuilding(grid, wall, floor, 19, 19, 32, 27, {{26,19}, {19,23}}); // Brody
    DrawBuilding(grid, wall, floor, 36, 19, 57, 30, {{47,19}, {36,24}}); // Stadium

    // --- Quarantine Setup ---
    world.AddQuarantineZone(Box::FromCorners(Point(demo::kQX1, demo::kQY1), Point(demo::kQX2, demo::kQY2)));
    world.SetClinicEntrance(WorldPosition{82, 24});
    world.SetRecoveryExit(WorldPosition{70, 17});

    // --- Disease parameters ---
    world.SetTransmissionRate(0.50);
    world.SetInfectionRadius(2.5);
    world.SetTreatmentDuration(40);
    world.SetImmunityDuration(50);
    world.SetFallbackRecoveryTicks(100);

    // --- Initialize Agents ---
    auto add_pacer = [&](WorldPosition pos, std::string script) {
        auto& pacer = world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{pos});
        pacer.SetScript(script);
    };

    // Add residents with patrol paths
    add_pacer(WorldPosition{7, 6},   CreatePatrolScript(3, 4));
    add_pacer(WorldPosition{20, 7},  CreatePatrolScript(2, 5));
    add_pacer(WorldPosition{37, 6},  CreatePatrolScript(4, 2));
    add_pacer(WorldPosition{5, 24},  CreatePatrolScript(3, 3));
    add_pacer(WorldPosition{22, 23}, CreatePatrolScript(2, 4));
    add_pacer(WorldPosition{52, 6},  CreatePatrolScript(4, 4));

    // Initial swarming visitors
    world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{14, 12}});
    world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{44, 12}});
    world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{68, 12}});

    // Patient zero
    world.InfectAgent(demo::kNumPacers);

    // --- Simulation Loop ---
    for (size_t tick = 0; tick < demo::kSimTicks; ++tick) {
        if (tick > 0 && tick % demo::kSpawnInterval == 0 && world.GetNumAgents() < demo::kMaxAgents) {
            size_t src = (tick / demo::kSpawnInterval) % demo::kSpawnSources;
            world.AddAgent<SwarmingAgent<DiseaseData>>(
                DiseaseData{WorldPosition{demo::kSpawnX[src], demo::kSpawnY[src]}});
            world.InfectAgent(world.GetNumAgents() - 1);
        }

        DrawWorld(world);
        world.RunAgents();
        world.UpdateWorld();
        std::this_thread::sleep_for(std::chrono::milliseconds(demo::kFrameDelayMs));
    }

    DrawWorld(world);
    std::cout << "\n" << ansi::BOLD << "=== Simulation Complete ===\n" << ansi::RESET;
    return 0;
}

#if defined(DEMO_GROUP13)
int main() { return RunInfectiousDemo(); }
#endif
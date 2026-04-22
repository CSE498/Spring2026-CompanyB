#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>

#include "Agents/ScriptedAgent.hpp"
#include "Agents/SwarmingAgent.hpp"
#include "Worlds/InfectiousWorld.hpp"
#include "core/BenchRunner.hpp"
#include "core/MetricCollector.hpp"

using namespace cse498;

namespace demo {
constexpr std::size_t kSimTicks = 400;
constexpr std::size_t kNumPacers = 6;
constexpr std::size_t kGridW = 90;
constexpr std::size_t kGridH = 33;
constexpr std::size_t kSpawnInterval = 12;
constexpr std::size_t kMaxAgents = 60;

constexpr std::size_t kRiverY1 = 14;
constexpr std::size_t kRiverY2 = 16;

constexpr double kQX1 = 77.0;
constexpr double kQY1 = 18.0;
constexpr double kQX2 = 89.0;
constexpr double kQY2 = 30.0;

constexpr std::size_t kSpawnSources = 3;
constexpr std::size_t kSpawnX[kSpawnSources] = {8, 44, 73};
constexpr std::size_t kSpawnY[kSpawnSources] = {12, 12, 12};
}  // namespace demo

namespace {

void DrawBuilding(WorldGrid& grid,
									size_t wall_id,
									size_t floor_id,
									size_t x1,
									size_t y1,
									size_t x2,
									size_t y2,
									std::initializer_list<std::pair<size_t, size_t>> doors) {
	for (size_t x = x1; x <= x2; ++x) {
		grid[x, y1] = wall_id;
		grid[x, y2] = wall_id;
	}

	for (size_t y = y1 + 1; y < y2; ++y) {
		grid[x1, y] = wall_id;
		grid[x2, y] = wall_id;
	}

	for (const auto& [dx, dy] : doors) {
		grid[dx, dy] = floor_id;
	}
}

void ConfigureCampus(InfectiousWorld& world) {
	WorldGrid& grid = world.GetGrid();
	const size_t wall = world.GetWallID();
	const size_t floor = world.GetFloorID();

	for (size_t y = 0; y < demo::kGridH; ++y) {
		grid[0, y] = wall;
		grid[demo::kGridW - 1, y] = wall;
	}

	for (size_t x = 0; x < demo::kGridW; ++x) {
		grid[x, 0] = wall;
		grid[x, demo::kGridH - 1] = wall;
	}

	DrawBuilding(grid, wall, floor, 2, 2, 13, 11, {{7, 11}, {13, 6}});
	DrawBuilding(grid, wall, floor, 17, 2, 29, 11,
							 {{17, 6}, {23, 11}, {29, 6}, {23, 2}});
	DrawBuilding(grid, wall, floor, 33, 2, 42, 11, {{33, 6}, {38, 11}});
	DrawBuilding(grid, wall, floor, 48, 2, 60, 11, {{48, 6}, {54, 11}});
	DrawBuilding(grid, wall, floor, 64, 2, 76, 11,
							 {{64, 6}, {70, 11}, {76, 6}});
	DrawBuilding(grid, wall, floor, 80, 2, 88, 11, {{80, 6}, {84, 11}});

	for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y) {
		for (size_t x = 1; x <= demo::kGridW - 2; ++x) {
			grid[x, y] = wall;
		}
	}

	for (size_t y = demo::kRiverY1; y <= demo::kRiverY2; ++y) {
		for (size_t x = 7; x <= 10; ++x) {
			grid[x, y] = floor;
		}
		for (size_t x = 43; x <= 46; ++x) {
			grid[x, y] = floor;
		}
		for (size_t x = 69; x <= 72; ++x) {
			grid[x, y] = floor;
		}
	}

	DrawBuilding(grid, wall, floor, 2, 19, 15, 28, {{9, 19}, {15, 24}, {9, 28}});
	DrawBuilding(grid, wall, floor, 19, 19, 32, 27, {{26, 19}, {19, 23}});
	DrawBuilding(grid, wall, floor, 36, 19, 57, 30,
							 {{47, 19}, {36, 24}, {47, 30}, {57, 24}});

	world.AddQuarantineZone(
			Box::FromCorners(Point(demo::kQX1, demo::kQY1),
											 Point(demo::kQX2, demo::kQY2)));

	world.SetTransmissionRate(0.50);
	world.SetInfectionRadius(2.5);
	world.SetTreatmentDuration(40);
	world.SetImmunityDuration(50);
	world.SetFallbackRecoveryTicks(100);
	world.SetClinicEntrance(WorldPosition{82, 24});
	world.SetRecoveryExit(WorldPosition{70, 17});
}

void SeedResidents(InfectiousWorld& world) {
	auto add_pacer = [&](WorldPosition pos) -> ScriptedAgent<DiseaseData>& {
		return world.AddAgent<ScriptedAgent<DiseaseData>>(DiseaseData{pos});
	};

	add_pacer(WorldPosition{7, 6});
	add_pacer(WorldPosition{20, 7});
	add_pacer(WorldPosition{37, 6});
	add_pacer(WorldPosition{5, 24});
	add_pacer(WorldPosition{22, 23});
	add_pacer(WorldPosition{52, 6});
}

void SeedSwarmers(InfectiousWorld& world) {
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{14, 12}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{44, 12}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{68, 12}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{16, 17}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{44, 17}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{62, 17}});
	world.AddAgent<SwarmingAgent<DiseaseData>>(DiseaseData{WorldPosition{47, 19}});
}

void RunSimulation(InfectiousWorld& world, std::size_t max_agents, std::size_t ticks) {
	world.InfectAgent(demo::kNumPacers);

	for (std::size_t tick = 0; tick < ticks; ++tick) {
		if (tick > 0 && tick % demo::kSpawnInterval == 0 && world.GetNumAgents() < max_agents) {
			const std::size_t src = (tick / demo::kSpawnInterval) % demo::kSpawnSources;
			world.AddAgent<SwarmingAgent<DiseaseData>>(
					DiseaseData{WorldPosition{demo::kSpawnX[src], demo::kSpawnY[src]}});
			world.InfectAgent(world.GetNumAgents() - 1);
		}

		world.RunAgents();
		world.UpdateWorld();
	}
}

}  // namespace

int main() {
	cse498::benchmarking::BenchRunner runner{};
	const std::string benchmark_id = "group13_infectious_world_logic";

	const auto registration = runner.Register(benchmark_id, [](cse498::benchmarking::Params& params) {
		if (cse498::benchmarking::BENCH_START() != cse498::benchmarking::MetricCollectorError::Ok) {
			return;
		}

		InfectiousWorld world(demo::kGridW, demo::kGridH);
		ConfigureCampus(world);
		SeedResidents(world);
		SeedSwarmers(world);
		RunSimulation(world, params.agent_count, params.tick_count);

		(void)cse498::benchmarking::BENCH_STOP();
	});

	if (registration != cse498::benchmarking::RegistrationResult::Inserted) {
		std::cerr << "Failed to register benchmark" << std::endl;
		return 1;
	}

	cse498::benchmarking::Params params{};
	params.benchmark_name = benchmark_id;
	params.agent_count = demo::kMaxAgents;
	params.tick_count = demo::kSimTicks;
	params.warmup_ticks = 1;
	params.repetitions = 10;
	params.seed = 42;

	const auto run_result = runner.RunBenchmarkAndWriteReport(
			benchmark_id,
			params,
			"Group13TrafficWorld",
			"../benchmarking/groups/results",
			cse498::benchmarking::ReportFormat::Csv);

	if (run_result.status != cse498::benchmarking::RunAndReportStatus::Ok) {
		std::cerr << "Benchmark run or report write failed" << std::endl;
		return 1;
	}

	std::cout << "Benchmark completed: " << benchmark_id << std::endl;
	std::cout << "Report written to: " << run_result.output_path.string() << std::endl;
	return 0;
}


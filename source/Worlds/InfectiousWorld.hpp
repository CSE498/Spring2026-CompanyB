/**
 * InfectiousWorld.hpp
 * @brief A grid-based world that simulates infectious disease spread.
 * @note Extends SimWorldBase<DiseaseData> which in turn extends
 *       StepWorldBase<DiseaseData>. Health state is stored inside each agent's
 *       DiseaseData, so no separate health_map is needed.
 **/

#pragma once

#include <cassert>
#include <functional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "../core/DiseaseData.hpp"
#include "../core/Step.hpp"
#include "../tools/Box.hpp" // quarantine zones with restricted movement
#include "../tools/Circle.hpp" //infection radius around each agent
#include "../tools/Point.hpp" // continuous agent positions for geometry calculations
#include "../tools/Surface.hpp" // efficient overlap detection for infection spread
#include "SimWorldBase.hpp"

namespace cse498 {

class InfectiousWorld : public SimWorldBase<DiseaseData> {
  using Base = SimWorldBase<DiseaseData>;
  using AgentPtr = std::shared_ptr<StepAgentBase<DiseaseData>>;

 private:
  // Geometry infrastructure
  Surface surface;
  std::vector<Box> quarantine_zones;
  std::unordered_set<Surface::ShapeID> quarantine_surface_ids;

  // Per-agent surface tracking: one ShapeID per agent, indexed to match agent_set
  std::vector<Surface::ShapeID> agent_surface_ids;
  std::unordered_map<Surface::ShapeID, size_t> surface_to_agent_idx;

  //  observer called at the end of every UpdateWorld tick.
  std::function<void(const InfectiousWorld&)> tick_observer;

  // Disease parameters
  double transmission_rate = 0.3;
  double infection_radius = 1.5;
  size_t infection_duration = 10;
  size_t immunity_duration = 0;

  mutable std::mt19937 rng{
      std::random_device{}()};  // Maybe use random.hpp from group 19

  // -- Private helpers --

  /// Convert a WorldPosition to the centre of its grid cell as a Point.
  [[nodiscard]] static Point PosToPoint(WorldPosition pos) {
    return Point(pos.X() + 0.5, pos.Y() + 0.5);
  }

  /// Called by AddAgent() before the agent is pushed into agent_set.
 
  void ConfigAgent(StepAgentBase<DiseaseData>& agent) override {
    size_t idx = agent.GetId();
    Surface::ShapeID sid =
        surface.AddCircle(Circle(PosToPoint(agent.GetState().position), 0.0));
   
    if (idx >= agent_surface_ids.size())
      agent_surface_ids.resize(idx + 1);
    agent_surface_ids[idx] = sid;
    surface_to_agent_idx[sid] = idx;
  }

 public:
  
  [[nodiscard]] bool IsInQuarantine(WorldPosition pos) const {
    auto hits = surface.QueryRadius(PosToPoint(pos), 0.0);
    return std::ranges::any_of(hits, [&](Surface::ShapeID id) {
      return quarantine_surface_ids.contains(id);
    });
  }

 private:


  ///  Sync all agent point-circles to their current positions (O(n)).
  ///  Each infected agent calls QueryRadius — Surface sector partitioning
  ///         means only nearby agents are examined, not the full set.
  void SpreadInfection() {
    // Sync every agent's registered circle to its current position
    for (size_t i = 0; i < agent_set.size(); ++i) {
      surface.UpdateCircle(
          agent_surface_ids[i],
          Circle(PosToPoint(agent_set[i]->GetState().position), 0.0));
    }

    std::uniform_real_distribution<double> roll(0.0, 1.0);
    std::unordered_set<size_t> newly_infected;

    for (size_t i = 0; i < agent_set.size(); ++i) {
      DiseaseData src = agent_set[i]->GetState();
      if (src.health != HealthState::INFECTED) continue;

      // QueryRadius returns ShapeIDs of all shapes within infection_radius.
      // This includes quarantine box IDs and other agents — filter both.
      auto nearby = surface.QueryRadius(
          PosToPoint(src.position), infection_radius);

      for (Surface::ShapeID sid : nearby) {
        if (quarantine_surface_ids.contains(sid)) continue;  // skip zone shapes
        auto it = surface_to_agent_idx.find(sid);
        if (it == surface_to_agent_idx.end()) continue;
        size_t j = it->second;
        if (j == i) continue;  // skip self

        DiseaseData tgt = agent_set[j]->GetState();
        if (tgt.health != HealthState::SUSCEPTIBLE) continue;
        if (!newly_infected.contains(j) && roll(rng) < transmission_rate) {
          newly_infected.insert(j);
        }
      }
    }

    for (size_t id : newly_infected) {
      DiseaseData state = agent_set[id]->GetState();
      state.health = HealthState::INFECTED;
      state.ticks_in_state = 0;
      agent_set[id]->SetState(state);
    }
  }

  /// Advance each agent's disease timer and transition states.
  void UpdateHealthTimers() {
    for (auto& agent_ptr : agent_set) {
      DiseaseData state = agent_ptr->GetState();
      state.ticks_in_state++;

      if (state.health == HealthState::INFECTED &&
          state.ticks_in_state >= infection_duration) {
        state.health = HealthState::RECOVERED;
        state.ticks_in_state = 0;
      } else if (state.health == HealthState::RECOVERED &&
                 immunity_duration > 0 &&
                 state.ticks_in_state >= immunity_duration) {
        state.health = HealthState::SUSCEPTIBLE;
        state.ticks_in_state = 0;
      }

      agent_ptr->SetState(state);
    }
  }

 public:
  InfectiousWorld(size_t width = 20, size_t height = 15) {
    main_grid.Resize(width, height, floor_id);
  }
  ~InfectiousWorld() override = default;

  /// Process one agent's turn: walk its StepContainer and execute
  /// MovementSteps against the grid. Returns the updated DiseaseData.
  DiseaseData DoAction(AgentPtr agent) override {
    using namespace cse498::steps;

    DiseaseData state = agent->GetState();
    StepContainer turn = agent->GetTurn();

    while (!turn.exhausted()) {
      auto step_result = turn.get_next();
      if (!step_result.has_value()) break;

      std::visit(
          [&](auto&& step) {
            using S = std::decay_t<decltype(step)>;

            if constexpr (std::is_same_v<S, MovementStep>) {
              WorldPosition new_pos = step.loc;
              if (main_grid.IsValid(new_pos) &&
                  main_grid[new_pos] != wall_id &&
                  !IsInQuarantine(new_pos)) {
                state.position = new_pos;
              }
            } else if constexpr (std::is_same_v<S, InfoStep>) {
              // Respond to LOC_AVAIL queries so agents can use ConditionalSteps
              if (step.aspect == InfoStep::Aspect::LOC_AVAIL) {
                WorldPosition target = step.target;
                bool avail = main_grid.IsValid(target) &&
                             main_grid[target] != wall_id &&
                             !IsInQuarantine(target);
                turn.inform(avail);
              }
            }
            // ConditionalStep / ReconStep are handled by StepContainer itself
          },
          step_result.value());
    }

    return state;
  }

  /// Each tick: advance health timers, then spread infection.
  /// Running timers first ensures agents infected this tick have
  /// ticks_in_state == 0 until the next UpdateWorld(), preventing
  /// same-tick recovery with infection_duration == 1.
  void UpdateWorld() override {
    tick_count++;
    UpdateHealthTimers();
    SpreadInfection();
    if (tick_observer) tick_observer(*this);
  }

  /// Register a callback invoked at the end of every tick.
  void RegisterTickObserver(std::function<void(const InfectiousWorld&)> observer) {
    tick_observer = std::move(observer);
  }

  // -- Infection control --

  /// Manually infect an agent. Throws std::out_of_range for invalid \p id.
  void InfectAgent(size_t id) {
    if (id >= agent_set.size())
      throw std::out_of_range("InfectAgent: agent id out of range");
    DiseaseData state = agent_set[id]->GetState();
    state.health = HealthState::INFECTED;
    state.ticks_in_state = 0;
    agent_set[id]->SetState(state);
  }

  /// @return Current health state; SUSCEPTIBLE if \p id is out of range.
  [[nodiscard]] HealthState GetAgentHealth(size_t id) const {
    if (id >= agent_set.size()) return HealthState::SUSCEPTIBLE;
    return agent_set[id]->GetState().health;
  }

  [[nodiscard]] bool IsAgentInfected(size_t id) const {
    return GetAgentHealth(id) == HealthState::INFECTED;
  }

  // -- Quarantine zones (Box) --

  void AddQuarantineZone(const Box& zone) {
    quarantine_zones.push_back(zone);
    quarantine_surface_ids.insert(surface.AddBox(zone));
  }

  void ClearQuarantineZones() {
    for (auto id : quarantine_surface_ids) surface.RemoveShape(id);
    quarantine_surface_ids.clear();
    quarantine_zones.clear();
  }

  [[nodiscard]] const std::vector<Box>& GetQuarantineZones() const {
    return quarantine_zones;
  }

  // -- Statistics --

  [[nodiscard]] size_t GetInfectedCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::INFECTED;
        }));
  }

  [[nodiscard]] size_t GetRecoveredCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::RECOVERED;
        }));
  }

  [[nodiscard]] size_t GetSusceptibleCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::SUSCEPTIBLE;
        }));
  }

  // -- Configuration --

  void SetTransmissionRate(double rate) { transmission_rate = rate; }
  void SetInfectionRadius(double r) { infection_radius = r; }
  void SetInfectionDuration(size_t ticks) { infection_duration = ticks; }
  void SetImmunityDuration(size_t ticks) { immunity_duration = ticks; }

  [[nodiscard]] double GetTransmissionRate() const { return transmission_rate; }
  [[nodiscard]] double GetInfectionRadius() const { return infection_radius; }
  [[nodiscard]] size_t GetInfectionDuration() const {
    return infection_duration;
  }
  [[nodiscard]] size_t GetImmunityDuration() const { return immunity_duration; }
};

}  // End of namespace cse498

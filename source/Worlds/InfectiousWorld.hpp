/**
 * InfectiousWorld.hpp
 * @brief A grid-based world that simulates infectious disease spread.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <random>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../tools/Box.hpp"     // quarantine zones with restricted movement
#include "../tools/Circle.hpp"  //infection radius around each agent
#include "../tools/Point.hpp"  // continuous agent positions for geometry calculations
#include "../tools/Surface.hpp"  // efficient overlap detection for infection spread
#include "SimWorldBase.hpp"

namespace cse498 {

class InfectiousWorld : public SimWorldBase {
 public:
  enum class HealthState { SUSCEPTIBLE, INFECTED, RECOVERED };

 private:
  /// Per-agent infection tracking.
  struct AgentHealth {
    HealthState state{HealthState::SUSCEPTIBLE};
    size_t ticks_in_state{0};
  };

  std::unordered_map<size_t, AgentHealth> health_map;

  // Geometry infrastructure
  Surface surface;
  std::vector<Box> quarantine_zones;  // rectangular restricted areas.

  // Disease parameters
  double transmission_rate = 0.3;
  double infection_radius = 1.5;
  size_t infection_duration = 10;
  size_t immunity_duration = 0;

  mutable std::mt19937 rng{
      std::random_device{}()};  // Maybe use random.hpp from group 19

  /// Register new agents into the health tracking system.
  void ConfigAgent(AgentBase& agent) override {
    SimWorldBase::ConfigAgent(agent);
    health_map[agent.GetID()] = AgentHealth{};
  }

  /// Convert an agent's grid position to a Point for geometry operations.
  [[nodiscard]] Point AgentToPoint(size_t agent_id) const {
    WorldPosition pos = GetAgent(agent_id).GetLocation().AsWorldPosition();
    return Point(pos.X() + 0.5, pos.Y() + 0.5);  // center of cell
  }

  /// Build a Circle representing an infected agent's transmission zone.
  [[nodiscard]] Circle MakeInfectionZone(size_t agent_id) const {
    return Circle(AgentToPoint(agent_id), infection_radius);
  }

  /// Check if an agent is inside any quarantine zone.
  [[nodiscard]] bool IsInQuarantine(size_t agent_id) const {
    Point p = AgentToPoint(agent_id);
    for (const auto& zone : quarantine_zones) {
      if (zone.Contains(p)) return true;
    }
    return false;
  }

  /// Spread infection: infected agent's Circle overlaps with susceptible
  /// agent's position.
  void SpreadInfection() {
    std::uniform_real_distribution<double> roll(0.0, 1.0);
    std::unordered_set<size_t> newly_infected;

    for (const auto& [id, info] : health_map) {
      if (info.state != HealthState::INFECTED) continue;

      Circle zone = MakeInfectionZone(id);

      for (const auto& [other_id, other_info] : health_map) {
        if (other_id == id) continue;
        if (other_info.state != HealthState::SUSCEPTIBLE) continue;

        Point other_pos = AgentToPoint(other_id);
        if (zone.Contains(other_pos) && roll(rng) < transmission_rate) {
          newly_infected.insert(other_id);
        }
      }
    }

    for (size_t id : newly_infected) {
      health_map[id].state = HealthState::INFECTED;
      health_map[id].ticks_in_state = 0;
    }
  }

  /// Advance infection/immunity timers and transition health states.
  void UpdateHealthTimers() {
    for (auto& [id, info] : health_map) {
      info.ticks_in_state++;

      if (info.state == HealthState::INFECTED &&
          info.ticks_in_state >= infection_duration) {
        info.state = HealthState::RECOVERED;
        info.ticks_in_state = 0;
      } else if (info.state == HealthState::RECOVERED &&
                 immunity_duration > 0 &&
                 info.ticks_in_state >= immunity_duration) {
        info.state = HealthState::SUSCEPTIBLE;
        info.ticks_in_state = 0;
      }
    }
  }

 public:
  InfectiousWorld(size_t width = 20, size_t height = 15) {
    main_grid.Resize(width, height, floor_id);
  }
  ~InfectiousWorld() override = default;

  /// Allow agents to move. Quarantine zones block movement.
  int DoAction(AgentBase& agent, size_t action_id) override {
    // Save position in case quarantine blocks the move.
    WorldPosition old_pos = agent.GetLocation().AsWorldPosition();

    if (!MoveAgent(agent, action_id)) return 0;

    // If agent moved into a quarantine zone, push them back.
    if (!quarantine_zones.empty() && IsInQuarantine(agent.GetID())) {
      agent.SetLocation(old_pos);
      return 0;
    }

    return 1;
  }

  /// Each tick: advance timers, then spread infection so agents infected this
  /// tick keep ticks_in_state == 0 until the next UpdateWorld().
  void UpdateWorld() override {
    tick_count++;
    UpdateHealthTimers();
    SpreadInfection();
  }

  // -- Infection control --

  void InfectAgent(size_t agent_id) {
    health_map.at(agent_id).state = HealthState::INFECTED;
    health_map.at(agent_id).ticks_in_state = 0;
  }

  /// @return Current health state, or SUSCEPTIBLE if \p agent_id is not in the
  /// health map (no record / unknown id).
  [[nodiscard]] HealthState GetAgentHealth(size_t agent_id) const {
    auto it = health_map.find(agent_id);
    if (it == health_map.end()) return HealthState::SUSCEPTIBLE;
    return it->second.state;
  }

  [[nodiscard]] bool IsAgentInfected(size_t agent_id) const {
    return GetAgentHealth(agent_id) == HealthState::INFECTED;
  }

  // -- Quarantine zones (Box) --

  void AddQuarantineZone(const Box& zone) { quarantine_zones.push_back(zone); }

  void ClearQuarantineZones() { quarantine_zones.clear(); }

  [[nodiscard]] const std::vector<Box>& GetQuarantineZones() const {
    return quarantine_zones;
  }

  // -- Statistics --

  [[nodiscard]] size_t GetInfectedCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        health_map | std::views::values, [](const AgentHealth& h) {
          return h.state == HealthState::INFECTED;
        }));
  }

  [[nodiscard]] size_t GetRecoveredCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        health_map | std::views::values, [](const AgentHealth& h) {
          return h.state == HealthState::RECOVERED;
        }));
  }

  [[nodiscard]] size_t GetSusceptibleCount() const {
    return static_cast<size_t>(std::ranges::count_if(
        health_map | std::views::values, [](const AgentHealth& h) {
          return h.state == HealthState::SUSCEPTIBLE;
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

/**
 * @file InfectiousWorld.hpp
 * @brief Grid-based world that simulates infectious disease spread.
 *
 * InfectiousWorld combines StepWorldBase-style agent turns with Group 13
 * geometry tools. Agents move on a discrete WorldGrid, while Surface, Circle,
 * Box, and Point provide spatial queries for infection radius checks and
 * quarantine zones.
 *
 * @note Extends SimWorldBase<DiseaseData> which in turn extends
 *       StepWorldBase<DiseaseData>. Health state is stored inside each agent's
 *       DiseaseData, so no separate health map is needed.
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

#include "../core/AgentData.hpp"
#include "../core/Step.hpp"
#include "../tools/Box.hpp"      // quarantine zones with restricted movement
#include "../tools/Circle.hpp"   // infection radius around each agent
#include "../tools/Point.hpp"    // continuous positions for geometry queries
#include "../tools/Surface.hpp"  // overlap detection for infection spread
#include "SimWorldBase.hpp"

namespace cse498 {

/**
 * @brief Step-based infectious disease simulation world.
 *
 * The world owns a grid, a set of StepAgentBase<DiseaseData> agents, optional
 * quarantine/treatment zones, and infection parameters. Each UpdateWorld() tick
 * advances health timers, applies proximity-based infection spread, and
 * notifies an optional observer.
 */
class InfectiousWorld : public SimWorldBase<DiseaseData> {
  using Base = SimWorldBase<DiseaseData>;
  using AgentPtr = std::shared_ptr<StepAgentBase<DiseaseData>>;

 private:
  /// Broad-phase geometry registry for agents and quarantine zones.
  Surface surface;

  /// Stored quarantine zones in world coordinates.
  std::vector<Box> quarantine_zones;

  /// Surface shape IDs corresponding to quarantine_zones.
  std::unordered_set<Surface::ShapeID> quarantine_surface_ids;

  /// Per-agent Surface circle IDs indexed to match agent_set.
  std::vector<Surface::ShapeID> agent_surface_ids;

  /// Reverse lookup from Surface circle ID to agent_set index.
  std::unordered_map<Surface::ShapeID, size_t> surface_to_agent_idx;

  /// Observer called at the end of every UpdateWorld tick.
  std::function<void(const InfectiousWorld&)> tick_observer;

  /// Probability that a susceptible nearby agent becomes infected per tick.
  double transmission_rate = 0.3;

  /// Radius around an infected agent used for proximity infection checks.
  double infection_radius = 1.5;

  /// Reserved disease duration parameter for future recovery policies.
  size_t infection_duration = 10;

  /// Number of recovered ticks before immunity expires; zero means permanent.
  size_t immunity_duration = 0;

  /// Number of quarantine ticks required before an infected agent recovers.
  size_t treatment_duration = 5;

  /// Destination assigned to infected agents when a clinic is configured.
  WorldPosition clinic_entrance{};

  /// Whether clinic_entrance has been configured.
  bool has_clinic{false};

  /// Destination assigned after quarantine recovery when configured.
  WorldPosition recovery_exit{};

  /// Whether recovery_exit has been configured.
  bool has_recovery_exit{false};

  /// If > 0, infected agents that have never reached the clinic recover.
  size_t fallback_recovery_ticks{0};

  /// Random generator used for infection transmission rolls.
  mutable std::mt19937 rng{
      std::random_device{}()};  // Maybe use random.hpp from group 19

  /**
   * @brief Convert a grid position to the center point of its cell.
   * @param pos Discrete grid position.
   * @return Continuous point at the cell center.
   */
  [[nodiscard]] static Point PosToPoint(WorldPosition pos) {
    return Point(pos.X() + 0.5, pos.Y() + 0.5);
  }

  /**
   * @brief Register a newly added agent with the Surface geometry index.
   *
   * Called by AddAgent() before the agent is pushed into agent_set. Each agent
   * is represented as a zero-radius circle for proximity queries.
   *
   * @param agent Agent being configured.
   */
  void ConfigAgent(StepAgentBase<DiseaseData>& agent) override {
    size_t idx = agent.GetId();
    Surface::ShapeID sid =
        surface.AddCircle(Circle(PosToPoint(agent.GetState().position), 0.0));

    if (idx >= agent_surface_ids.size()) agent_surface_ids.resize(idx + 1);
    agent_surface_ids[idx] = sid;
    surface_to_agent_idx[sid] = idx;
  }

 public:
  /**
   * @brief Check whether a position overlaps any quarantine zone.
   * @param pos Grid position to test.
   * @return true if the position lies inside a registered quarantine zone.
   */
  [[nodiscard]] bool IsInQuarantine(WorldPosition pos) const {
    auto hits = surface.QueryRadius(PosToPoint(pos), 0.0);
    return std::ranges::any_of(hits, [&](Surface::ShapeID id) {
      return quarantine_surface_ids.contains(id);
    });
  }

 private:
  /**
   * @brief Apply proximity-based infection spread for the current tick.
   *
   * Agent circles are first synchronized to their current grid positions.
   * Infected agents then query nearby surface entries and may infect
   * susceptible agents within infection_radius according to transmission_rate.
   */
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
      auto nearby =
          surface.QueryRadius(PosToPoint(src.position), infection_radius);

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
      state.quarantine_ticks = 0;
      if (has_clinic) state.destination = clinic_entrance;
      agent_set[id]->SetState(state);
    }
  }

  /**
   * @brief Advance disease, treatment, fallback recovery, and immunity timers.
   */
  void UpdateHealthTimers() {
    for (auto& agent_ptr : agent_set) {
      DiseaseData state = agent_ptr->GetState();

      if (state.health == HealthState::INFECTED) {
        // Always count total infection ticks (drives fallback recovery).
        state.ticks_in_state++;

        if (IsInQuarantine(state.position)) {
          // Count time specifically inside the clinic zone.
          state.quarantine_ticks++;
          if (state.quarantine_ticks >= treatment_duration) {
            state.health = HealthState::RECOVERED;
            state.ticks_in_state = 0;
            state.quarantine_ticks = 0;
            state.destination =
                has_recovery_exit ? std::optional<WorldPosition>{recovery_exit}
                                  : std::optional<WorldPosition>{};
          }
        } else if (fallback_recovery_ticks > 0 &&
                   state.ticks_in_state >= fallback_recovery_ticks) {
          // Fallback: agent never reached Olin — recover in place.
          state.health = HealthState::RECOVERED;
          state.ticks_in_state = 0;
          state.quarantine_ticks = 0;
          state.destination = {};
        }

      } else if (state.health == HealthState::RECOVERED) {
        // Clear destination once the agent reaches the recovery exit.
        if (state.destination.has_value() &&
            state.position == state.destination.value()) {
          state.destination = {};
        }
        // Immunity waning countdown.
        if (immunity_duration > 0) {
          state.ticks_in_state++;
          if (state.ticks_in_state >= immunity_duration) {
            state.health = HealthState::SUSCEPTIBLE;
            state.ticks_in_state = 0;
          }
        }
      }

      agent_ptr->SetState(state);
    }
  }

 public:
  /**
   * @brief Construct an infectious world with a rectangular grid.
   * @param width Grid width in cells.
   * @param height Grid height in cells.
   * @param script Input script for ScriptedAgents, defaults to none.
   */
  InfectiousWorld(size_t width = 20, size_t height = 15,
                  std::string const& script = "") {
    main_grid.Resize(width, height, floor_id);
    SetupScriptedAgents(script);
  }

  /**
   * @brief Default virtual destructor.
   */
  ~InfectiousWorld() override = default;

  /**
   * @brief Execute one agent turn and return its updated state.
   *
   * MovementSteps are applied only when the target cell is inside the grid, not
   * a wall, not occupied by another agent, and allowed by quarantine rules.
   * Susceptible and recovered agents cannot enter quarantine zones. Infected
   * agents cannot leave quarantine zones until treatment has recovered them.
   * InfoSteps for LOC_AVAIL apply the same checks so ConditionalSteps can
   * choose primary or backup movement.
   *
   * @param agent Agent whose turn should be processed.
   * @return Updated DiseaseData after processing the StepContainer.
   */
  DiseaseData DoAction(AgentPtr agent) override {
    using namespace cse498::steps;

    // I switched the order of these because for scripted agents
    // its crucial that GetTurn() is called before GetState()
    StepContainer turn = agent->GetTurn();
    DiseaseData state = agent->GetState();

    while (!turn.exhausted()) {
      auto step_result = turn.get_next();
      if (!step_result.has_value()) break;

      std::visit(
          [&](auto&& step) {
            using S = std::decay_t<decltype(step)>;

            // Returns true if any other agent already occupies the given cell.
            auto cell_occupied = [&](WorldPosition pos) {
              for (const auto& a : agent_set) {
                if (a == agent) continue;
                WorldPosition ap = a->GetState().position;
                if (ap.CellX() == pos.CellX() && ap.CellY() == pos.CellY())
                  return true;
              }
              return false;
            };

            if constexpr (std::is_same_v<S, MovementStep>) {
              WorldPosition new_pos = step.loc;
              WorldPosition old_pos = state.position;
              // Non-infected agents cannot enter the quarantine zone.
              bool entering_qtn =
                  !IsInQuarantine(old_pos) && IsInQuarantine(new_pos);
              bool entry_blocked =
                  entering_qtn && (state.health != HealthState::INFECTED);
              // Any infected agent, scripted or swarming, cannot leave until
              // treatment is complete (quarantine_ticks >= treatment_duration).
              bool leaving_qtn =
                  IsInQuarantine(old_pos) && !IsInQuarantine(new_pos);
              bool exit_blocked =
                  leaving_qtn && (state.health == HealthState::INFECTED);
              if (main_grid.IsValid(new_pos) && main_grid[new_pos] != wall_id &&
                  !entry_blocked && !exit_blocked && !cell_occupied(new_pos)) {
                state.position = new_pos;
              }
            } else if constexpr (std::is_same_v<S, InfoStep>) {
              // Respond to LOC_AVAIL queries so agents can use ConditionalSteps
              if (step.aspect == InfoStep::Aspect::LOC_AVAIL) {
                WorldPosition target = step.target;
                bool entering_qtn =
                    !IsInQuarantine(state.position) && IsInQuarantine(target);
                bool entry_blocked =
                    entering_qtn && (state.health != HealthState::INFECTED);
                bool leaving_qtn =
                    IsInQuarantine(state.position) && !IsInQuarantine(target);
                bool exit_blocked =
                    leaving_qtn && (state.health == HealthState::INFECTED);
                bool avail = main_grid.IsValid(target) &&
                             main_grid[target] != wall_id && !entry_blocked &&
                             !exit_blocked && !cell_occupied(target);
                turn.inform(avail);
              }
            }
            // ConditionalStep / ReconStep are handled by StepContainer itself
          },
          step_result.value());
    }

    return state;
  }

  /**
   * @brief Advance the infectious world by one tick.
   *
   * Timers run before spread so agents infected during this tick keep
   * ticks_in_state == 0 until the next UpdateWorld() call. The optional
   * observer is called after health and spread updates finish.
   */
  void UpdateWorld() override {
    RunAgents();
    tick_count++;
    UpdateHealthTimers();
    SpreadInfection();
    if (tick_observer) tick_observer(*this);
  }

  /**
   * @brief Register a callback invoked at the end of every tick.
   * @param observer Function receiving this world after each UpdateWorld().
   */
  void RegisterTickObserver(
      std::function<void(const InfectiousWorld&)> observer) {
    tick_observer = std::move(observer);
  }

  /**
   * @brief Manually infect an agent by ID.
   * @param id Agent index in agent_set.
   * @throws std::out_of_range if id does not refer to an existing agent.
   */
  void InfectAgent(size_t id) {
    if (id >= agent_set.size())
      throw std::out_of_range("InfectAgent: agent id out of range");
    DiseaseData state = agent_set[id]->GetState();
    state.health = HealthState::INFECTED;
    state.ticks_in_state = 0;
    state.quarantine_ticks = 0;
    if (has_clinic) state.destination = clinic_entrance;
    agent_set[id]->SetState(state);
  }

  /**
   * @brief Get an agent's health state.
   * @param id Agent index in agent_set.
   * @return Current health state, or SUSCEPTIBLE if id is out of range.
   */
  [[nodiscard]] HealthState GetAgentHealth(size_t id) const {
    if (id >= agent_set.size()) return HealthState::SUSCEPTIBLE;
    return agent_set[id]->GetState().health;
  }

  /**
   * @brief Check whether an agent is currently infected.
   * @param id Agent index in agent_set.
   * @return true if the agent exists and is infected.
   */
  [[nodiscard]] bool IsAgentInfected(size_t id) const {
    return GetAgentHealth(id) == HealthState::INFECTED;
  }

  /**
   * @brief Add a rectangular quarantine/treatment zone.
   * @param zone Axis-aligned box in grid-cell coordinates.
   */
  void AddQuarantineZone(const Box& zone) {
    quarantine_zones.push_back(zone);
    quarantine_surface_ids.insert(surface.AddBox(zone));
  }

  /**
   * @brief Remove all quarantine zones from the world and Surface registry.
   */
  void ClearQuarantineZones() {
    for (auto id : quarantine_surface_ids) surface.RemoveShape(id);
    quarantine_surface_ids.clear();
    quarantine_zones.clear();
  }

  void ClearAgents() {
    for (auto sid : agent_surface_ids) {
      surface.RemoveShape(sid);
    }
    agent_surface_ids.clear();
    surface_to_agent_idx.clear();
    Base::ClearAgents();
  }
  /**
   * @brief Access registered quarantine zones.
   * @return Const reference to the stored quarantine boxes.
   */
  [[nodiscard]] const std::vector<Box>& GetQuarantineZones() const {
    return quarantine_zones;
  }

  /**
   * @brief Count currently infected agents.
   * @return Number of agents in HealthState::INFECTED.
   */
  [[nodiscard]] size_t GetInfectedCount() const {
    return static_cast<size_t>(
        std::ranges::count_if(agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::INFECTED;
        }));
  }

  /**
   * @brief Count currently recovered agents.
   * @return Number of agents in HealthState::RECOVERED.
   */
  [[nodiscard]] size_t GetRecoveredCount() const {
    return static_cast<size_t>(
        std::ranges::count_if(agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::RECOVERED;
        }));
  }

  /**
   * @brief Count currently susceptible agents.
   * @return Number of agents in HealthState::SUSCEPTIBLE.
   */
  [[nodiscard]] size_t GetSusceptibleCount() const {
    return static_cast<size_t>(
        std::ranges::count_if(agent_set, [](const auto& ptr) {
          return ptr->GetState().health == HealthState::SUSCEPTIBLE;
        }));
  }

  /**
   * @brief Set per-contact transmission probability.
   * @param rate Probability in the range expected by callers, usually [0, 1].
   */
  void SetTransmissionRate(double rate) { transmission_rate = rate; }

  /**
   * @brief Set radius used for infection proximity checks.
   * @param r Infection radius in grid-cell units.
   */
  void SetInfectionRadius(double r) { infection_radius = r; }

  /**
   * @brief Set reserved infection duration parameter.
   * @param ticks Number of ticks.
   */
  void SetInfectionDuration(size_t ticks) { infection_duration = ticks; }

  /**
   * @brief Set recovered immunity duration.
   * @param ticks Number of ticks before recovered agents become susceptible;
   * zero disables immunity expiration.
   */
  void SetImmunityDuration(size_t ticks) { immunity_duration = ticks; }

  /**
   * @brief Set treatment duration inside quarantine zones.
   * @param ticks Quarantine ticks required for recovery.
   */
  void SetTreatmentDuration(size_t ticks) { treatment_duration = ticks; }

  /**
   * @brief Set fallback recovery delay for infected agents outside treatment.
   * @param ticks Number of infected ticks before recovery; zero disables it.
   */
  void SetFallbackRecoveryTicks(size_t ticks) {
    fallback_recovery_ticks = ticks;
  }

  /**
   * @brief Configure the destination assigned to newly infected agents.
   * @param pos Clinic entrance position.
   */
  void SetClinicEntrance(WorldPosition pos) {
    clinic_entrance = pos;
    has_clinic = true;
  }

  /**
   * @brief Configure destination assigned after quarantine recovery.
   * @param pos Recovery exit position.
   */
  void SetRecoveryExit(WorldPosition pos) {
    recovery_exit = pos;
    has_recovery_exit = true;
  }

  /**
   * @brief Get per-contact transmission probability.
   * @return Current transmission_rate value.
   */
  [[nodiscard]] double GetTransmissionRate() const { return transmission_rate; }

  /**
   * @brief Get infection proximity radius.
   * @return Current infection_radius value.
   */
  [[nodiscard]] double GetInfectionRadius() const { return infection_radius; }

  /**
   * @brief Get configured infection duration.
   * @return Current infection_duration value.
   */
  [[nodiscard]] size_t GetInfectionDuration() const {
    return infection_duration;
  }

  /**
   * @brief Get recovered immunity duration.
   * @return Current immunity_duration value.
   */
  [[nodiscard]] size_t GetImmunityDuration() const { return immunity_duration; }
};

}  // End of namespace cse498

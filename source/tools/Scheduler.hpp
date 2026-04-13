/**
 * @file Scheduler.hpp
 * @brief Generic priority scheduler: deterministic (deficit round-robin) or
 *        probabilistic (weighted random), optional rebalancing and
 * failure/backoff.
 * @details Examples and behavior notes: see tests/tools/SchedulerTest.cpp.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>   // for std::size_t
#include <expected>  // for std::expected (error handling for return values)
#include <optional>  // for std::optional, to signal the presence or absence of a value
#include <random>  // for RNG, std::mt19937, std::uniform_real_distribution
#include <ranges>  // for views and std::transform
#include <string_view>
#include <unordered_map>
#include <utility>  // for std::pair,
#include <vector>

namespace cse498 {

/// Error codes for Scheduler operations
enum class SchedulerError {
  EmptyScheduler,
  ProcessNotFound,
  ProcessAlreadyExists,
  InvalidWeight,
  NegativeWeight,
  NoSchedulableProcesses,
  InvalidParameter,
  WeightBlockedByRebalancing,  ///< Cannot set base weight while rebalancing is
                               ///< enabled
  ZeroMaxFailures,
  InvalidBackoffMultiplier,  ///< Backoff multiplier must be >= 1.0
  ZeroRecoveryThreshold      ///< Recovery threshold must be > 0
};

// Convert SchedulerError to a readable string using a constexpr array of string
// views
constexpr std::array<std::string_view, 12> SchedulerErrorStrings = {
    "Scheduler has no processes",
    "Process not found in scheduler",
    "Process with this ID already exists",
    "Invalid weight value",
    "Weight must be non-negative",
    "No schedulable processes available",
    "Invalid configuration parameter",
    "Cannot set base weight while rebalancing is enabled; call "
    "EnableRebalancing(false) first",
    "Max consecutive failures must be greater than zero",
    "Backoff multiplier must be >= 1.0",
    "Recovery success threshold must be greater than zero",
    "Unknown error"};

constexpr const char* to_string(SchedulerError error) noexcept {
  std::size_t idx = static_cast<std::size_t>(error);
  return (idx < SchedulerErrorStrings.size() - 1)
             ? SchedulerErrorStrings[idx].data()
             : SchedulerErrorStrings.back().data();
}

// Weighted scheduler: DETERMINISTIC (deficit round-robin) or PROBABILISTIC.
// Optional rebalancing (adjusted weights) and optional failure/backoff.

template <typename ID_TYPE = size_t>
class Scheduler {
 public:
  /// Scheduling modes
  enum class Mode {
    DETERMINISTIC,  ///< Round-robin weighted
    PROBABILISTIC   ///< Random weighted
  };

  struct Config {
    // Rebalancing
    double weight_floor = 0.1;
    double weight_ceiling = 1000.0;
    double wait_boost_factor = 0.1;
    double frequency_penalty = 0.05;

    // Failure handling
    size_t max_consecutive_failures = 3;
    size_t initial_backoff_cycles = 1;
    double backoff_multiplier = 2.0;
    size_t max_backoff_cycles = 64;
    size_t recovery_success_threshold = 2;
  };

  static constexpr double MAX_FREQ_PENALTY = 1.0;

 private:
  /// Internal metadata for each scheduled process
  struct ProcessState {
    double base_weight{};  // Base priority weight

    // dynamically adjusted weight (used when rebalancing is enabled)
    double adjusted_weight{};

    // round-robin accumulator (goes negative after selection)
    double deficit{};

    size_t execution_count{};  // times this process has been scheduled
    size_t wait_cycles{};      // no of cycles since last execution
    size_t last_execution_cycle{};
    size_t insertion_order{};

    // Failure handling
    bool enabled{};               // Is this process currently enabled?
    size_t failure_count{};       // no of consecutive failures
    size_t success_count{};       // no of consecutive successes
    size_t total_failures{};      // Total failures across all time
    size_t backoff_cycles{};      // Cycles to wait before retry (exponential
                                  // backoff)
    size_t cycles_until_retry{};  // Countdown until process can be retried

    ProcessState(double weight, size_t order)
        : base_weight(weight),
          adjusted_weight(weight),
          deficit(weight),
          execution_count(0),
          wait_cycles(0),
          last_execution_cycle(0),
          insertion_order(order),
          enabled(true),
          failure_count(0),
          success_count(0),
          total_failures(0),
          backoff_cycles(0),
          cycles_until_retry(0) {
      assert(weight >= 0.0 && "Weight must be non-negative");
    }

    void ResetFailureState() {
      failure_count = 0;
      success_count = 0;
      backoff_cycles = 0;
      cycles_until_retry = 0;
    }
  };

  // Member Variables

  std::unordered_map<ID_TYPE, ProcessState> process_map;
  Mode scheduling_mode;
  size_t next_insertion_order{};

  // Random number generation for probabilistic mode
  // Remember to use Random.hpp from Group 19
  mutable std::mt19937 rng;

  // Whether to RebalanceWeights() after each GetNext()
  bool rebalance_enabled{};
  double weight_floor{};       // Lower bound for adjusted
  double weight_ceiling{};     // Upper bound for adjusted weights
  double wait_boost_factor{};  // Weight increase per wait cycle
  double frequency_penalty{};  // Weight reduction for frequent execution
  size_t scheduling_cycle{};   // Current scheduling cycle number

  bool failure_handling_enabled{};
  size_t max_consecutive_failures{};  // Max failures before disabling process
  size_t initial_backoff_cycles{};    // Initial backoff period after failure
  double backoff_multiplier{};        // Exponential backoff growth factor
  size_t max_backoff_cycles{};  // Limits exponential backoff after repeated
                                // failures to prevent excessively long delays.
  size_t recovery_success_threshold{};  // Successes needed to clear failure
                                        // count

  // function Methods

  /**
   * @brief Rebalance adjusted weights after a scheduling decision.
   * @param selected_id The process that was just selected.
   *
   * Called after each GetNext(). Does nothing if rebalancing is off.
   * - The selected process's weight is reduced (frequency penalty).
   * - All other processes' weights are increased (wait boost).
   * - All weights are clamped to [weight_floor, weight_ceiling].
   */
  void RebalanceWeights(const ID_TYPE& selected_id) {
    if (!rebalance_enabled) {
      return;
    }

    for (auto& [id, state] : process_map) {
      if (id == selected_id) {
        state.adjusted_weight *= (1.0 - frequency_penalty);
        state.last_execution_cycle = scheduling_cycle;
        state.wait_cycles = 0;
      } else {
        ++state.wait_cycles;
        state.adjusted_weight += (wait_boost_factor * state.wait_cycles);
      }
      state.adjusted_weight =
          std::clamp(state.adjusted_weight, weight_floor, weight_ceiling);
    }
  }

  /// returns base_weight or adjusted_weight based on rebalancing state
  double GetEffectiveWeight(const ProcessState& state) const {
    return rebalance_enabled ? state.adjusted_weight : state.base_weight;
  }

  /// Check if a process is currently schedulable
  [[nodiscard]] bool IsSchedulable(const ProcessState& state) const {
    return state.enabled && state.cycles_until_retry == 0;
  }

  /// Update backoff counters for all processes
  /// Decrements cycles_until_retry for processes in backoff.
  /// Called at the start of each scheduling cycle.
  void UpdateBackoffCounters() {
    if (!failure_handling_enabled) return;

    std::ranges::for_each(process_map | std::views::values,
                          [](ProcessState& state) {
                            if (state.cycles_until_retry > 0) {
                              --state.cycles_until_retry;
                            }
                          });
  }

  /**
   * Uses Deficit Round-Robin algorithm to select the next process to execute
   * Algorithm:
   * @code
   *   for each schedulable process P:
   *       P.deficit += GetEffectiveWeight(P)
   *   selected = process with highest deficit (ties broken by insertion order)
   *   selected.deficit -= sum of all effective weights
   *   return selected
   * @endcode
   * Note the deficit for the selected process will be negative because
   * it is subtracted from the total weight of all processes, but eventually
   * climbs up by adding its effective weight (base or adjusted weight) to the
   * deficit, and it will be positive eventually.
   */
  [[nodiscard]] std::expected<ID_TYPE, SchedulerError> GetNextDeterministic() {
    if (process_map.empty()) {
      return std::unexpected(SchedulerError::EmptyScheduler);
    }

    // we dont need to store the whole state of the process
    struct BestCandidate {
      ID_TYPE id;
      double weight;
      size_t order;
    };

    std::optional<BestCandidate> best;
    for (auto& [id, state] : process_map) {
      if (!IsSchedulable(state)) continue;

      // Add the process's budget (effective weight) to its accumulated
      // deficit, allowing it to build up credit for selection if not chosen.
      state.deficit += GetEffectiveWeight(state);

      // chooses process with highest deficit(most credit)
      // breaks ties using insertion order
      bool is_better = !best || (state.deficit > best->weight) ||
                       (state.deficit == best->weight &&
                        state.insertion_order < best->order);

      if (is_better) {
        best = BestCandidate{id, state.deficit, state.insertion_order};
      }
    }

    if (!best) {
      return std::unexpected(SchedulerError::NoSchedulableProcesses);
    }

    // Subtract the total weight from the selected process's deficit.
    // This prevents a process from running excessively more often than its
    // share.
    process_map.find(best->id)->second.deficit -= GetTotalWeight();

    return best->id;
  }

  /**
   * @brief Select next process using probabilistic weighted random selection
   * @return ID of the process to execute next
   *
   * Algorithm:
   * @code
   *   total_weight = sum of effective weights for schedulable processes
   *   r = random number in [0, total_weight)
   *   cumulative = 0
   *   for each schedulable process P:
   *       cumulative += GetEffectiveWeight(P)
   *       if r < cumulative: return P
   * @endcode
   */
  std::expected<ID_TYPE, SchedulerError> GetNextProbabilistic() const {
    if (process_map.empty()) {
      return std::unexpected(SchedulerError::EmptyScheduler);
    }

    double total_weight = GetTotalWeight();
    if (total_weight <= 0.0) {
      return std::unexpected(SchedulerError::NoSchedulableProcesses);
    }

    // Draw to determine where we land on the weighted process interval
    std::uniform_real_distribution<double> dist(0.0, total_weight);
    double random_value = dist(rng);

    double cumulative = 0.0;
    std::optional<ID_TYPE> selected_id;

    // selects process whose weight interval contains random_value
    for (const auto& [id, state] : process_map) {
      if (!IsSchedulable(state)) continue;

      cumulative +=
          GetEffectiveWeight(state);  // move to the next process's interval
      // checks if interval contains random_value
      if (random_value < cumulative) {
        selected_id = id;
        break;
      }
    }

    if (!selected_id) {
      return std::unexpected(SchedulerError::NoSchedulableProcesses);
    }

    return *selected_id;
  }

 public:
  // Constructor
  explicit Scheduler(Mode mode = Mode::DETERMINISTIC,
                     unsigned int seed = std::random_device{}(),
                     Config cfg = {})
      : scheduling_mode(mode),
        next_insertion_order(0),
        rng(seed),
        rebalance_enabled(false),
        weight_floor(cfg.weight_floor),
        weight_ceiling(cfg.weight_ceiling),
        wait_boost_factor(cfg.wait_boost_factor),
        frequency_penalty(cfg.frequency_penalty),
        scheduling_cycle(0),
        failure_handling_enabled(false),
        max_consecutive_failures(cfg.max_consecutive_failures),
        initial_backoff_cycles(cfg.initial_backoff_cycles),
        backoff_multiplier(cfg.backoff_multiplier),
        max_backoff_cycles(cfg.max_backoff_cycles),
        recovery_success_threshold(cfg.recovery_success_threshold) {}

  // Core API

  ///  weight must be non-negative and finite
  [[nodiscard]] std::expected<void, SchedulerError> AddProcess(ID_TYPE id,
                                                               double weight) {
    if (!std::isfinite(weight)) {
      return std::unexpected(SchedulerError::InvalidWeight);
    }
    if (weight < 0.0) {
      return std::unexpected(SchedulerError::NegativeWeight);
    }

    if (process_map.contains(id)) {
      return std::unexpected(SchedulerError::ProcessAlreadyExists);
    }

    process_map.emplace(id, ProcessState(weight, next_insertion_order++));
    return {};  // Success
  }

  /// remove a process from the scheduler
  [[nodiscard]] std::expected<void, SchedulerError> RemoveProcess(ID_TYPE id) {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    process_map.erase(it);
    return {};  // Success
  }

  /// resets scheduler state
  void Clear() noexcept {
    process_map.clear();
    next_insertion_order = 0;
    scheduling_cycle = 0;
  }

  [[nodiscard]] std::expected<ID_TYPE, SchedulerError> GetNext() {
    if (process_map.empty()) {
      return std::unexpected(SchedulerError::EmptyScheduler);
    }

    // Step 1: Advance the scheduler cycle and decrement cycles_until_retry for
    // any processes in backoff
    scheduling_cycle++;
    UpdateBackoffCounters();  // (UpdateBackoffCounters reduces
                              // cycles_until_retry for  failing processes to
                              // eventually make them schedulable again)

    // Step 2: Pick the next process using the current scheduling algorithm.
    std::expected<ID_TYPE, SchedulerError> selected =
        (scheduling_mode == Mode::DETERMINISTIC) ? GetNextDeterministic()
                                                 : GetNextProbabilistic();

    if (!selected) {
      return selected;
    }  // If there was an error, it returns that error to the caller.

    // Step 3: Record the selection and rebalance weights for next time.
    ID_TYPE selected_id = *selected;
    process_map.find(selected_id)->second.execution_count++;
    RebalanceWeights(selected_id);

    return selected_id;
  }

  // Configuration

  /// change the scheduling mode
  void SetMode(Mode mode) noexcept {
    if (mode == scheduling_mode) return;

    scheduling_mode = mode;

    // initialize each process's deficit to its current effective weight(either
    // base_weight or adjusted_weight), so the round-robin algorithm starts
    // fairly.
    // For probabilistic mode, no persistent state (like deficit)
    // is needed, so there's nothing to reset—selection is inherently
    // memoryless.
    if (mode == Mode::DETERMINISTIC) {
      for (auto& [id, state] : process_map) {
        state.deficit = GetEffectiveWeight(state);
      }
    }
  }

  /// get the current scheduling mode
  [[nodiscard]] Mode GetMode() const noexcept { return scheduling_mode; }

  // Query Methods

  /// check if scheduler has any processes
  [[nodiscard]] bool HasProcesses() const noexcept {
    return !process_map.empty();
  }

  /// get the number of processes currently scheduled
  [[nodiscard]] size_t GetProcessCount() const noexcept {
    return process_map.size();
  }

  /// check if a specific process exists in the scheduler
  [[nodiscard]] bool HasProcess(ID_TYPE id) const noexcept {
    return process_map.contains(id);
  }

  /// Get the base weight of a process
  [[nodiscard]] std::expected<double, SchedulerError> GetBaseWeight(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.base_weight;
  }

  // Get the highest effective weight among all schedulable processes
  // Returns adjusted weight when rebalancing is enabled, base weight otherwise.
  [[nodiscard]] std::expected<double, SchedulerError> GetHighestWeight() const {
    if (process_map.empty()) {
      return std::unexpected(SchedulerError::EmptyScheduler);
    }

    // Build a  pipeline: map values -> keep schedulable -> extract weight.
    auto schedulable_weights =
        process_map | std::views::values  // ProcessState stream
        | std::views::filter([this](const ProcessState& s) {
            return IsSchedulable(s);
          })  // drop disabled / backed-off
        | std::views::transform([this](const ProcessState& s) {
            return GetEffectiveWeight(s);
          });  // -> double

    // Find the maximum weight in the filtered range.
    auto it = std::ranges::max_element(schedulable_weights);
    if (it == schedulable_weights.end()) {
      return std::unexpected(SchedulerError::NoSchedulableProcesses);
    }
    return *it;
  }

  /// Get the sum of all effective weights for schedulable processes
  /// Returns adjusted weights when rebalancing is enabled, base weights
  /// otherwise.
  [[nodiscard]] double GetTotalWeight() const noexcept {
    auto schedulable_effective_weights =
        process_map | std::views::values |
        std::views::filter(
            [this](const ProcessState& s) { return IsSchedulable(s); }) |
        std::views::transform(
            [this](const ProcessState& s) { return GetEffectiveWeight(s); });
    return std::ranges::fold_left(schedulable_effective_weights, 0.0,
                                  std::plus<>{});
  }

  /// Get the number of times a process has been selected
  [[nodiscard]] std::expected<size_t, SchedulerError> GetProcessExecutionCount(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.execution_count;
  }

  //  Rebalancing API

  /**
   * @brief Enable or disable weight rebalancing after each scheduling decision.
   * @param enable true to rebalance weights each cycle, false to use static
   * base weights.
   *
   * When enabled, RebalanceWeights() runs after every GetNext() call:
   *  - The selected process's weight is reduced (frequency penalty).
   *  - Waiting processes' weights are boosted (wait boost).
   * Disabling resets all adjusted weights back to their base values.
   */
  void EnableRebalancing(bool enable) noexcept {
    rebalance_enabled = enable;
    if (!enable) {
      for (auto& [id, state] : process_map) {
        state.adjusted_weight = state.base_weight;
        state.wait_cycles = 0;
      }
    }
  }

  /// checks if weight rebalancing is enabled
  [[nodiscard]] bool IsRebalancingEnabled() const noexcept {
    return rebalance_enabled;
  }

  /// Set the weight floor for rebalancing must be non-negative and finite
  [[nodiscard]] std::expected<void, SchedulerError> SetWeightFloor(
      double floor) {
    if (!std::isfinite(floor))
      return std::unexpected(SchedulerError::InvalidWeight);
    if (floor < 0.0) return std::unexpected(SchedulerError::NegativeWeight);
    if (floor > weight_ceiling)
      return std::unexpected(SchedulerError::InvalidParameter);
    weight_floor = floor;
    return {};
  }

  [[nodiscard]] double GetWeightFloor() const noexcept { return weight_floor; }

  /// Set the weight ceiling for rebalancing must be positive and finite
  [[nodiscard]] std::expected<void, SchedulerError> SetWeightCeiling(
      double ceiling) {
    if (!std::isfinite(ceiling) || ceiling <= 0.0)
      return std::unexpected(SchedulerError::InvalidParameter);
    if (ceiling < weight_floor)
      return std::unexpected(SchedulerError::InvalidParameter);
    weight_ceiling = ceiling;
    return {};
  }

  [[nodiscard]] double GetWeightCeiling() const noexcept {
    return weight_ceiling;
  }

  /// Set the wait boost factor (weight increase per wait cycle)
  [[nodiscard]] std::expected<void, SchedulerError> SetWaitBoostFactor(
      const double factor) {
    if (!std::isfinite(factor))
      return std::unexpected(SchedulerError::InvalidParameter);
    if (factor < 0.0) return std::unexpected(SchedulerError::InvalidParameter);
    wait_boost_factor = factor;
    return {};
  }

  [[nodiscard]] double GetWaitBoostFactor() const noexcept {
    return wait_boost_factor;
  }

  /**
   * @brief Set the frequency penalty factor
   * @param penalty Weight reduction factor must be betweeen 0-1
   * @return Success or error code
   *
   * Each process s multiplied by (1 - penalty) whenever it is selected.
   */
  [[nodiscard]] std::expected<void, SchedulerError> SetFrequencyPenalty(
      const double penalty) {
    if (!std::isfinite(penalty) || penalty < 0.0 ||
        penalty > MAX_FREQ_PENALTY) {
      return std::unexpected(SchedulerError::InvalidParameter);
    }
    frequency_penalty = penalty;
    return {};
  }

  [[nodiscard]] double GetFrequencyPenalty() const noexcept {
    return frequency_penalty;
  }

  /**
   * @brief Sets the base weight of a process
   * @param id Process ID
   * @param weight New base weight (must be non-negative and finite)
   * @return Success or error code
   *
   * Fails with WeightBlockedByRebalancing if rebalancing is enabled.
   * When rebalancing is on, effective weight is adjusted dynamically; setting
   * base weight has no effect. Call EnableRebalancing(false) first, set the
   * weight, then re-enable if desired.
   */
  [[nodiscard]] std::expected<void, SchedulerError> SetBaseWeight(
      ID_TYPE id, const double weight) {
    if (rebalance_enabled) {
      return std::unexpected(SchedulerError::WeightBlockedByRebalancing);
    }
    if (!std::isfinite(weight)) {
      return std::unexpected(SchedulerError::InvalidWeight);
    }
    if (weight < 0.0) {
      return std::unexpected(SchedulerError::NegativeWeight);
    }

    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }

    it->second.base_weight = weight;
    it->second.adjusted_weight = weight;
    return {};
  }

  /// Get the current adjusted weight of a process
  [[nodiscard]] std::expected<double, SchedulerError> GetAdjustedWeight(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.adjusted_weight;
  }

  /// Get the number of cycles a process has been waiting
  [[nodiscard]] std::expected<size_t, SchedulerError> GetWaitCycles(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.wait_cycles;
  }

  /// Get the current scheduling cycle number
  [[nodiscard]] size_t GetSchedulingCycle() const noexcept {
    return scheduling_cycle;
  }

  /// Reset all adjusted weights (dynamic weights) to their base values
  void ResetAdjustedWeights() {
    for (auto& [id, state] : process_map) {
      state.adjusted_weight = state.base_weight;
      state.wait_cycles = 0;
      state.execution_count = 0;
      state.last_execution_cycle = 0;
    }
    scheduling_cycle = 0;
  }

  // Failure Handling & Recovery API

  /**
   * @brief Enable or disable automatic failure handling
   * @param enable true to enable failure tracking and backoff, false to disable
   *
   * When disabling: resets failure counters and backoff state for all
   * processes, but does NOT change the enabled/disabled status of any process.
   * Processes that were manually disabled via DisableProcess() stay disabled.
   * Processes that were auto-disabled due to max consecutive failures stay
   * disabled until the user explicitly calls EnableProcess(id).
   */
  void EnableFailureHandling(bool enable) noexcept {
    failure_handling_enabled = enable;

    if (!enable) {
      for (auto& [id, state] : process_map) {
        state.ResetFailureState();
      }
    }
  }

  /// Check if failure handling is enabled
  [[nodiscard]] bool IsFailureHandlingEnabled() const noexcept {
    return failure_handling_enabled;
  }

  /**
   * @brief Mark that a process execution failed
   * @param id Process ID
   * @return Success or error code
   *
   * Increments failure count, applies exponential backoff, and may disable
   * the process if it exceeds max_consecutive_failures.
   *
   * Backoff formula: min(initial_backoff * (multiplier ^ failure_count),
   * max_backoff)
   */
  [[nodiscard]] std::expected<void, SchedulerError> MarkProcessFailed(
      ID_TYPE id) {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }

    if (!failure_handling_enabled) return {};

    ProcessState& state = it->second;
    state.failure_count++;
    state.total_failures++;
    state.success_count = 0;

    size_t backoff = (state.backoff_cycles == 0)
                         ? initial_backoff_cycles
                         : static_cast<size_t>(std::min(
                               state.backoff_cycles * backoff_multiplier,
                               static_cast<double>(max_backoff_cycles)));
    state.backoff_cycles = backoff;
    state.cycles_until_retry = backoff;

    if (state.failure_count >= max_consecutive_failures) {
      state.enabled = false;
      // Keep backoff values for informational purposes
    }

    return {};
  }

  /**
   * @brief Mark that a process execution succeeded
   * @param id Process ID
   * @return Success or error code
   *
   * Increments success count. If enough consecutive successes occur,
   * resets failure count (recovery).
   */
  [[nodiscard]] std::expected<void, SchedulerError> MarkProcessSuccess(
      ID_TYPE id) {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }

    if (!failure_handling_enabled) return {};

    ProcessState& state = it->second;
    state.success_count++;

    if (state.success_count >= recovery_success_threshold) {
      state.ResetFailureState();
    }

    return {};
  }

  /// Manually enable a disabled process
  [[nodiscard]] std::expected<void, SchedulerError> EnableProcess(ID_TYPE id) {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }

    ProcessState& state = it->second;
    state.enabled = true;
    state.deficit = GetEffectiveWeight(state);
    state.ResetFailureState();

    return {};
  }

  /// Manually disable a process
  [[nodiscard]] std::expected<void, SchedulerError> DisableProcess(ID_TYPE id) {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }

    it->second.enabled = false;
    return {};
  }

  /// Check if a process is currently enabled
  [[nodiscard]] std::expected<bool, SchedulerError> IsProcessEnabled(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.enabled;
  }

  /// Check if a process can currently be scheduled
  [[nodiscard]] std::expected<bool, SchedulerError> IsProcessSchedulable(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return IsSchedulable(it->second);
  }

  /// Get the failure count for a process
  [[nodiscard]] std::expected<size_t, SchedulerError> GetProcessFailureCount(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.failure_count;
  }

  /// Get the total failure count for a process
  [[nodiscard]] std::expected<size_t, SchedulerError> GetTotalFailures(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.total_failures;
  }

  /// Get the success streak count for a process
  [[nodiscard]] std::expected<size_t, SchedulerError> GetProcessSuccessCount(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.success_count;
  }

  /// Get the current backoff period for a process
  [[nodiscard]] std::expected<size_t, SchedulerError> GetBackoffCycles(
      ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.backoff_cycles;
  }

  /// Get cycles remaining until process can retry
  // Used to check when a failed process will be eligible
  // to run again according to backoff policy.
  [[nodiscard]] std::expected<size_t, SchedulerError>
  GetProcessCyclesUntilRetry(ID_TYPE id) const {
    auto it = process_map.find(id);
    if (it == process_map.end()) {
      return std::unexpected(SchedulerError::ProcessNotFound);
    }
    return it->second.cycles_until_retry;
  }

  // Failure Handling Configuration

  // Set maximum consecutive failures before disabling
  // Must be > 0 (Default: 3)
  [[nodiscard]] std::expected<void, SchedulerError> SetMaxConsecutiveFailures(
      const size_t max) {
    if (max == 0) {
      return std::unexpected(SchedulerError::ZeroMaxFailures);
    }
    max_consecutive_failures = max;
    return {};
  }

  [[nodiscard]] size_t GetMaxConsecutiveFailures() const noexcept {
    return max_consecutive_failures;
  }

  // Set initial backoff period after first failure
  // Must be > 0 (Default: 1)
  [[nodiscard]] std::expected<void, SchedulerError> SetInitialBackoffCycles(
      size_t cycles) {
    if (cycles <= 0) {
      return std::unexpected(SchedulerError::InvalidParameter);
    }
    initial_backoff_cycles = cycles;
    return {};
  }

  [[nodiscard]] size_t GetInitialBackoffCycles() const noexcept {
    return initial_backoff_cycles;
  }

  // Set exponential backoff growth factor
  // Must be >= 1.0 (Default: 2.0)
  [[nodiscard]] std::expected<void, SchedulerError> SetBackoffMultiplier(
      const double multiplier) {
    if (!std::isfinite(multiplier) || multiplier < 1.0) {
      return std::unexpected(SchedulerError::InvalidBackoffMultiplier);
    }
    backoff_multiplier = multiplier;
    return {};
  }

  [[nodiscard]] double GetBackoffMultiplier() const noexcept {
    return backoff_multiplier;
  }

  // Set maximum backoff period cap:
  // Limits exponential backoff after repeated failures to prevent excessively
  // long delays. Must be > 0 (Default: 64)
  [[nodiscard]] std::expected<void, SchedulerError> SetMaxBackoffCycles(
      size_t max) {
    if (max == 0) {
      return std::unexpected(SchedulerError::InvalidParameter);
    }
    max_backoff_cycles = max;
    return {};
  }

  [[nodiscard]] size_t GetMaxBackoffCycles() const noexcept {
    return max_backoff_cycles;
  }

  /// Sets the number of consecutive successful attempts to recover a process
  /// (must be > 0) [prevent flapping]
  [[nodiscard]] std::expected<void, SchedulerError> SetRecoverySuccessThreshold(
      size_t threshold) {
    if (threshold == 0) {
      return std::unexpected(SchedulerError::ZeroRecoveryThreshold);
    }
    recovery_success_threshold = threshold;
    return {};
  }

  [[nodiscard]] size_t GetRecoverySuccessThreshold() const noexcept {
    return recovery_success_threshold;
  }

  /// get the number of enabled processes
  [[nodiscard]] size_t GetEnabledProcessCount() const noexcept {
    return std::ranges::count_if(
        process_map | std::views::values,
        [](const ProcessState& s) noexcept { return s.enabled; });
  }

  /// get the number of processes that can be scheduled now
  [[nodiscard]] size_t GetSchedulableProcessCount() const noexcept {
    return std::ranges::count_if(
        process_map | std::views::values,
        [this](const ProcessState& s) noexcept { return IsSchedulable(s); });
  }
};

///  default ID type is size_t when no template argument is given.
Scheduler() -> Scheduler<size_t>;

}  // namespace cse498
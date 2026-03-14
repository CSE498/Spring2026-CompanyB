/**
 * @file Scheduler.hpp
 * @brief A generic priority-based scheduler for managing process execution order
 * @author Joshua Twumasi
 * 
 * Header-only, template-based class that manages a set of processes by priority
 * weight and returns the ID of which one should execute next. Supports
 * deterministic (weighted round-robin) and probabilistic (weighted random) modes.
 */

#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <expected>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>
#include <ranges> // for views and std::transform

namespace cse498 {

  /**
   * @enum SchedulerError
   * @brief Error codes for Scheduler operations
   * 
   */
  enum class SchedulerError 
  {
    EmptyScheduler,              ///< Scheduler has no processes
    ProcessNotFound,             ///< Requested process ID doesn't exist
    ProcessAlreadyExists,        ///< Cannot add duplicate process ID
    InvalidWeight,               ///< Weight value is invalid
    NegativeWeight,              ///< Weight cannot be negative
    NoSchedulableProcesses,      ///< No processes available to schedule
    InvalidParameter,            ///< Invalid configuration parameter
    WeightBlockedByRebalancing,   ///< Cannot set base weight while rebalancing is enabled
    ZeroMaxFailures,             ///< Max failures must be > 0
    InvalidBackoffMultiplier,    ///< Backoff multiplier must be >= 1.0
    ZeroRecoveryThreshold        ///< Recovery threshold must be > 0
  };

  /**
   * @brief Convert SchedulerError to readable string
   * @param error The error code
   * @return String description of the error
   */
  constexpr const char* to_string(SchedulerError error) noexcept 
  {
    switch(error) {
      case SchedulerError::EmptyScheduler:
        return "Scheduler has no processes";
      case SchedulerError::ProcessNotFound:
        return "Process not found in scheduler";
      case SchedulerError::ProcessAlreadyExists:
        return "Process with this ID already exists";
      case SchedulerError::InvalidWeight:
        return "Invalid weight value";
      case SchedulerError::NegativeWeight:
        return "Weight must be non-negative";
      case SchedulerError::NoSchedulableProcesses:
        return "No schedulable processes available";
      case SchedulerError::InvalidParameter:
        return "Invalid configuration parameter";
      case SchedulerError::WeightBlockedByRebalancing:
        return "Cannot set base weight while rebalancing is enabled; call EnableRebalancing(false) first";
      case SchedulerError::ZeroMaxFailures:
        return "Max consecutive failures must be greater than zero";
      case SchedulerError::InvalidBackoffMultiplier:
        return "Backoff multiplier must be >= 1.0";
      case SchedulerError::ZeroRecoveryThreshold:
        return "Recovery success threshold must be greater than zero";
    }
    return "Unknown error";
  }

  /**
   * @class Scheduler
   * @brief Generic template-based scheduler for priority-weighted process execution
   * 
   * @tparam ID_TYPE type used to identify processes (default is size_t)
   * 
   * ## Overview
   * 
   * Scheduler maintains a collection of processes, each with a priority weight,
   * and decides which one should execute next. It has three layers of functionality
   * that can be enabled independently:
   * 
   *   1. Core scheduling (always active) — picks the next process by weight.
   *   2. Dynamic weight adjustment (opt-in) — automatically adjusts weights
   *      to prevent starvation and penalize over-execution.
   *   3.   Failure handling (opt-in) — tracks failures, applies exponential
   *      backoff, and auto-disables unreliable processes.
   * 
   * ## Weight types
   * 
   * Each process tracks three weight values:
   * 
   *   - base_weight    — the user-configured weight (set via AddProcess / SetBaseWeight)
   *   - adjusted_weight  — adjusted copy of base_weight (only changes when rebalancing is on)
   *   - deficit          — round-robin accumulator used by deterministic mode (can go negative)
   * 
   * GetEffectiveWeight() returns adjusted_weight when rebalancing is enabled,
   * otherwise base_weight. This is the weight used in both scheduling algorithms.
   * 
   * ## Algorithm 1: Deterministic (Weighted Deficit Round-Robin)
   * 
   * @code
   *   for each schedulable process P:
   *       P.deficit += GetEffectiveWeight(P) 
   *   selected = process with highest deficit (ties broken by insertion order)
   *   selected.deficit -= sum of all effective weights
   *   return selected
   * @endcode
   * 
   * deficit acts as a deficit counter. It goes negative after selection,
   * then accumulates back up. Over N cycles, each process is selected in exact
   * proportion to its weight.
   * 
   * Example: A(weight=2), B(weight=1), total=3; deficits starts at A=0 and B=0 before we add their weights
   *   In the order A,B                
   *   Cycle 1: deficits  0,0   -> add weights -> 2,1 -> select A -> subtract 3 -> -1,1
   *   Cycle 2: deficits -1,1   -> add weights -> 1,2 -> select B -> subtract 3 -> 1,-1
   *   Cycle 3: deficits  1,-1  -> add weights -> 3,0 -> select A -> subtract 3 -> 0,0
   *      Therefore A will run 2 times and B runs 1 time. Consistent with A:B -> 2:1 ratio
   * 
   * 
   * ## Algorithm 2: Probabilistic (Weighted Random)
   * 
   * @code
   *   total = sum of effective weights for schedulable processes
   *   r = random number in [0, total)
   *   cumulative = 0
   *   for each schedulable process P:
   *       cumulative += GetEffectiveWeight(P)
   *       if r < cumulative: return P
   * @endcode
   * 
   * Each process has selection probability = its weight / total weight.
   * 
   * ## Dynamic weight adjustment (when enabled)
   * 
   * After each GetNext() call:
   * 
   * @code
   *   for each process P:
   *       if P was selected:
   *           P.adjusted_weight *= (1 - frequency_penalty)   // penalize over-use
   *           P.wait_cycles = 0
   *       else:
   *           P.wait_cycles++
   *           P.adjusted_weight += wait_boost_factor * P.wait_cycles  // reward waiting
   *       clamp P.adjusted_weight to [min_weight, max_weight]
   * @endcode
   * 
   * This gradually shifts weight toward starving processes and away from
   * frequently selected ones.
   * 
   * ## Failure handling (when enabled)
   * 
   * @code
   *   on MarkProcessFailed(id):
   *       P.failure_count++
   *       P.backoff_cycles = min(P.backoff_cycles * multiplier, max_backoff)
   *       P.cycles_until_retry = P.backoff_cycles   // countdown starts
   *       if P.failure_count >= max_consecutive_failures:
   *           P.enabled = false                      // auto-disable
   * 
   *   on each GetNext():
   *       for each process: if cycles_until_retry > 0, decrement it
   *       (process is skipped while cycles_until_retry > 0)
   * 
   *   on MarkProcessSuccess(id):
   *       P.success_count++
   *       if P.success_count >= recovery_threshold:
   *           reset failure state (failure_count, backoff, etc.)
   * @endcode
   * 
   * ## Quick-start example
   * 
   * @code
   *   Scheduler<size_t> sched;
   *   sched.AddProcess(1, 10.0);   // high priority
   *   sched.AddProcess(2, 5.0);    // medium
   *   sched.AddProcess(3, 1.0);    // low
   * 
   *   auto next = sched.GetNext();
   *   if (next) { execute(*next); }
   *   // Over 16 calls: process 1 runs 10x, 2 5x, 3 1x
   * @endcode
   * 
   */
  template<typename ID_TYPE = size_t>
  class Scheduler {
  public:

    /// Scheduling modes
    enum class Mode 
    {
      DETERMINISTIC,  ///< Round-robin weighted
      PROBABILISTIC   ///< Random weighted
    };
    
    /// Default configuration values
    static constexpr double MIN_WEIGHT = 0.1;
    static constexpr double MAX_WEIGHT = 1000.0;
    static constexpr double WAIT_BOOST_FACTOR = 0.1;
    static constexpr double FREQ_PENALTY = 0.05;
    static constexpr double MAX_FREQ_PENALTY = 1.0;
    static constexpr size_t MAX_FAILURES = 3;
    static constexpr size_t INTIAL_BACKOFF_CYCLES = 1;
    static constexpr double BACKOFF_MULT = 2.0;
    static constexpr size_t MAX_BACKOFF_CYCLES = 64;
    static constexpr size_t RECOVERY_THRESHOLD = 2;

  private:
    /**
     * @brief Internal metadata for each scheduled process
     * 
     * This struct contains only scheduling-specific information
     */
    struct ProcessState {
      double base_weight{};               ///< Base priority weight 
      double adjusted_weight{};            ///< Dynamically adjusted weight (used when rebalancing is enabled)
      double deficit{};                   ///< Round-robin accumulator (goes negative after selection)
      size_t execution_count{};           ///< no of times this process has been scheduled
      size_t wait_cycles{};               ///< no of cycles since last execution
      size_t last_execution_cycle{};      ///< Cycle number when last executed
      size_t insertion_order{};           ///< Order of insertion (for FCFS tie-breaking)
      
      // Failure handling
      bool enabled{};                     ///< Is this process currently enabled?
      size_t failure_count{};             ///< no of consecutive failures
      size_t success_count{};             ///< no of consecutive successes
      size_t total_failures{};            ///< Total failures across all time
      size_t backoff_cycles{};            ///< Cycles to wait before retry (exponential backoff)
      size_t cycles_until_retry{};        ///< Countdown until process can be retried
      
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
          cycles_until_retry(0)
      {
        assert(weight >= 0.0 && "Weight must be non-negative");
      }
      
      void ResetFailureState() 
      {
        failure_count = 0;
        success_count = 0;
        backoff_cycles = 0;
        cycles_until_retry = 0;
      }
    };

    // Member Variables
    
    std::unordered_map<ID_TYPE, ProcessState> process_map;  ///< Maps process ID to metadata
    Mode scheduling_mode;                                   ///< Current scheduling algorithm
    size_t next_insertion_order{};                            ///< Counter for insertion order
    
    // Random number generation for probabilistic mode
    // Remember to use Random.hpp from Group 19
    mutable std::mt19937 rng;                               ///< Random number generator
    
  
    bool rebalance_enabled{};                               ///< Whether RebalanceWeights() runs after each GetNext()
    double min_weight{};                                      ///< Minimum weight (so as to prevent starvation)
    double max_weight{};                                      ///< Maximum weight ceiling (prevents unbounded growth)
    double wait_boost_factor{};                               ///< Weight increase per wait cycle
    double frequency_penalty{};                               ///< Weight reduction for frequent execution
    size_t scheduling_cycle{};                                ///< Current scheduling cycle number
    
    
    bool failure_handling_enabled{};                          ///< Enable automatic failure handling
    size_t max_consecutive_failures{};                        ///< Max failures before disabling process
    size_t initial_backoff_cycles{};                          ///< Initial backoff period after failure
    double backoff_multiplier{};                              ///< Exponential backoff growth factor
    size_t max_backoff_cycles{};                              ///< Maximum backoff period
    size_t recovery_success_threshold{};                      ///< Successes needed to clear failure count
    
    // function Methods 
    
    /**
     * @brief Rebalance adjusted weights after a scheduling decision.
     * @param selected_id The process that was just selected.
     * 
     * Called after each GetNext(). Does nothing if rebalancing is off.
     * - The selected process's weight is reduced (frequency penalty).
     * - All other processes' weights are increased (wait boost).
     * - All weights are clamped to [min_weight, max_weight].
     */
    void RebalanceWeights(ID_TYPE selected_id) 
    {
      if (!rebalance_enabled) {return;}
      
      for (auto& [id, state] : process_map)
      {
        if (id == selected_id)
        {
          state.adjusted_weight *= (1.0 - frequency_penalty);
          state.last_execution_cycle = scheduling_cycle;
          state.wait_cycles = 0;
        } else {
          state.wait_cycles++;
          state.adjusted_weight += (wait_boost_factor * state.wait_cycles);
        }
        
        if (state.adjusted_weight < min_weight) 
        {
          state.adjusted_weight = min_weight;
        }
        if (state.adjusted_weight > max_weight) 
        {
          state.adjusted_weight = max_weight;
        }
      }
    }
    
    /**
     * @brief Get the effective weight for a process based on current settings
     * @param state ProcessState to get weight from
     * @return Effective weight
     */
    double GetEffectiveWeight(const ProcessState& state) const 
    {
      return rebalance_enabled ? state.adjusted_weight : state.base_weight;
    }
    
    /**
     * @brief Check if a process is currently schedulable 
     * @param state ProcessState to check
     * @return true if process can be scheduled false otherwise
     */
    [[nodiscard]] bool IsSchedulable(const ProcessState& state) const 
    {
      return state.enabled && state.cycles_until_retry == 0;
    }
    
    /**
     * @brief Update backoff counters for all processes 
     * 
     * Decrements cycles_until_retry for processes in backoff.
     * Called at the start of each scheduling cycle.
     */
    void UpdateBackoffCounters() 
    {
      if (!failure_handling_enabled) return;
      
      for (auto& [id, state] : process_map)
      {
        if (state.cycles_until_retry > 0)
        {
          state.cycles_until_retry--;
        }
      }
    }
    
    /**
     * @brief Select next process using deterministic round-robin weighted scheduling
     * @return ID of the process to execute next
     * 
     * Algorithm: 
     * @code
     *   for each schedulable process P:
     *       P.deficit += GetEffectiveWeight(P) 
     *   selected = process with highest deficit (ties broken by insertion order)
     *   selected.deficit -= sum of all effective weights
     *   return selected
     * @endcode
     * Note the deficit for the selected process will be negative because 
     * it is subtracted from the total weight of all processes, but eventually climbs 
     * up by adding its effective weight (base or adjusted weight) to the deficit, 
     * and it will be positive eventually.
     * https://en.wikipedia.org/wiki/Deficit_round_robin
     */
    [[nodiscard]] std::expected<ID_TYPE, SchedulerError> GetNextDeterministic() 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      
      // Struct to store the id, weight, and order of the best candidate 
      //we dont need to store the whole state of the process
      struct BestCandidate { ID_TYPE id; double weight; size_t order; };

      std::optional<BestCandidate> best;
      for (auto& [id, state] : process_map)
      {
        if (!IsSchedulable(state)) continue;
        
        // Add the process's "budget" (effective weight) to its accumulated deficit,
        // allowing it to build up credit for selection if not chosen.
        state.deficit += GetEffectiveWeight(state);

        // Choose the process with the highest deficit (most credit).
        // Break ties by choosing the process added earlier (lower insertion_order).
        bool is_better = !best ||
                        (state.deficit > best->weight) ||
                        (state.deficit == best->weight && state.insertion_order < best->order);
        
        if (is_better) 
        {
          best = BestCandidate{id, state.deficit, state.insertion_order};
        }
      }
      
      if (!best) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }

      
      // Subtract the total weight from the selected process's deficit.
      // This prevents a process from running excessively more often than its share.
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
    std::expected<ID_TYPE, SchedulerError> GetNextProbabilistic() const 
    {
      if (process_map.empty())  { return std::unexpected(SchedulerError::EmptyScheduler);  }
      
      double total_weight = GetTotalWeight();
      if (total_weight <= 0.0)  {  return std::unexpected(SchedulerError::NoSchedulableProcesses);   }
      
      // Draw to determine where we land on the weighted process interval
      std::uniform_real_distribution<double> dist(0.90, total_weight);
      double random_value = dist(rng);  

      
      double cumulative = 0.0;
      std::optional<ID_TYPE> selected_id;

      // Iterate through all processes and accumulate their weights
      // The process whose weight interval contains random_value gets selected
      for (const auto& [id, state] : process_map)
      {
        if (!IsSchedulable(state)) continue;
        
        cumulative += GetEffectiveWeight(state); // move to the next process's interval
        // check if it contains random_value
        if (random_value < cumulative) 
        {
          selected_id = id; // select the first process whose interval covers the random value
          break;
        }
      }
      
      if (!selected_id) {  return std::unexpected(SchedulerError::NoSchedulableProcesses);   }
      
      return *selected_id;
    }

  public:
    // Constructors
    
    /**
     * @brief Construct a new Scheduler
     * @param mode Scheduling algorithm to use
     * @param seed Random seed for probabilistic mode 
     */
    explicit Scheduler(Mode mode = Mode::DETERMINISTIC, 
                      unsigned int seed = std::random_device{}())
      : scheduling_mode(mode),
        next_insertion_order(0),
        rng(seed),
        rebalance_enabled(false),
        min_weight(MIN_WEIGHT),
        max_weight(MAX_WEIGHT),
        wait_boost_factor(WAIT_BOOST_FACTOR),
        frequency_penalty(FREQ_PENALTY),
        scheduling_cycle(0),
        failure_handling_enabled(false),
        max_consecutive_failures(MAX_FAILURES),
        initial_backoff_cycles(INTIAL_BACKOFF_CYCLES),
        backoff_multiplier(BACKOFF_MULT),
        max_backoff_cycles(MAX_BACKOFF_CYCLES),
        recovery_success_threshold(RECOVERY_THRESHOLD)
    { }
    

    // Core API 
    
    /**
     * @brief Add a new process to the scheduler
     * @param id id for the process
     * @param weight Priority weight (non-negative)
     * @return Success or error code
     * 
     */
    [[nodiscard]] std::expected<void, SchedulerError> AddProcess(ID_TYPE id, double weight) 
    {
      if (!std::isfinite(weight)) 
      {
        return std::unexpected(SchedulerError::InvalidWeight);
      }
      if (weight < 0.0) 
      {
        return std::unexpected(SchedulerError::NegativeWeight);
      }
      
      if (process_map.contains(id)) 
      {
        return std::unexpected(SchedulerError::ProcessAlreadyExists);
      }
      
      process_map.emplace(id, ProcessState(weight, next_insertion_order++));
      return {};  // Success
    }
    
    /**
     * @brief Remove a process from the scheduler
     * @param id id of the process
     * @return Success or error code
     */
    [[nodiscard]] std::expected<void, SchedulerError> RemoveProcess(ID_TYPE id) 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      process_map.erase(it);
      return {};  // Success
    }
    
    /**
     * @brief Remove all processes and reset scheduler state
     */
    void Clear() noexcept
    {
      process_map.clear();
      next_insertion_order = 0;
      scheduling_cycle = 0;
    }
    
    /**
     * @brief Get the ID of the next process to execute
     * @return Expected containing process ID or error code
     */
    [[nodiscard]] std::expected<ID_TYPE, SchedulerError> GetNext() 
    {
      if (process_map.empty()) {return std::unexpected(SchedulerError::EmptyScheduler);}
      
      // Step 1: Advance the scheduler cycle and decrement cycles_until_retry for any processes in backoff
      scheduling_cycle++;
      UpdateBackoffCounters(); // (UpdateBackoffCounters reduces cycles_until_retry for  failing processes to eventually make them schedulable again)
      
      // Step 2: Pick the next process using the current scheduling algorithm.
      std::expected<ID_TYPE, SchedulerError> selected = 
          (scheduling_mode == Mode::DETERMINISTIC)
              ? GetNextDeterministic()
              : GetNextProbabilistic();
      
      
      if (!selected) { return selected; } //If there was an error, it returns that error to the caller.
      
      // Step 3: Record the selection and rebalance weights for next time.
      ID_TYPE selected_id = *selected;
      process_map.find(selected_id)->second.execution_count++;
      RebalanceWeights(selected_id);
      
      return selected_id;
    }
    
    // Configuration
    
    /**
     * @brief Change the scheduling mode
     * @param mode New scheduling algorithm 
     */
    void SetMode(Mode mode) noexcept 
    {
      if (mode == scheduling_mode) return;
      
      scheduling_mode = mode;
      
       
      // initialize each process's deficit to its current effective weight(either base_weight or adjusted_weight),
      // so the round-robin algorithm starts fairly. 
      //For probabilistic mode, no persistent state (like deficit)
      // is needed, so there's nothing to reset—selection is inherently memoryless.
      if (mode == Mode::DETERMINISTIC) 
      {
        for (auto& [id, state] : process_map) 
        {
          state.deficit = GetEffectiveWeight(state);
        }
      }
    }
    
    /**
     * @brief Get the current scheduling mode
     * @return Current scheduling algorithm
     */
    [[nodiscard]] Mode GetMode() const noexcept {return scheduling_mode;}
    
    // Query Methods 
    
    /**
     * @brief Check if scheduler has any processes
     * @return true if at least one process is scheduled
     */
    [[nodiscard]] bool HasProcesses() const noexcept { return !process_map.empty(); }
    
    /**
     * @brief Get the number of processes currently scheduled
     * @return Count of processes
     */
    [[nodiscard]] size_t GetProcessCount() const noexcept { return process_map.size(); }
    
    /**
     * @brief Check if a specific process exists in the scheduler
     * @param id Process ID to check
     * @return true if process is scheduled
     */
    [[nodiscard]] bool HasProcess(ID_TYPE id) const noexcept { return process_map.contains(id); }
    
    /**
     * @brief Get the base weight of a process
     * @param id Process id
     * @return Expected containing weight or error code
     */
    [[nodiscard]] std::expected<double, SchedulerError> GetBaseWeight(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.base_weight;
    }
    
    /**
     * @brief Get the highest effective weight among all schedulable processes
     * @return Expected containing maximum weight or error code
     * 
     * Returns adjusted weight when rebalancing is enabled, base weight otherwise.
     */
   
    [[nodiscard]] std::expected<double, SchedulerError> GetHighestWeight() const 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      // Build a  pipeline: map values -> keep schedulable -> extract weight.
      auto schedulable_weights = process_map 
          | std::views::values                                                        // ProcessState stream
          | std::views::filter([this](const ProcessState& s) { return IsSchedulable(s); })  // drop disabled / backed-off
          | std::views::transform([this](const ProcessState& s) { return GetEffectiveWeight(s); }); // -> double
      
      // Find the maximum weight in the filtered range.
      auto it = std::ranges::max_element(schedulable_weights);
      if (it == schedulable_weights.end()) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }
      return *it;
    }
    /**
     * @brief Get the sum of all effective weights for schedulable processes
     * @return Total weight across all schedulable processes
     * 
     * Returns adjusted weights when rebalancing is enabled, base weights otherwise.
     */
    [[nodiscard]] double GetTotalWeight() const noexcept 
    {
      double total = 0.0;
      for (const auto& [id, state] : process_map) 
      {
        if (IsSchedulable(state)) {
          total += GetEffectiveWeight(state);
        }
      }
      return total;
    }
    
    /**
     * @brief Get the number of times a process has been selected
     * @param id Process ID
     * @return Expected containing execution count or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetExecutionCount(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.execution_count;
    }
    


    //  Dynamic Weight Adjustment API
    
    /**
     * @brief Enable or disable weight rebalancing after each scheduling decision.
     * @param enable true to rebalance weights each cycle, false to use static base weights.
     * 
     * When enabled, RebalanceWeights() runs after every GetNext() call:
     *  - The selected process's weight is reduced (frequency penalty).
     *  - Waiting processes' weights are boosted (wait boost).
     * Disabling resets all adjusted weights back to their base values.
     */
    void EnableRebalancing(bool enable) noexcept 
    {
      rebalance_enabled = enable;
      if (!enable) {
        for (auto& [id, state] : process_map) {
          state.adjusted_weight = state.base_weight;
          state.wait_cycles = 0;
        }
      }
    }
    
    /**
     * @brief Check if weight rebalancing is enabled
     * @return true if rebalancing is active
     */
    [[nodiscard]] bool IsRebalancingEnabled() const noexcept 
    {
      return rebalance_enabled;
    }
    
    /**
     * @brief Set the minimum weight threshold
     * @param min Minimum weight value (not negative)
     * @return Success or error code
     * Default is  0.1
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetMinWeight(double min) 
    {
      if (!std::isfinite(min)) return std::unexpected(SchedulerError::InvalidWeight);
      if (min < 0.0) return std::unexpected(SchedulerError::NegativeWeight);
      min_weight = min;
      return {};
    }
    
    /**
     * @brief Get the current minimum weight threshold
     * @return Min weight value
     */
    [[nodiscard]] double GetMinWeight() const noexcept { return min_weight;  }
    
    /**
     * @brief Set the maximum weight ceiling
     * @param max Maximum weight value (must be positive and finite)
     * @return Success or error code
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetMaxWeight(double max) 
    {
      if (!std::isfinite(max) || max <= 0.0) return std::unexpected(SchedulerError::InvalidParameter);
      max_weight = max;
      return {};
    }
    
    /**
     * @brief Get the current maximum weight ceiling
     * @return Max weight value
     */
    [[nodiscard]] double GetMaxWeight() const noexcept { return max_weight; }
    
    /**
     * @brief Set the wait boost factor
     * @param factor Weight increase per wait cycle (must be non-negative)
     * @return Success or error code
     * 
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetWaitBoostFactor(double factor) 
    {
      if (!std::isfinite(factor)) return std::unexpected(SchedulerError::InvalidParameter);
      if (factor < 0.0)  return std::unexpected(SchedulerError::InvalidParameter);
      wait_boost_factor = factor;
      return {};
    }
    
    /**
     * @brief Get the current wait boost factor
     * @return Wait boost factor value
     */
    [[nodiscard]] double GetWaitBoostFactor() const noexcept { return wait_boost_factor;  }
    
    /**
     * @brief Set the frequency penalty factor
     * @param penalty Weight reduction factor must be betweeen 0-1
     * @return Success or error code
     * 
     * Each process s multiplied by (1 - penalty) whenever it is selected.
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetFrequencyPenalty(double penalty) 
    {
      if (!std::isfinite(penalty) || penalty < 0.0 || penalty > MAX_FREQ_PENALTY) 
      {
        return std::unexpected(SchedulerError::InvalidParameter);
      }
      frequency_penalty = penalty;
      return {};
    }
    
    /**
     * @brief Get the current frequency penalty factor
     * @return Frequency penalty value
     */
    [[nodiscard]] double GetFrequencyPenalty() const noexcept {  return frequency_penalty;  }
    
    /**
     * @brief Sets the base weight of a process
     * @param id Process ID
     * @param weight New base weight (must be non-negative and finite)
     * @return Success or error code
     * 
     * Fails with WeightBlockedByRebalancing if rebalancing is enabled.
     * When rebalancing is on, effective weight is adjusted dynamically; setting base weight has no effect.
     * Call EnableRebalancing(false) first, set the weight, then
     * re-enable if desired.
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetBaseWeight(ID_TYPE id, double weight) 
    {
      if (rebalance_enabled)
      {
        return std::unexpected(SchedulerError::WeightBlockedByRebalancing);
      }
      if (!std::isfinite(weight)) { return std::unexpected(SchedulerError::InvalidWeight); }
      if (weight < 0.0) { return std::unexpected(SchedulerError::NegativeWeight);  }
      
      auto it = process_map.find(id);
      if (it == process_map.end()) { return std::unexpected(SchedulerError::ProcessNotFound); }
      
      it->second.base_weight = weight;
      it->second.adjusted_weight = weight;
      return {};
    }
    
    /**
     * @brief Get the current dynamic weight of a process
     * @param id Process ID
     * @return Expected containing dynamic weight or error code
     */
    [[nodiscard]] std::expected<double, SchedulerError> GetDynamicWeight(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.adjusted_weight;
    }
    
    /**
     * @brief Get the number of cycles a process has been waiting
     * @param id Process ID
     * @return Expected containing wait cycle count or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetWaitCycles(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.wait_cycles;
    }
    
    /**
     * @brief Get the current scheduling cycle number
     * @return Total number of GetNext() calls made
     */
    [[nodiscard]] size_t GetSchedulingCycle() const noexcept { return scheduling_cycle;  }
    
    /**
     * @brief Reset all dynamic weights to their base values
     * 
     */
    void ResetDynamicWeights() 
    {
      for (auto& [id, state] : process_map) 
      {
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
     * When disabling: resets failure counters and backoff state for all processes,
     * but does NOT change the enabled/disabled status of any process. Processes
     * that were manually disabled via DisableProcess() stay disabled. Processes
     * that were auto-disabled due to max consecutive failures stay disabled until
     * the user explicitly calls EnableProcess(id).
     */
    void EnableFailureHandling(bool enable) noexcept 
    {
      failure_handling_enabled = enable;
      
      if (!enable) {
        for (auto& [id, state] : process_map) 
        {
          state.ResetFailureState();
        }
      }
    }
    
    /**
     * @brief Check if failure handling is enabled
     * @return true if failure tracking is active
     */
    [[nodiscard]] bool IsFailureHandlingEnabled() const noexcept 
    {
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
     * Backoff formula: min(initial_backoff * (multiplier ^ failure_count), max_backoff)
     */
    [[nodiscard]] std::expected<void, SchedulerError> MarkProcessFailed(ID_TYPE id) 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
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
      
      if (state.failure_count >= max_consecutive_failures) 
      {
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
    [[nodiscard]] std::expected<void, SchedulerError> MarkProcessSuccess(ID_TYPE id) 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      
      if (!failure_handling_enabled) return {};
      
      ProcessState& state = it->second;
      state.success_count++;
      
      if (state.success_count >= recovery_success_threshold) 
      {
        state.ResetFailureState();
      }
      
      return {};
    }
    
    /**
     * @brief Manually enable a disabled process
     * @param id Process ID
     * @return Success or error code
     * 
     */
    [[nodiscard]] std::expected<void, SchedulerError> EnableProcess(ID_TYPE id) 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      
      ProcessState& state = it->second;
      state.enabled = true;
      state.deficit = GetEffectiveWeight(state);
      state.ResetFailureState();
      
      return {};
    }
    
    /**
     * @brief Manually disable a process
     * @param id Process ID
     * @return Success or error code
     * 
     */
    [[nodiscard]] std::expected<void, SchedulerError> DisableProcess(ID_TYPE id) 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      
      it->second.enabled = false;
      return {};
    }
    
    /**
     * @brief Check if a process is currently enabled
     * @param id Process ID
     * @return Expected containing boolean or error code
     */
    [[nodiscard]] std::expected<bool, SchedulerError> IsProcessEnabled(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.enabled;
    }
    
    /**
     * @brief Check if a process can currently be scheduled
     * @param id Process ID
     * @return Expected containing boolean or error code
     */
    [[nodiscard]] std::expected<bool, SchedulerError> IsProcessSchedulable(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return IsSchedulable(it->second);
    }
    
    /**
     * @brief Get the failure count for a process
     * @param id Process ID
     * @return Expected containing consecutive failure count or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetFailureCount(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.failure_count;
    }
    
    /**
     * @brief Get the total failure count for a process
     * @param id Process ID
     * @return Expected containing total failures or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetTotalFailures(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.total_failures;
    }
    
    /**
     * @brief Get the success streak count for a process
     * @param id Process ID
     * @return Expected containing consecutive success count or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetSuccessCount(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.success_count;
    }
    
    /**
     * @brief Get the current backoff period for a process
     * @param id Process ID
     * @return Expected containing backoff cycle count or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetBackoffCycles(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.backoff_cycles;
    }
    
    /**
     * @brief Get cycles remaining until process can retry
     * @param id Process ID
     * @return Expected containing cycles until retry or error code
     */
    [[nodiscard]] std::expected<size_t, SchedulerError> GetCyclesUntilRetry(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.cycles_until_retry;
    }
    
    // Failure Handling Configuration
    
    /**
     * @brief Set maximum consecutive failures before disabling
     * @param max Maximum failures (must be > 0)
     * @return Success or error code
     * Default: 3
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetMaxConsecutiveFailures(size_t max) 
    {
      if (max == 0) 
      {
        return std::unexpected(SchedulerError::ZeroMaxFailures);
      }
      max_consecutive_failures = max;
      return {};
    }
    
    [[nodiscard]] size_t GetMaxConsecutiveFailures() const noexcept { return max_consecutive_failures; }
    
    /**
     * @brief Set initial backoff period after first failure
     * @param cycles Number of cycles to wait
     * Default: 1
     */
    void SetInitialBackoffCycles(size_t cycles) 
    {
      initial_backoff_cycles = cycles;
    }
    
    [[nodiscard]] size_t GetInitialBackoffCycles() const noexcept { return initial_backoff_cycles;    }
    
    /**
     * @brief Set exponential backoff growth factor
     * @param multiplier Growth rate (must be >= 1.0)
     * @return Success or error code
     * Default: 2.0
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetBackoffMultiplier(double multiplier) 
    {
      if (!std::isfinite(multiplier) || multiplier < 1.0) 
      {
        return std::unexpected(SchedulerError::InvalidBackoffMultiplier);
      }
      backoff_multiplier = multiplier;
      return {};
    }
    
    [[nodiscard]] double GetBackoffMultiplier() const noexcept  { return backoff_multiplier;  }
    
    /**
     * @brief Set maximum backoff period cap
     * @param max Maximum backoff cycles
     * Default: 64
     */
    void SetMaxBackoffCycles(size_t max) 
    {
      max_backoff_cycles = max;
    }
    
    [[nodiscard]] size_t GetMaxBackoffCycles() const noexcept 
    {
      return max_backoff_cycles;
    }
    
    /**
     * @brief Set consecutive successes needed for recovery
     * @param threshold Number of successes (must be > 0)
     * @return Success or error code
     * Default: 2
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetRecoverySuccessThreshold(size_t threshold) 
    {
      if (threshold == 0) 
      {
        return std::unexpected(SchedulerError::ZeroRecoveryThreshold);
      }
      recovery_success_threshold = threshold;
      return {};
    }
    
    [[nodiscard]] size_t GetRecoverySuccessThreshold() const noexcept {return recovery_success_threshold;  }
    
    /**
     * @brief Get count of currently enabled processes
     * @return Number of enabled processes
     */
    [[nodiscard]] size_t GetEnabledProcessCount() const noexcept 
    {
      size_t count = 0;
      for (const auto& [id, state] : process_map) {
        if (state.enabled) count++;
      }
      return count;
    }
    
    /**
     * @brief Get count of currently schedulable processes
     * @return Number of processes that can be scheduled now
     */
    [[nodiscard]] size_t GetSchedulableProcessCount() const noexcept 
    {
      size_t count = 0;
      for (const auto& [id, state] : process_map) 
      {
        if (IsSchedulable(state)) count++;
      }
      return count;
    }
  };

} // closing namespace
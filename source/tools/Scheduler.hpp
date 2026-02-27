/**
 * @file Scheduler.hpp
 * @brief A generic priority-based scheduler for managing process execution order
 * @author Joshua Twumasi
 * 
 * This is a header only class by design for template bases
 * It manages a set of processes based on priority weights and returns
 * the ID of which one should execute next. Supporrts deterministic and probabilistic modes
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <expected>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>

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
   * Scheduler maintains a collection of processes with associated weights and
   * determines execution order based on those weights. 
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

  private:
    /**
     * @brief Internal metadata for each scheduled process
     * 
     * This struct contains only scheduling-specific information
     */
    struct ProcessInfo {
      double base_weight{};               ///< Base priority weight 
      double dynamic_weight{};            ///< Dynamically adjusted weight (used when auto-adjust enabled)
      double current_weight{};            ///< Current effective weight (for deterministic mode)
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
      
      ProcessInfo(double weight, size_t order)
        : base_weight(weight),
          dynamic_weight(weight),
          current_weight(weight),
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
    };

    // Member Variables
    
    std::unordered_map<ID_TYPE, ProcessInfo> process_map;  ///< Maps process ID to metadata
    Mode scheduling_mode;                                   ///< Current scheduling algorithm
    size_t next_insertion_order{};                            ///< Counter for insertion order
    
    // Random number generation for probabilistic mode
    mutable std::mt19937 rng;                               ///< Random number generator
    
  
    bool auto_adjust_enabled{};                               ///< Enable automatic weight adjustment
    double min_weight{};                                      ///< Minimum weight (so as to prevent starvation)
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
     * @brief Apply dynamic weight adjustments to all processes 
     * @param selected_id The ID of the process 
     * 
     * It is called after each GetNext() when auto_adjust_enabled is true.
     */
    void ApplyDynamicAdjustments(ID_TYPE selected_id) 
    {
      if (!auto_adjust_enabled) {return;}
      
      for (auto& [id, info] : process_map) 
      {
        if (id == selected_id) 
        {
          // Selected process, apply frequency penalty and reset wait
          info.dynamic_weight *= (1.0 - frequency_penalty);
          info.last_execution_cycle = scheduling_cycle;
          info.wait_cycles = 0;
        } else {
          // Waiting process; increment wait and apply boost
          info.wait_cycles++;
          info.dynamic_weight += (wait_boost_factor * info.wait_cycles);
        }
        
        // Enforcing minimum weight to prevent starvation
        if (info.dynamic_weight < min_weight) 
        {
          info.dynamic_weight = min_weight;
        }
      }
    }
    
    /**
     * @brief Get the effective weight for a process based on current settings
     * @param info ProcessInfo to get weight from
     * @return Effective weight
     */
    double GetEffectiveWeight(const ProcessInfo& info) const 
    {
      return auto_adjust_enabled ? info.dynamic_weight : info.base_weight;
    }
    
    /**
     * @brief Check if a process is currently schedulable 
     * @param info ProcessInfo to check
     * @return true if process can be scheduled false otherwise
     */
    bool IsSchedulable(const ProcessInfo& info) const 
    {
      return info.enabled && info.cycles_until_retry == 0; // enabled or not in backoff period
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
      
      for (auto& [id, info] : process_map) 
      {
        if (info.cycles_until_retry > 0) 
        {
          info.cycles_until_retry--;
        }
      }
    }
    
    /**
     * @brief Select next process using deterministic round-robin weighted scheduling
     * @return ID of the process to execute next
     * 
     * Algorithm: Each process accumulates its base_weight to current_weight each cycle.
     * The process with the highest current_weight is selected, then its current_weight
     * is reduced by the total weight. 
     */
    std::expected<ID_TYPE, SchedulerError> GetNextDeterministic() 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      // Find process with maximum current weight
      // In case of tie, use insertion order (FCFS)
      struct BestCandidate 
      {
        ID_TYPE id;
        double weight;
        size_t order;
      };
      
      std::optional<BestCandidate> best;
      
      for (auto& [id, info] : process_map) 
      {
     
        if (!IsSchedulable(info)) continue;
        
       
        double effective_weight = GetEffectiveWeight(info);
        info.current_weight += effective_weight;
        
        // Check if this is the new maximum
        bool is_better = !best ||
                        (info.current_weight > best->weight) ||
                        (info.current_weight == best->weight && info.insertion_order < best->order);
        
        if (is_better) 
        {
          best = BestCandidate{id, info.current_weight, info.insertion_order};
        }
      }
      
      // If no schedulable process found, return error
      if (!best) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }
      
      // Reduce the selected process's weight
      double total_weight = GetTotalWeight();
      process_map.at(best->id).current_weight -= total_weight;
      
      return best->id;
    }
    
    /**
     * @brief Select next process using probabilistic weighted random selection
     * @return ID of the process to execute next
     * 
     * Algorithm: Each process has probability equal to
     *   P(process) = weight of process / sum(all weights)
     */
    std::expected<ID_TYPE, SchedulerError> GetNextProbabilistic() const 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      double total_weight = GetTotalWeight();
      if (total_weight <= 0.0) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }
      
      
      std::uniform_real_distribution<double> dist(0.0, total_weight);
      double random_value = dist(rng);
      
    
      double cumulative = 0.0;
      std::optional<ID_TYPE> selected_id;
      
      for (const auto& [id, info] : process_map) 
      {
       
        if (!IsSchedulable(info)) continue;
        
        cumulative += GetEffectiveWeight(info);
        if (random_value < cumulative) 
        {
          selected_id = id;
          break;
        }
      }
      
 
      if (!selected_id) 
      {
        for (const auto& [id, info] : process_map) 
        {
          if (IsSchedulable(info)) 
          {
            selected_id = id;
            break;
          }
        }
      }
      
      if (!selected_id) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }
      
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
        auto_adjust_enabled(false),
        min_weight(0.1),
        wait_boost_factor(0.1),
        frequency_penalty(0.05),
        scheduling_cycle(0),
        failure_handling_enabled(false),
        max_consecutive_failures(3),
        initial_backoff_cycles(1),
        backoff_multiplier(2.0),
        max_backoff_cycles(64),
        recovery_success_threshold(2)
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
      if (weight < 0.0) 
      {
        return std::unexpected(SchedulerError::NegativeWeight);
      }
      
      if (process_map.count(id)) 
      {
        return std::unexpected(SchedulerError::ProcessAlreadyExists);
      }
      
      process_map.emplace(id, ProcessInfo(weight, next_insertion_order++));
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
     * @brief Get the ID of the next process to execute
     * @return Expected containing process ID or error code
     */
    [[nodiscard]] std::expected<ID_TYPE, SchedulerError> GetNext() 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      // Increment scheduling cycle
      scheduling_cycle++;
      
      // Update backoff counters
      UpdateBackoffCounters();
      
      std::expected<ID_TYPE, SchedulerError> selected;
      
      if (scheduling_mode == Mode::DETERMINISTIC) 
      {
        selected = GetNextDeterministic();
      } 
      else 
      {
        selected = GetNextProbabilistic();
      }
      
      // Propagate error if selection failed
      if (!selected) 
      {
        return selected;
      }
      
      ID_TYPE selected_id = *selected;
      
      // Update statistics
      process_map.at(selected_id).execution_count++;
      
      // Apply dynamic weight adjustments
      ApplyDynamicAdjustments(selected_id);
      
      return selected_id;
    }
    
    // Configuration
    
    /**
     * @brief Change the scheduling mode
     * @param mode New scheduling algorithm 
     */
    void SetMode(Mode mode) 
    {
      scheduling_mode = mode;
      if (mode == Mode::DETERMINISTIC) 
      {
        for (auto& [id, info] : process_map) 
        {
          info.current_weight = info.base_weight;
        }
      }
    }
    
    /**
     * @brief Get the current scheduling mode
     * @return Current scheduling algorithm
     */
    [[nodiscard]] Mode GetMode() const {return scheduling_mode;}
    
    // Query Methods 
    
    /**
     * @brief Check if scheduler has any processes
     * @return true if at least one process is scheduled
     */
    [[nodiscard]] bool HasProcesses() const { return !process_map.empty(); }
    
    /**
     * @brief Get the number of processes currently scheduled
     * @return Count of processes
     */
    [[nodiscard]] size_t GetProcessCount() const { return process_map.size(); }
    
    /**
     * @brief Check if a specific process exists in the scheduler
     * @param id Process ID to check
     * @return true if process is scheduled
     */
    [[nodiscard]] bool HasProcess(ID_TYPE id) const { return process_map.count(id) > 0; }
    
    /**
     * @brief Get the base weight of a process
     * @param id Process id
     * @return Expected containing weight or error code
     */
    [[nodiscard]] std::expected<double, SchedulerError> GetWeight(ID_TYPE id) const 
    {
      auto it = process_map.find(id);
      if (it == process_map.end()) 
      {
        return std::unexpected(SchedulerError::ProcessNotFound);
      }
      return it->second.base_weight;
    }
    
    /**
     * @brief Get the highest base weight among all processes
     * @return Expected containing maximum weight or error code
     */
    [[nodiscard]] std::expected<double, SchedulerError> GetHighestWeight() const 
    {
      if (process_map.empty()) 
      {
        return std::unexpected(SchedulerError::EmptyScheduler);
      }
      
      std::optional<double> max_weight;
      
      for (const auto& [id, info] : process_map) 
      {
        // Only consider schedulable processes
        if (IsSchedulable(info)) 
        {
          double effective_weight = GetEffectiveWeight(info);
          if (!max_weight || effective_weight > *max_weight) 
          {
            max_weight = effective_weight;
          }
        }
      }
      
      if (!max_weight) 
      {
        return std::unexpected(SchedulerError::NoSchedulableProcesses);
      }
      return *max_weight;
    }
    
    /**
     * @brief Get the sum of all base weights
     * @return Total weight across all processes
     */
    [[nodiscard]] double GetTotalWeight() const 
    {
      double total = 0.0;
      for (const auto& [id, info] : process_map) 
      {
        if (IsSchedulable(info)) {
          total += GetEffectiveWeight(info);
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
     * @brief Enable or disable automatic weight adjustment
     * @param enable true to enable dynamic adjustments, false to use static weights
     * 
     * When enabled, process weights are automatically adjusted based on:
     *  Frequency of execution (frequently executed processes get reduced weight)
     *  Wait time (longer processes get increased weight)
     */
    void EnableAutoAdjustment(bool enable) 
    {
      auto_adjust_enabled = enable;
      
      // Reset dynamic weights to base weights when disabling
      if (!enable) 
      {
        for (auto& [id, info] : process_map) 
        {
          info.dynamic_weight = info.base_weight;
          info.wait_cycles = 0;
        }
      }
    }
    
    /**
     * @brief Check if automatic weight adjustment is enabled
     * @return true if auto-adjustment is active
     */
    [[nodiscard]] bool IsAutoAdjustmentEnabled() const 
    {
      return auto_adjust_enabled;
    }
    
    /**
     * @brief Set the minimum weight threshold
     * @param min Minimum weight value (not negative)
     * @return Success or error code
     * Default is  0.1
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetMinWeight(double min) 
    {
      if (min < 0.0) return std::unexpected(SchedulerError::NegativeWeight);
      min_weight = min;
      return {};
    }
    
    /**
     * @brief Get the current minimum weight threshold
     * @return Min weight value
     */
    [[nodiscard]] double GetMinWeight() const { return min_weight;  }
    
    /**
     * @brief Set the wait boost factor
     * @param factor Weight increase per wait cycle (must be non-negative)
     * @return Success or error code
     * 
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetWaitBoostFactor(double factor) 
    {
      if (factor < 0.0)  return std::unexpected(SchedulerError::InvalidParameter);
      wait_boost_factor = factor;
      return {};
    }
    
    /**
     * @brief Get the current wait boost factor
     * @return Wait boost factor value
     */
    [[nodiscard]] double GetWaitBoostFactor() const { return wait_boost_factor;  }
    
    /**
     * @brief Set the frequency penalty factor
     * @param penalty Weight reduction factor must be betweeen 0-1
     * @return Success or error code
     * 
     * Each process s multiplied by (1 - penalty) whenever it is selected.
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetFrequencyPenalty(double penalty) 
    {
      if (penalty < 0.0 || penalty > 1.0) 
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
    [[nodiscard]] double GetFrequencyPenalty() const {  return frequency_penalty;  }
    
    /**
     * @brief Sets the base weight of a process
     * @param id Process ID
     * @param weight New base weight (must be non-negative)
     * @return Success or error code
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetWeight(ID_TYPE id, double weight) 
    {
      if (weight < 0.0) { return std::unexpected(SchedulerError::NegativeWeight);  }
      
      auto it = process_map.find(id);
      if (it == process_map.end()) { return std::unexpected(SchedulerError::ProcessNotFound); }
      
      it->second.base_weight = weight;
      
      // If auto-adjust is disabled, also update dynamic weight
      if (!auto_adjust_enabled) { it->second.dynamic_weight = weight; }
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
      return it->second.dynamic_weight;
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
    [[nodiscard]] size_t GetSchedulingCycle() const { return scheduling_cycle;  }
    
    /**
     * @brief Reset all dynamic weights to their base values
     * 
     */
    void ResetDynamicWeights() 
    {
      for (auto& [id, info] : process_map) 
      {
        info.dynamic_weight = info.base_weight;
        info.wait_cycles = 0;
        info.execution_count = 0;
        info.last_execution_cycle = 0;
      }
      scheduling_cycle = 0;
    }
    

    // Failure Handling & Recovery API

    /**
     * @brief Enable or disable automatic failure handling
     * @param enable true to enable failure tracking and backoff, false to disable
     * 
     */
    void EnableFailureHandling(bool enable) 
    {
      failure_handling_enabled = enable;
      
      // Reset failure state when disabling
      if (!enable) {
        for (auto& [id, info] : process_map) 
        {
          info.failure_count = 0;
          info.success_count = 0;
          info.backoff_cycles = 0;
          info.cycles_until_retry = 0;
          info.enabled = true;
        }
      }
    }
    
    /**
     * @brief Check if failure handling is enabled
     * @return true if failure tracking is active
     */
    [[nodiscard]] bool IsFailureHandlingEnabled() const 
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
      
      ProcessInfo& info = it->second;
      
      // Increment failure counters
      info.failure_count++;
      info.total_failures++;
      info.success_count = 0;  // Reset success streak
      
      // Apply exponential backoff
      size_t backoff = initial_backoff_cycles;
      for (size_t i = 1; i < info.failure_count; ++i) 
      {
        backoff = static_cast<size_t>(backoff * backoff_multiplier);
        if (backoff > max_backoff_cycles) 
        {
          backoff = max_backoff_cycles;
          break;
        }
      }
      info.backoff_cycles = backoff;
      info.cycles_until_retry = backoff;
      
      // Check if process should be disabled
      if (info.failure_count >= max_consecutive_failures) 
      {
        info.enabled = false;
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
      
      ProcessInfo& info = it->second;
      
      // Increment success counter
      info.success_count++;
      
      // Check for recovery: enough consecutive successes clears failures
      if (info.success_count >= recovery_success_threshold) 
      {
        info.failure_count = 0;
        info.success_count = 0;
        info.backoff_cycles = 0;
        info.cycles_until_retry = 0;
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
      
      ProcessInfo& info = it->second;
      info.enabled = true;
      info.failure_count = 0;
      info.success_count = 0;
      info.backoff_cycles = 0;
      info.cycles_until_retry = 0;
      
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
    
    [[nodiscard]] size_t GetMaxConsecutiveFailures() const { return max_consecutive_failures; }
    
    /**
     * @brief Set initial backoff period after first failure
     * @param cycles Number of cycles to wait
     * Default: 1
     */
    void SetInitialBackoffCycles(size_t cycles) 
    {
      initial_backoff_cycles = cycles;
    }
    
    [[nodiscard]] size_t GetInitialBackoffCycles() const { return initial_backoff_cycles;    }
    
    /**
     * @brief Set exponential backoff growth factor
     * @param multiplier Growth rate (must be >= 1.0)
     * @return Success or error code
     * Default: 2.0
     */
    [[nodiscard]] std::expected<void, SchedulerError> SetBackoffMultiplier(double multiplier) 
    {
      if (multiplier < 1.0) 
      {
        return std::unexpected(SchedulerError::InvalidBackoffMultiplier);
      }
      backoff_multiplier = multiplier;
      return {};
    }
    
    [[nodiscard]] double GetBackoffMultiplier() const  { return backoff_multiplier;  }
    
    /**
     * @brief Set maximum backoff period cap
     * @param max Maximum backoff cycles
     * Default: 64
     */
    void SetMaxBackoffCycles(size_t max) 
    {
      max_backoff_cycles = max;
    }
    
    [[nodiscard]] size_t GetMaxBackoffCycles() const 
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
    
    [[nodiscard]] size_t GetRecoverySuccessThreshold() const {return recovery_success_threshold;  }
    
    /**
     * @brief Get count of currently enabled processes
     * @return Number of enabled processes
     */
    [[nodiscard]] size_t GetEnabledProcessCount() const 
    {
      size_t count = 0;
      for (const auto& [id, info] : process_map) {
        if (info.enabled) count++;
      }
      return count;
    }
    
    /**
     * @brief Get count of currently schedulable processes
     * @return Number of processes that can be scheduled now
     */
    [[nodiscard]] size_t GetSchedulableProcessCount() const 
    {
      size_t count = 0;
      for (const auto& [id, info] : process_map) 
      {
        if (IsSchedulable(info)) count++;
      }
      return count;
    }
  };

} 
// ATTRIBUTIONS:
// I had Chat-GPT write the docstring style comments. I modified them a bit
// afterwards

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <exception>
#include <expected>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <type_traits>
#include <vector>

namespace cse498 {

template <typename T>
concept VectorReturnable =
    !std::is_reference_v<T> &&
    requires(std::vector<T> v, T t) { v.emplace_back(t); };

enum class FunctionSetError { IndexOutOfBounds };

/**
 * @brief A collection of callable objects with uniform signature.
 *
 * Stores std::function objects and allows batch invocation, error
 * handling, and indexed access.
 *
 * @tparam Ret Return type of the functions
 * @tparam Params Parameter types of the functions
 */
template <typename Ret, typename... Params>
class FunctionSet {
  using FuncType = std::function<Ret(Params...)>;

  std::vector<FuncType> functions;

 public:
  /** @brief Default constructor, creates an empty FunctionSet. */
  constexpr FunctionSet() = default;

  /**
   * @brief Construct a FunctionSet from an initializer list of functions.
   *
   * @param list The initializer list of std::function objects.
   */
  constexpr FunctionSet(std::initializer_list<FuncType> list) {
    for (const auto& val : list) {
      add(val);
    }
  }

  /**
   * @brief Add a function to the set.
   *
   * @param func The std::function object to add.
   */
  constexpr void add(const FuncType& func) { functions.emplace_back(func); }

  /**
   * @brief Check whether a function can be stored in the set.
   *
   * @param func The function to check.
   *
   * @note We never actually use f, but its useful so the compiler can auto
   *deduce type
   **/
  template <typename F>
  constexpr bool check_storable([[maybe_unused]] const F& f) const {
    return std::is_constructible_v<FuncType, F>;
  }

  /**
   * @brief Invoke all functions in the set with the given arguments.
   *
   * Exceptions thrown by any function propagate out.
   *
   * @param args Arguments to pass to each stored function.
   * @returns std::vector<Ret> with the function return types, if they are
   * vector storable, void otherwise.
   */
  constexpr auto invoke(const Params&... args) const {
    if constexpr (VectorReturnable<Ret>) {
      std::vector<Ret> results;
      results.reserve(functions.size());
      for (const auto& func : functions) results.emplace_back(func(args...));
      return results;
    } else {
      std::ranges::for_each(functions,
                            [&](const FuncType& func) { func(args...); });
    }
  }

  /**
   * @brief Invoke all functions and collect any errors.
   *
   * Executes each function in the set. If a function throws, its index
   * and the exception are recorded and execution continues with the next
   * function. Returns a std::expected:
   * - On success: std::vector<Ret> (if Ret is VectorReturnable), or void
   * - On failure: std::vector<std::pair<size_t, std::exception_ptr>> containing
   *   the index and exception pointer for each function that threw
   *
   * @param args Arguments to pass to each function.
   * @return std::expected<std::vector<Ret>, ErrorType> if Ret is
   * VectorReturnable, std::expected<void, ErrorType> otherwise, where ErrorType
   * = std::vector<std::pair<size_t, std::exception_ptr>>
   */
  constexpr auto invoke_catch(const Params&... args) const {
    using ErrorType = std::vector<std::pair<size_t, std::exception_ptr>>;
    std::vector<std::pair<size_t, std::exception_ptr>> errors;

    if constexpr (VectorReturnable<Ret>) {
      std::vector<Ret> results;
      results.reserve(functions.size());
      for (size_t index{0}; index < functions.size(); ++index) {
        try {
          results.emplace_back(functions[index](args...));
        } catch (...) {
          errors.emplace_back(index, std::current_exception());
        }
      }
      if (!errors.empty())
        return std::expected<std::vector<Ret>, ErrorType>{
            std::unexpected{errors}};
      return std::expected<std::vector<Ret>, ErrorType>{results};
    } else {
      for (size_t index{0}; index < functions.size(); ++index) {
        try {
          functions[index](args...);
        } catch (...) {
          errors.emplace_back(index, std::current_exception());
        }
      }
      if (!errors.empty())
        return std::expected<void, ErrorType>{std::unexpected{errors}};
      return std::expected<void, ErrorType>{};
    }
  }

  /**
   * @brief Invoke functions in the set until one throws.
   *
   * Executes each function in order. If a function throws, execution stops
   * immediately and the index of the failing function and its exception are
   * returned. Returns a std::expected:
   * - On success: std::vector<Ret> (if Ret is VectorReturnable), or void
   * - On failure: std::pair<size_t, std::exception_ptr> containing the index
   *   and exception pointer of the first function that threw
   *
   * @param args Arguments to pass to each function.
   * @return std::expected<std::vector<Ret>, ErrorType> if Ret is
   * VectorReturnable, std::expected<void, ErrorType> otherwise, where ErrorType
   * = std::pair<size_t, std::exception_ptr>
   */
  constexpr auto invoke_until_catch(const Params&... args) const {
    using ErrorType = std::pair<size_t, std::exception_ptr>;

    if constexpr (VectorReturnable<Ret>) {
      std::vector<Ret> results;
      results.reserve(functions.size());
      for (size_t index{0}; index < functions.size(); ++index) {
        try {
          results.emplace_back(functions[index](args...));
        } catch (...) {
          return std::expected<std::vector<Ret>, ErrorType>{
              std::unexpected{ErrorType{index, std::current_exception()}}};
        }
      }
      return std::expected<std::vector<Ret>, ErrorType>{results};
    } else {
      for (size_t index{0}; index < functions.size(); ++index) {
        try {
          functions[index](args...);
        } catch (...) {
          return std::expected<void, ErrorType>{
              std::unexpected{ErrorType{index, std::current_exception()}}};
        }
      }
      return std::expected<void, ErrorType>{};
    }
  }

  /**
   * @brief Call operator forwarding to invoke.
   *
   * Allows the FunctionSet to be called like a function.
   *
   * @param args Arguments to pass to each stored function.
   */
  constexpr void operator()(Params... args) const { invoke(args...); }

  /**
   * @brief Get underlying function container if the caller wants to do any
   * modification or uncommon operations that hasnt been implemented in the
   * FunctionSet. We can do this because the FunctionSet holds no state on the
   * function vector ,so it being modified without our knowledge doesnt break
   * any invariant.
   */
  [[nodiscard]] constexpr std::vector<FuncType>& GetFunctions() noexcept {
    return functions;
  }

  [[nodiscard]] constexpr const std::vector<FuncType>& GetFunctions()
      const noexcept {
    return functions;
  }

  /**
   * @brief Access a function by index with bounds checking.
   *
   * @param index Index of the function to access.
   * @return FuncType& Reference to the function.
   * @throws std::out_of_range if index is invalid.
   */
  [[nodiscard]] constexpr FuncType& at(size_t index) {
    return functions.at(index);
  }

  /**
   * @brief Const overload of at().
   *
   * @param index Index of the function to access.
   * @return const FuncType& Const reference to the function.
   * @throws std::out_of_range if index is invalid.
   */
  [[nodiscard]] constexpr const FuncType& at(size_t index) const {
    return functions.at(index);
  }

  /**
   * @brief Access a function by index without bounds checking.
   *
   * @param index Index of the function.
   * @return FuncType& Reference to the function.
   *
   * @note UB if index is out of bounds
   */
  [[nodiscard]] constexpr FuncType& operator[](size_t index) noexcept {
    assert(index < size());
    return functions[index];
  }

  /**
   * @brief Const overload of operator[].
   *
   * @param index Index of the function.
   * @return const FuncType& Const reference to the function.
   *
   * @note UB if index is out of bounds
   */
  [[nodiscard]] constexpr const FuncType& operator[](
      size_t index) const noexcept {
    assert(index < size());
    return functions[index];
  }

  /**
   * @brief Remove a function at a specific index with bounds checking.
   *
   * @param index Index of the function to remove.
   * @returns std::expected<void, FunctionSetError> depending on if index is out
   * of bounds.
   */
  constexpr std::expected<void, FunctionSetError> pop_at(size_t index) {
    if (index >= functions.size()) {
      return std::unexpected{FunctionSetError::IndexOutOfBounds};
    }
    functions.erase(functions.begin() + index);
    return {};
  }

  /**
   * @brief Remove the last function in the set.
   *
   */
  constexpr void pop() noexcept { functions.pop_back(); }

  /** @brief Remove all functions from the set. */
  constexpr void clear() noexcept { functions.clear(); }

  /** @brief Check if the set is empty. */
  [[nodiscard]] constexpr bool empty() const noexcept {
    return functions.empty();
  }

  /** @brief Return the number of stored functions. */
  [[nodiscard]] constexpr size_t size() const noexcept {
    return functions.size();
  }

  /**
   * @brief Access the first function in the set.
   * @return FuncType& Reference to the first function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr FuncType& front() noexcept {
    assert(size());
    return functions.front();
  }

  /**
   * @brief Const overload of front().
   * @return const FuncType& Const reference to the first function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr const FuncType& front() const noexcept {
    assert(size());
    return functions.front();
  }

  /**
   * @brief Access the last function in the set.
   * @return FuncType& Reference to the last function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr FuncType& back() noexcept {
    assert(size());
    return functions.back();
  }

  /**
   * @brief Const overload of back().
   * @return const FuncType& Const reference to the last function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr const FuncType& back() const noexcept {
    assert(size());
    return functions.back();
  }

  // -----------------------------------------------------------------------------
  // Iterator accessors
  // -----------------------------------------------------------------------------
  // Below this point are iterator overloads (begin, end, rbegin, rend, etc.)
  [[nodiscard]] constexpr auto begin() noexcept { return functions.begin(); }
  [[nodiscard]] constexpr auto begin() const noexcept {
    return functions.begin();
  }

  [[nodiscard]] constexpr auto end() noexcept { return functions.end(); }
  [[nodiscard]] constexpr auto end() const noexcept { return functions.end(); }

  [[nodiscard]] constexpr auto cbegin() const noexcept {
    return functions.cbegin();
  }
  [[nodiscard]] constexpr auto cend() const noexcept {
    return functions.cend();
  }

  [[nodiscard]] constexpr auto rbegin() noexcept { return functions.rbegin(); }
  [[nodiscard]] constexpr auto rbegin() const noexcept {
    return functions.rbegin();
  }

  [[nodiscard]] constexpr auto rend() noexcept { return functions.rend(); }
  [[nodiscard]] constexpr auto rend() const noexcept {
    return functions.rend();
  }

  [[nodiscard]] constexpr auto crbegin() const noexcept {
    return functions.crbegin();
  }
  [[nodiscard]] constexpr auto crend() const noexcept {
    return functions.crend();
  }
};
}  // namespace cse498

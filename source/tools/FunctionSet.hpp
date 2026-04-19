// ATTRIBUTIONS:
// I had Chat-GPT write the docstring style comments. I modified them a bit
// afterwards

#pragma once

#include <algorithm>
#include <expected>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <type_traits>
#include <vector>

namespace cse498 {

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
  FunctionSet() = default;

  /**
   * @brief Construct a FunctionSet from an initializer list of functions.
   *
   * @param list The initializer list of std::function objects.
   */
  FunctionSet(std::initializer_list<FuncType> list) {
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
   * @brief Check whether a fucntion can be stored in the set.
   *
   * @pram func The function to check.
   *
   * @note We never actually use f, but its useful so the compiler can auto
   *deduce type
   **/
  template <typename F>
  constexpr bool check_storable(const F& f) {
    return std::is_constructible_v<FuncType, F>;
  }

  /**
   * @brief Invoke all functions in the set with the given arguments.
   *
   * Exceptions thrown by any function propagate out.
   *
   * @param args Arguments to pass to each stored function.
   */
  constexpr void invoke(const Params&... args) const {
    std::ranges::for_each(functions,
                          [&](const FuncType& func) { func(args...); });
  }

  /**
   * @brief Invoke all functions and collect any errors.
   *
   * Executes each function in the set. If a function throws, its index
   * is recorded. Returns a std::expected:
   * - `void` on success
   * - `std::vector<std::pair<size_t, std::exception_ptr>>` of function indexes
   * that threw and the exception
   *
   * @param args Arguments to pass to each function.
   * @return std::expected<void, std::vector<size_t>>
   */
  constexpr std::expected<void,
                          std::vector<std::pair<size_t, std::exception_ptr>>>
  invoke_catch(const Params&... args) const {
    std::vector<std::pair<size_t, std::exception_ptr>> errors;
    // std::views::enumerate doesnt work on apple clang so we do old style for
    // loop instead
    // We cant do a normal range based loop because we want the index
    for (size_t index{0}; index < functions.size(); ++index) {
      auto func = functions[index];
      try {
        func(args...);
      } catch (...) {
        errors.emplace_back(index, std::current_exception());
      }
    }
    if (!errors.empty()) {
      return std::unexpected{errors};
    }
    return {};
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
    return functions.front();
  }

  /**
   * @brief Const overload of front().
   * @return const FuncType& Const reference to the first function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr const FuncType& front() const noexcept {
    return functions.front();
  }

  /**
   * @brief Access the last function in the set.
   * @return FuncType& Reference to the last function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr FuncType& back() noexcept { return functions.back(); }

  /**
   * @brief Const overload of back().
   * @return const FuncType& Const reference to the last function.
   *
   * @note UB if set is empty
   */
  [[nodiscard]] constexpr const FuncType& back() const noexcept {
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

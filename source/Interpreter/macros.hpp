#pragma once

/** @brief Check result of `expr`, returning early with the error if the
 * expected contains the unexp value. Otherwise sets `var` to the success value
 * of the expected.
 */
#define TRY_DECL(var, expr)         \
  auto _result_##var = (expr);      \
  if (!_result_##var.has_value()) { \
    return _result_##var.error();   \
  }                                 \
  auto var = _result_##var.value();

/** @brief Check result of `expr`, returning early with the contianed error if
 * the expected contains the unexp value, otherwise continuing evaluation.
 */
#define TRY(expr)               \
  {                             \
    auto _result = (expr);      \
    if (!_result.has_value()) { \
      return _result.error();   \
    }                           \
  }

/** @brief `TRY_DECL` for move-only objects. */
#define TRY_DECL_M(var, expr)       \
  auto _result_##var = (expr);      \
  if (!_result_##var.has_value()) { \
    return _result_##var.error();   \
  }                                 \
  auto var = std::move(_result_##var.value());

/** @brief Wrap expression in an if-constexpr which checks if that
 * expression is well-formed, returning it if so. If not, execution continues
 * without alteration, permitting fallthrough when used in a switch.
 */
#define STATIC_CHECKED_OP(expression)         \
  {                                           \
    if constexpr (requires { expression; }) { \
      return (expression);                    \
    }                                         \
  }

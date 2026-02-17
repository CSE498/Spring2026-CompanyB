/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A specification of a coordinate position in a World.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>    // For assert
#include <concepts>   // For std::integral
#include <compare>    // For operator<=>
#include <cstddef>    // For size_t

namespace cse498 {

  /// @class WorldPosition
  /// @brief Represents a position within a 2D world.
  /// Stored as floating point, but can be converted to coordinates
  class WorldPosition {
  private:
    double x = 0.0;
    double y = 0.0;

  public:
    WorldPosition() = default;
    WorldPosition(double x, double y) : x(x), y(y) { }
    template <std::integral T, std::integral U>
    WorldPosition(T x, U y)
      : WorldPosition(static_cast<double>(x), static_cast<double>(y)) {}

    WorldPosition(const WorldPosition &) = default;
    WorldPosition & operator=(const WorldPosition &) = default;

    // -- Accessors --

    [[nodiscard]] double X() const { return x; }
    [[nodiscard]] double Y() const { return y; }
    [[nodiscard]] size_t CellX() const {
      assert(x >= 0.0);
      return static_cast<size_t>(x);
    }
    [[nodiscard]] size_t CellY() const {
      assert(y >= 0.0);
      return static_cast<size_t>(y);
    }

    /// Enable all comparison operators (==, !=, <, <=, >, >=)
    auto operator<=>(const WorldPosition &) const = default;

    WorldPosition & Set(double in_x, double in_y) {
      x = in_x;
      y = in_y;
      return *this;
    }

    // DEVELOPER NOTE: Add a SameCell function to identify if two positions are in the same cell.

    /// Return a the WorldPosition at the requested offset.
    [[nodiscard]] WorldPosition GetOffset(double offset_x, double offset_y) const {
      return WorldPosition{x+offset_x,y+offset_y};
    }

    [[nodiscard]] WorldPosition Up()    const { return {x, y-1.0}; }
    [[nodiscard]] WorldPosition Down()  const { return {x, y+1.0}; }
    [[nodiscard]] WorldPosition Left()  const { return {x-1.0, y}; }
    [[nodiscard]] WorldPosition Right() const { return {x+1.0, y}; }
  };

} // End of namespace cse498

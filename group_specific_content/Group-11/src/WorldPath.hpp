
// WorldPath is a list of ordered points representing an agent's path.

#pragma once

#include <cmath>
#include <cstddef>
#include <optional>
#include <print>
#include <vector>

#include "Math/Point.hpp"

using Point = Math::Point;

class WorldPath {
private:
    std::vector<Point> points;

    static bool isValidPoint(const Point& p) noexcept {
        return std::isfinite(p.x) && std::isfinite(p.y);
    }

    static double distance(const Point& a, const Point& b) noexcept {
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

public:
    void Reserve(std::size_t n) { points.reserve(n); }
    void Clear() noexcept { points.clear(); }

    [[nodiscard]] bool Empty() const noexcept { return points.empty(); }
    [[nodiscard]] std::size_t Size() const noexcept { return points.size(); }

    // Returns false if p has NaN/Inf coordinates.
    [[nodiscard]] bool AddPoint(const Point& p) {
        if (!isValidPoint(p)) return false;
        points.push_back(p);
        return true;
    }

    // Returns nullptr if index is out of range.
    [[nodiscard]] const Point* Get(std::size_t index) const noexcept {
        return index < points.size() ? &points[index] : nullptr;
    }

    // Returns false if already empty.
    [[nodiscard]] bool PopBack() noexcept {
        if (points.empty()) return false;
        points.pop_back();
        return true;
    }

    // Returns nullptr if empty.
    [[nodiscard]] const Point* Front() const noexcept {
        return points.empty() ? nullptr : &points.front();
    }
    [[nodiscard]] const Point* Back() const noexcept {
        return points.empty() ? nullptr : &points.back();
    }

    [[nodiscard]] const std::vector<Point>& Points() const noexcept { return points; }

    [[nodiscard]] bool IsValid() const noexcept {
        for (const auto& p : points) {
            if (!isValidPoint(p)) return false;
        }
        return true;
    }

    [[nodiscard]] double TotalLength() const noexcept {
        double sum = 0.0;
        for (std::size_t i = 1; i < points.size(); ++i) {
            sum += distance(points[i - 1], points[i]);
        }
        return sum;
    }

    // Returns the length between the point[index] and the next point.
    // Returns nullopt if there is no "next point" (so index is the last point or out of range).
    [[nodiscard]] std::optional<double> SegmentLength(std::size_t index) const noexcept {
        if (index + 1 >= points.size()) return std::nullopt;
        return distance(points[index], points[index + 1]);
    }

    [[nodiscard]] bool SelfIntersects() const noexcept { return false; }

    void LogPath() const {
        for (const auto& p : points) {
            std::print("({}, {})\n", p.x, p.y);
        }
    }
};

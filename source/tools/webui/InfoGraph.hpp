/**
 * @file InfoGraph.hpp
 * @brief A web-based graph component for displaying time-series data.
 * @author Fatima Saad
 * Supports line and bar charts, tick-based updates, and horizontal scrolling
 * to visualize recent data without compression.
 */

#pragma once

#include <string>
#include <vector>

#include "WebCanvas.hpp"

namespace cse498 {

class InfoGraph : public WebCanvas {
 public:
  using DataSeries = std::vector<double>;

  InfoGraph(int width, int height, const WebOptions& options = {});
/// @brief Draw a line chart from a set of values.
  void DrawLineChart(const DataSeries& values,
                     const std::string& title = "");
/// @brief Draw a bar chart from a set of values.
  void DrawBarChart(const DataSeries& values,
                    const std::string& title = "");

/// @brief Add a new data point (one simulation tick).
  void AddDataPoint(double value, const std::string& title = "Live Line Chart");
/// @brief Clear all stored data and reset the graph.
  void ClearData();
/// @brief Scroll the visible window left (older data).
  void ScrollLeft();
  /// @brief Scroll the visible window right (newer data).
  void ScrollRight();

/// @brief Enable or disable automatic scrolling.
  void SetAutoScroll(bool enabled);
  enum class ChartType { Line, Bar };
  void SetChartType(ChartType type) { chart_type_ = type; }
 private:
  static constexpr double kLeftMargin = 60.0;
  static constexpr double kRightMargin = 20.0;
  static constexpr double kTopMargin = 40.0;
  static constexpr double kBottomMargin = 40.0;
  static constexpr size_t kVisiblePoints = 50;
  //static constexpr size_t kVisiblePoints = 10;
  ChartType chart_type_{ChartType::Line};
  DataSeries data_{};
  size_t view_start_index_{0};
  bool auto_scroll_{true};
  std::string current_title_{"Live Line Chart"};

  void DrawAxes(double max_value, const std::string& title);
  double GetMaxValue(const DataSeries& values) const;
  DataSeries GetVisibleData() const;
};

}  // namespace cse498

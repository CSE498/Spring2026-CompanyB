/**
 * @file InfoGraph.hpp
 * @brief A web-based graph component for displaying time-series data.
 * @author Fatima Saad
 * Supports line and bar charts, tick-based updates, and horizontal scrolling
 * to visualize recent data without compression.
 */

#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "WebCanvas.hpp"

namespace cse498 {

class InfoGraph : public WebCanvas {
 public:
  using DataSeries = std::vector<double>;
  using Color = std::tuple<int, int, int>;

  InfoGraph(int width, int height, const WebOptions& options = {});

  /// Single-series line chart
  void DrawLineChart(const DataSeries& values, const std::string& title = "");

  /// Single-color bars
  void DrawBarChart(const DataSeries& values, const std::string& title = "");
  /// Per-bar colors and per-bar labels (labels drawn under each bar).
  void DrawBarChart(const DataSeries& values,
                    const std::vector<Color>& bar_colors,
                    const std::vector<std::string>& bar_labels,
                    const std::string& title = "");

  /// Live single-series point append
  void AddDataPoint(double value, const std::string& title = "");

  /// Live multi-series point append. values[i] feeds series i.
  void AddDataPoints(const std::vector<double>& values,
                     const std::vector<Color>& series_colors,
                     const std::vector<std::string>& series_names,
                     const std::string& chart_title = "");

  void ClearData();
  void ScrollLeft();
  void ScrollRight();
  void SetAutoScroll(bool enabled);

  enum class ChartType { Line, Bar };
  void SetChartType(ChartType type) { chart_type_ = type; }

 private:
  static constexpr double kLeftMargin = 60.0;
  static constexpr double kRightMargin = 20.0;
  static constexpr double kTopMargin = 40.0;
  static constexpr double kBottomMargin = 40.0;
  static constexpr size_t kVisiblePoints = 50;

  ChartType chart_type_{ChartType::Line};
  DataSeries data_{};
  std::vector<DataSeries> extra_series_{};
  std::vector<Color> series_colors_{};
  std::vector<std::string> series_names_{};
  size_t view_start_index_{0};
  bool auto_scroll_{true};
  std::string current_title_{};

  void DrawAxes(double max_value, const std::string& title);
  double GetMaxValue(const DataSeries& values) const;
  DataSeries GetVisibleData() const;
  DataSeries GetVisibleSeries(const DataSeries& s) const;
  void RenderMultiSeries();
  void DrawLegend();
};

}  // namespace cse498

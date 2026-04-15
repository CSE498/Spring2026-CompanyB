#pragma once

#include <string>
#include <vector>

#include "WebCanvas.hpp"

namespace cse498 {

class InfoGraph : public WebCanvas {
 public:
  using DataSeries = std::vector<double>;

  InfoGraph(int width, int height, const std::string& id);

  void DrawLineChart(const DataSeries& values,
                     const std::string& title = "");

  void DrawBarChart(const DataSeries& values,
                    const std::string& title = "");

  void AddDataPoint(double value, const std::string& title = "Live Line Chart");

  void ClearData();

 private:
  static constexpr double kLeftMargin = 60.0;
  static constexpr double kRightMargin = 20.0;
  static constexpr double kTopMargin = 40.0;
  static constexpr double kBottomMargin = 40.0;
  static constexpr size_t kMaxPoints = 50;

  DataSeries data_{};

  void DrawAxes(double max_value, const std::string& title);
  double GetMaxValue(const DataSeries& values) const;
};

}  // namespace cse498
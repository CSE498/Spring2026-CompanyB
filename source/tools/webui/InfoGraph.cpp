/**
 * @class InfoGraph
 * @brief A canvas-based graph for visualizing simulation data over time.
 * @author Fatima Saad
 */
#include "InfoGraph.hpp"

#include <algorithm>
#include <string>

#include "WebOptions.hpp"

namespace cse498 {

InfoGraph::InfoGraph(int width, int height, const WebOptions& options)
    : WebCanvas(width, height, options) {
  SetBackgroundColor({245, 245, 245});
  SetPenColor({30, 30, 30});
  SetFillColor({30, 30, 30});
  SetLineWidth(2.0);
  SetFont("16px Arial");
}

double InfoGraph::GetMaxValue(const DataSeries& values) const {
  if (values.empty()) return 1.0;
  const double max_val = *std::max_element(values.begin(), values.end());
  return max_val <= 0.0 ? 1.0 : max_val;
}

InfoGraph::DataSeries InfoGraph::GetVisibleData() const {
  return GetVisibleSeries(data_);
}

InfoGraph::DataSeries InfoGraph::GetVisibleSeries(const DataSeries& s) const {
  if (s.empty()) return {};
  const size_t end_index =
      std::min(view_start_index_ + kVisiblePoints, s.size());
  return DataSeries(s.begin() + static_cast<long>(view_start_index_),
                    s.begin() + static_cast<long>(end_index));
}

void InfoGraph::DrawAxes(double max_value, const std::string& title) {
  Clear();

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());

  // Reserve right padding for the legend. Multi-series only.
  const double right_pad =
      series_names_.empty() ? kRightMargin : kRightMargin + 100.0;

  const double x0 = kLeftMargin;
  const double y0 = height - kBottomMargin;
  const double x1 = width - right_pad;
  const double y1 = kTopMargin;

  SetPenColor({255, 255, 255});
  SetLineWidth(2.0);
  DrawLine({x0, y0}, {x1, y0});
  DrawLine({x0, y0}, {x0, y1});

  if (!title.empty()) {
    SetFillColor({255, 255, 255});
    SetFont("18px Arial");
    DrawText(title, x0, 24);
  }

  SetFont("14px Arial");
  SetFillColor({255, 255, 255});
  DrawText("0", 20, y0 + 5);
  DrawText(std::to_string(static_cast<int>(max_value)), 20, y1 + 5);

  // X-axis labels:
  // First visible tick number under the origin
  // Last visible tick number under the rightmost data point
  // "tick" centered between them as the axis title
  if (!data_.empty()) {
    SetFont("13px Arial");
    const size_t end_index =
        std::min(view_start_index_ + kVisiblePoints, data_.size());
    DrawText(std::to_string(view_start_index_), x0 - 4, y0 + 16);

    const size_t visible_count = end_index - view_start_index_;
    if (visible_count > 1) {
      const double plot_width = width - kLeftMargin - right_pad;
      const double x_step =
          plot_width / static_cast<double>(visible_count - 1);
      const double last_x = kLeftMargin + (visible_count - 1) * x_step;
      DrawText(std::to_string(end_index - 1), last_x - 8, y0 + 16);
    }
    DrawText("tick", (x0 + x1) / 2.0 - 12, y0 + 30);
  }
}

void InfoGraph::DrawLegend() {
  if (series_names_.empty()) return;
  const double width = static_cast<double>(GetWidth());
  // Place the legend's left edge well to the right of the plot's right edge.
  const double x = width - kRightMargin - 70.0;
  double y = kTopMargin + 8.0;
  SetFont("13px Arial");
  for (size_t i = 0; i < series_names_.size(); ++i) {
    auto [r, g, b] = series_colors_[i];
    SetFillColor({r, g, b});
    DrawRect(x, y - 10, 14.0, 10.0, true);
    SetFillColor({255, 255, 255});
    DrawText(series_names_[i], x + 22.0, y);
    y += 18.0;
  }
}

void InfoGraph::DrawLineChart(const DataSeries& values,
                              const std::string& title) {
  if (values.empty()) {
    DrawAxes(1.0, title);
    return;
  }

  const double max_value = GetMaxValue(values);
  DrawAxes(max_value, title);

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());

  const double plot_width = width - kLeftMargin - kRightMargin;
  const double plot_height = height - kTopMargin - kBottomMargin;

  SetPenColor({50, 120, 220});
  SetLineWidth(3.0);

  if (values.size() == 1) {
    const double x = kLeftMargin;
    const double y =
        (height - kBottomMargin) - (values[0] / max_value) * plot_height;
    SetFillColor({50, 120, 220});
    DrawCircle(x, y, 4.0, true);
    return;
  }

  const double x_step = plot_width / static_cast<double>(values.size() - 1);

  for (size_t i = 1; i < values.size(); ++i) {
    const double x_prev = kLeftMargin + (i - 1) * x_step;
    const double y_prev =
        (height - kBottomMargin) - (values[i - 1] / max_value) * plot_height;

    const double x_curr = kLeftMargin + i * x_step;
    const double y_curr =
        (height - kBottomMargin) - (values[i] / max_value) * plot_height;

    DrawLine({x_prev, y_prev}, {x_curr, y_curr});
  }

  SetFillColor({50, 120, 220});
  for (size_t i = 0; i < values.size(); ++i) {
    const double x = kLeftMargin + i * x_step;
    const double y =
        (height - kBottomMargin) - (values[i] / max_value) * plot_height;
    DrawCircle(x, y, 4.0, true);
  }
}

void InfoGraph::RenderMultiSeries() {
  // Gather visible windows for series 0 (data_) and the extras.
  std::vector<DataSeries> visible;
  visible.push_back(GetVisibleSeries(data_));
  for (const auto& s : extra_series_) visible.push_back(GetVisibleSeries(s));

  // Compute global max across all visible series for shared y-axis.
  double max_value = 1.0;
  for (const auto& v : visible) {
    if (v.empty()) continue;
    double m = *std::max_element(v.begin(), v.end());
    if (m > max_value) max_value = m;
  }

  DrawAxes(max_value, current_title_);
  DrawLegend();

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());
  const double right_pad =
      series_names_.empty() ? kRightMargin : kRightMargin + 100.0;
  const double plot_width = width - kLeftMargin - right_pad;
  const double plot_height = height - kTopMargin - kBottomMargin;

  SetFont("13px Arial");
  for (size_t s = 0; s < visible.size(); ++s) {
    const DataSeries& vs = visible[s];
    if (vs.empty()) continue;
    const Color c =
        s < series_colors_.size() ? series_colors_[s] : Color{50, 120, 220};
    auto [r, g, b] = c;
    SetPenColor({r, g, b});
    SetFillColor({r, g, b});
    SetLineWidth(3.0);

    if (vs.size() == 1) {
      const double x = kLeftMargin;
      const double y =
          (height - kBottomMargin) - (vs[0] / max_value) * plot_height;
      DrawCircle(x, y, 4.0, true);
      DrawText(std::to_string(static_cast<int>(vs[0])), x + 8.0, y + 4.0);
      continue;
    }

    const double x_step = plot_width / static_cast<double>(vs.size() - 1);
    for (size_t i = 1; i < vs.size(); ++i) {
      const double x_prev = kLeftMargin + (i - 1) * x_step;
      const double y_prev =
          (height - kBottomMargin) - (vs[i - 1] / max_value) * plot_height;
      const double x_curr = kLeftMargin + i * x_step;
      const double y_curr =
          (height - kBottomMargin) - (vs[i] / max_value) * plot_height;
      DrawLine({x_prev, y_prev}, {x_curr, y_curr});
    }
    for (size_t i = 0; i < vs.size(); ++i) {
      const double x = kLeftMargin + i * x_step;
      const double y =
          (height - kBottomMargin) - (vs[i] / max_value) * plot_height;
      DrawCircle(x, y, 4.0, true);
    }

    // Latest-value label next to the rightmost point.
    const double last_x = kLeftMargin + (vs.size() - 1) * x_step;
    const double last_y =
        (height - kBottomMargin) - (vs.back() / max_value) * plot_height;
    DrawText(std::to_string(static_cast<int>(vs.back())), last_x + 8.0,
             last_y + 4.0);
  }
}

void InfoGraph::DrawBarChart(const DataSeries& values,
                             const std::string& title) {
  if (values.empty()) {
    DrawAxes(1.0, title);
    return;
  }

  const double max_value = GetMaxValue(values);
  DrawAxes(max_value, title);

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());

  const double plot_width = width - kLeftMargin - kRightMargin;
  const double plot_height = height - kTopMargin - kBottomMargin;
  const double bar_width = plot_width / static_cast<double>(values.size());

  SetFillColor({80, 170, 120});

  for (size_t i = 0; i < values.size(); ++i) {
    const double scaled_height = (values[i] / max_value) * plot_height;
    const double x = kLeftMargin + i * bar_width + 4.0;
    const double y = (height - kBottomMargin) - scaled_height;
    const double w = std::max(1.0, bar_width - 8.0);

    DrawRect(x, y, w, scaled_height, true);
  }
}

void InfoGraph::DrawBarChart(const DataSeries& values,
                             const std::vector<Color>& bar_colors,
                             const std::vector<std::string>& bar_labels,
                             const std::string& title) {
  if (values.empty()) {
    DrawAxes(1.0, title);
    return;
  }

  const double max_value = GetMaxValue(values);
  DrawAxes(max_value, title);

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());

  const double plot_width = width - kLeftMargin - kRightMargin;
  const double plot_height = height - kTopMargin - kBottomMargin;
  const double bar_width = plot_width / static_cast<double>(values.size());
  const double baseline_y = height - kBottomMargin;

  SetFont("13px Arial");
  for (size_t i = 0; i < values.size(); ++i) {
    const double scaled_height = (values[i] / max_value) * plot_height;
    const double x = kLeftMargin + i * bar_width + 4.0;
    const double y = baseline_y - scaled_height;
    const double w = std::max(1.0, bar_width - 8.0);

    const Color c =
        i < bar_colors.size() ? bar_colors[i] : Color{80, 170, 120};
    auto [r, g, b] = c;
    SetFillColor({r, g, b});
    DrawRect(x, y, w, scaled_height, true);

    // Value label above the bar.
    SetFillColor({255, 255, 255});
    DrawText(std::to_string(static_cast<int>(values[i])), x, y - 4);

    // Category label under the bar.
    if (i < bar_labels.size()) {
      DrawText(bar_labels[i], x, baseline_y + 16);
    }
  }
}

void InfoGraph::ClearData() {
  data_.clear();
  for (auto& s : extra_series_) s.clear();
  view_start_index_ = 0;
}

void InfoGraph::ScrollLeft() {
  if (view_start_index_ > 0) --view_start_index_;
  if (extra_series_.empty()) {
    DrawLineChart(GetVisibleData(), current_title_);
  } else {
    RenderMultiSeries();
  }
}

void InfoGraph::ScrollRight() {
  if (view_start_index_ + kVisiblePoints < data_.size()) ++view_start_index_;
  if (view_start_index_ + kVisiblePoints >= data_.size()) auto_scroll_ = true;
  if (extra_series_.empty()) {
    DrawLineChart(GetVisibleData(), current_title_);
  } else {
    RenderMultiSeries();
  }
}

void InfoGraph::SetAutoScroll(bool enabled) {
  auto_scroll_ = enabled;
  if (auto_scroll_ && data_.size() > kVisiblePoints) {
    view_start_index_ = data_.size() - kVisiblePoints;
  }
  if (extra_series_.empty()) {
    DrawLineChart(GetVisibleData(), current_title_);
  } else {
    RenderMultiSeries();
  }
}

void InfoGraph::AddDataPoint(double value, const std::string& title) {
  if (!title.empty()) current_title_ = title;
  data_.push_back(value);
  if (auto_scroll_ && data_.size() > kVisiblePoints) {
    view_start_index_ = data_.size() - kVisiblePoints;
  }
  if (chart_type_ == ChartType::Bar) {
    DrawBarChart(GetVisibleData(), current_title_);
  } else {
    DrawLineChart(GetVisibleData(), current_title_);
  }
}

void InfoGraph::AddDataPoints(const std::vector<double>& values,
                              const std::vector<Color>& series_colors,
                              const std::vector<std::string>& series_names,
                              const std::string& chart_title) {
  if (values.empty()) return;
  if (!chart_title.empty()) current_title_ = chart_title;

  // Reconfigure series buffers if shape changed.
  if (extra_series_.size() + 1 != values.size()) {
    extra_series_.assign(values.size() - 1, DataSeries{});
    data_.clear();
    view_start_index_ = 0;
  }
  series_colors_ = series_colors;
  series_names_ = series_names;

  // Append one value per series.
  data_.push_back(values[0]);
  for (size_t i = 1; i < values.size(); ++i) {
    extra_series_[i - 1].push_back(values[i]);
  }
  if (auto_scroll_ && data_.size() > kVisiblePoints) {
    view_start_index_ = data_.size() - kVisiblePoints;
  }
  RenderMultiSeries();
}

}  // namespace cse498

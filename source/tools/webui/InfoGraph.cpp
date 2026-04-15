#include "InfoGraph.hpp"

#include <algorithm>
#include <string>

namespace cse498 {

InfoGraph::InfoGraph(int width, int height, const std::string& id)
    : WebCanvas(width, height, id) {
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

void InfoGraph::DrawAxes(double max_value, const std::string& title) {
  Clear();

  const double width = static_cast<double>(GetWidth());
  const double height = static_cast<double>(GetHeight());

  const double x0 = kLeftMargin;
  const double y0 = height - kBottomMargin;
  const double x1 = width - kRightMargin;
  const double y1 = kTopMargin;

  SetPenColor({40, 40, 40});
  SetLineWidth(2.0);
  DrawLine({x0, y0}, {x1, y0});
  DrawLine({x0, y0}, {x0, y1});

  if (!title.empty()) {
    SetFillColor({20, 20, 20});
    SetFont("18px Arial");
    DrawText(title, x0, 24);
  }

  SetFont("14px Arial");
  SetFillColor({20, 20, 20});
  DrawText("0", 20, y0 + 5);
  DrawText(std::to_string(static_cast<int>(max_value)), 20, y1 + 5);
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

void InfoGraph::AddDataPoint(double value, const std::string& title) {
  data_.push_back(value);

  if (data_.size() > kMaxPoints) {
    data_.erase(data_.begin());
  }

  DrawLineChart(data_, title);
}

void InfoGraph::ClearData() {
  data_.clear();
  Clear();
}

}  // namespace cse498
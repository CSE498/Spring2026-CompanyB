/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Web entry point for the Emscripten build.
 **/

#include <cmath>
#include <memory>
#include <vector>

#include <emscripten.h>

#include "InfoGraph.hpp"
#include "WebLayout.hpp"

using namespace cse498;

// Globals kept alive for the duration of the page
static std::unique_ptr<WebLayout> layout;
static std::shared_ptr<InfoGraph> line_graph;
static std::shared_ptr<InfoGraph> bar_graph;

static double t = 0.0;
static std::vector<double> bar_data{8, 4, 10, 6, 12};

void UpdateGraphs() {
  const double new_value = 10.0 + 5.0 * std::sin(t);
  line_graph->AddDataPoint(new_value, "Live Line Chart");

  bar_graph->DrawBarChart(bar_data, "Bar Chart Demo");

  t += 0.2;
}

int main() {
  layout = std::make_unique<WebLayout>("app-layout");
  layout->SetDirection("column")
      .SetAlignItems("center")
      .SetJustifyContent("center")
      .SetGap("24px");

  line_graph = std::make_shared<InfoGraph>(700, 280, "line-graph");
  bar_graph = std::make_shared<InfoGraph>(700, 280, "bar-graph");

  layout->AddChild(line_graph);
  layout->AddChild(bar_graph);

  UpdateGraphs();

  emscripten_set_main_loop(UpdateGraphs, 2, true);

  return 0;
}
#pragma once

#include "BenchmarkTypes.hpp"

namespace cse498::benchmarking {

class MetricCollector {
  public:
   MetricCollector() = default;

   ~MetricCollector() = default;

   MetricCollectorError BENCH_START();

   MetricCollectorError BENCH_STOP();

   MetricSample GetSample();
};

}
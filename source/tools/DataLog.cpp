#include "DataLog.hpp"

#include <algorithm>

#include "GlobalClock.hpp"

namespace cse498 {

// Took AI's assistance to help with the syntax of this function.

// Currently, we're calculating stats based on "duration" 
// field in the JSON data, but this can be easily modified to 
// accommodate other fields as needed (will be included in new module specs)
void DataLog::AddEntry(const nlohmann::json &data) {
  // Create log entry with automatic timestamp
  nlohmann::json logEntry = data;
  logEntry["timestamp"] = GlobalClock::GetTime();

  // Store the complete entry
  mEntries.push_back(logEntry);

  // Update statistics based on duration field
  // Note: Only non-negative durations are included in statistics, as negative
  // durations do not make physical sense in a simulation context.
  if (data.contains("duration") && data["duration"].is_number()) {
    double duration = data["duration"].get<double>();
    
    // Only process non-negative durations
    if (duration >= 0.0) {
      mRunningSum += duration;

      if (mCount == 0) {
        // First entry with duration
        mMinValue = duration;
        mMaxValue = duration;
      } else {
        // Subsequent entries
        if (duration < mMinValue) {
          mMinValue = duration;
        }
        if (duration > mMaxValue) {
          mMaxValue = duration;
        }
      }

      // Increment count
      mCount++;
      mDurations.insert(duration);
      mMedianIsValid = false;  // Invalidate cache when new data is added
    }
  }
}

std::optional<double> DataLog::GetMean() const {
  if (mCount == 0) {
    return std::nullopt;
  }
  return mRunningSum / static_cast<double>(mCount);
}

std::optional<double> DataLog::GetMedian() const {
  if (mCount == 0) {
    return std::nullopt;
  }

  // Check if cached median is valid
  if (mMedianIsValid) {
    return mMedianCache;
  }
  
  size_t size = mDurations.size();
  if (size % 2 == 0) {
    // Even number of elements - return average of middle two
    auto it = mDurations.begin();
    std::advance(it, size / 2 - 1);
    double first = *it;
    std::advance(it, 1);
    double second = *it;
    mMedianCache = (first + second) / 2.0;
  } else {
    // Odd number of elements - return middle element
    auto it = mDurations.begin();
    std::advance(it, size / 2);
    mMedianCache = *it;
  }

  mMedianIsValid = true;
  return mMedianCache;
}

std::optional<double> DataLog::GetMin() const {
  if (mCount == 0) {
    return std::nullopt;
  }
  return mMinValue;
}

std::optional<double> DataLog::GetMax() const {
  if (mCount == 0) {
    return std::nullopt;
  }
  return mMaxValue;
}

size_t DataLog::GetCount() const { return mCount; }

const std::vector<nlohmann::json> &DataLog::GetEntries() const {
  return mEntries;
}

void DataLog::Reset() {
  mEntries.clear();
  mRunningSum = 0.0;
  mMinValue = 0.0;
  mMaxValue = 0.0;
  mCount = 0;
  mDurations.clear();
  mMedianCache = 0.0;
  mMedianIsValid = false;
}

}  // namespace cse498

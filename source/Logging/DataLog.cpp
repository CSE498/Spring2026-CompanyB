#include "DataLog.hpp"
#include "Timer.hpp"
#include <algorithm>
#include <cmath>

namespace cse498 {

    DataLog::DataLog(Timer& timer)
        : mTimer(timer), mRunningSum(0.0), mMinValue(0.0), mMaxValue(0.0), 
          mCount(0), mHasData(false) {}

    void DataLog::AddEntry(const nlohmann::json& data) {
        // Create log entry with automatic timestamp
        nlohmann::json logEntry = data;
        logEntry["timestamp"] = mTimer.GetGlobalTime();
        
        // Store the complete entry
        mEntries.push_back(logEntry);
        
        // Update statistics based on duration field
        if (data.contains("duration") && data["duration"].is_number()) {
            double duration = data["duration"].get<double>();
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
            mHasData = true;
        }
    }

    double DataLog::GetMean() const {
        if (mCount == 0) {
            return 0.0;
        }
        return mRunningSum / static_cast<double>(mCount);
    }

    double DataLog::GetMedian() const {
        if (mCount == 0) {
            return 0.0;
        }
        
        // Create a vector of durations sorted for median calculation
        std::vector<double> durations;
        for (const auto& entry : mEntries) {
            if (entry.contains("duration") && entry["duration"].is_number()) {
                durations.push_back(entry["duration"].get<double>());
            }
        }
        
        // Sort the durations
        std::sort(durations.begin(), durations.end());
        
        // Calculate median
        size_t size = durations.size();
        if (size % 2 == 0) {
            // Even number of elements - return average of middle two
            return (durations[size / 2 - 1] + durations[size / 2]) / 2.0;
        } else {
            // Odd number of elements - return middle element
            return durations[size / 2];
        }
    }

    double DataLog::GetMin() const {
        if (mCount == 0) {
            return 0.0;
        }
        return mMinValue;
    }

    double DataLog::GetMax() const {
        if (mCount == 0) {
            return 0.0;
        }
        return mMaxValue;
    }

    size_t DataLog::GetCount() const {
        return mCount;
    }

    const std::vector<nlohmann::json>& DataLog::GetEntries() const {
        return mEntries;
    }

    void DataLog::Reset() {
        mEntries.clear();
        mRunningSum = 0.0;
        mMinValue = 0.0;
        mMaxValue = 0.0;
        mCount = 0;
        mHasData = false;
    }

}

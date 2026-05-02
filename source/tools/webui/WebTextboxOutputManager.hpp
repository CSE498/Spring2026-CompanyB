/**
 * @file WebTextboxOutputManager.hpp
 * @brief IOutputManager that routes log messages to a WebTextbox.
 **/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../Interfaces/IOutputManager.hpp"
#include "WebTextbox.hpp"
#include "nlohmann/json.hpp"

namespace cse498 {

class WebTextboxOutputManager : public IOutputManager {
 private:
  std::shared_ptr<WebTextbox> mTextbox;
  LogLevel mCurrentLevel = LogLevel::Normal;
  nlohmann::json mBufferedLog = nlohmann::json::object();
  std::vector<nlohmann::json> mPendingActionEvents;

  [[nodiscard]] bool ShouldLog(LogLevel messageLevel) const noexcept {
    if (mCurrentLevel == LogLevel::Silent) return false;
    return static_cast<int>(messageLevel) <= static_cast<int>(mCurrentLevel);
  }

  static const char* LevelTag(LogLevel level) noexcept {
    switch (level) {
      case LogLevel::Normal:
        return "INFO";
      case LogLevel::Verbose:
        return "WARN";
      case LogLevel::Debug:
        return "ERROR";
      case LogLevel::Silent:
        return "INFO";
    }
    return "INFO";
  }

 public:
  explicit WebTextboxOutputManager(std::shared_ptr<WebTextbox> textbox,
                                   LogLevel level = LogLevel::Normal)
      : mTextbox(std::move(textbox)), mCurrentLevel(level) {}

  ~WebTextboxOutputManager() override = default;

  WebTextboxOutputManager(const WebTextboxOutputManager&) = delete;
  WebTextboxOutputManager& operator=(const WebTextboxOutputManager&) = delete;

  bool SetOutputFile(const std::string& /*path*/) override { return true; }

  void LogMessage(LogLevel level, const std::string& message) override {
    if (!ShouldLog(level) || !mTextbox) return;
    mTextbox->AppendLine(message, LevelTag(level));
    mBufferedLog["messages"].push_back({
        {"level", LevelTag(level)},
        {"message", message},
    });
  }

  void LogTagged(const std::string& tag, const std::string& message) {
    if (!mTextbox) return;
    mTextbox->AppendLine(message, tag);
    mBufferedLog["messages"].push_back({
        {"level", tag},
        {"message", message},
    });
  }

  void SetLogLevel(LogLevel level) noexcept override { mCurrentLevel = level; }

  bool Flush() override { return true; }

  const nlohmann::json& GetBufferedLog() noexcept override {
    mBufferedLog["action_events"] = mPendingActionEvents;
    return mBufferedLog;
  }

  void WriteActionEvents(const std::vector<nlohmann::json>& events) override {
    mPendingActionEvents.insert(mPendingActionEvents.end(), events.begin(),
                                events.end());
  }

  void Clear() noexcept {
    mBufferedLog = nlohmann::json::object();
    mPendingActionEvents.clear();
  }
};

}  // namespace cse498

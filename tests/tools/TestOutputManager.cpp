#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../../source/tools/DataLog.hpp"
#include "../../source/tools/OutputManager.hpp"

using namespace cse498;
/*
TEST_CASE("OutputManager sets log level and output file", "[OutputManager]") {
        OutputManager manager;
        manager.SetLogLevel(LogLevel::Verbose);
        manager.SetOutputFile("test_output.json");
        manager.LogMessage(LogLevel::Verbose, "Verbose message");
        manager.WriteSimulationOutput(DataLog());
        std::ifstream in("test_output.json");
        REQUIRE(in.good());
        in.close();
        std::remove("test_output.json");
}

TEST_CASE("OutputManager logs messages according to verbosity",
"[OutputManager]") { OutputManager manager;
        manager.SetLogLevel(LogLevel::Normal);
        manager.SetOutputFile("test_output.json");
        manager.LogMessage(LogLevel::Verbose, "Should not appear");
        manager.LogMessage(LogLevel::Normal, "Should appear");
        manager.WriteSimulationOutput(DataLog());
        std::ifstream in("test_output.json");
        std::string line;
        bool found = false;
        while (std::getline(in, line)) {
                if (line.find("Should appear") != std::string::npos) found =
true; if (line.find("Should not appear") != std::string::npos) found = false;
        }
        REQUIRE(found);
        in.close();
        std::remove("test_output.json");
}

TEST_CASE("OutputManager silent mode disables logging", "[OutputManager]") {
        OutputManager manager;
        manager.SetLogLevel(LogLevel::Silent);
        manager.SetOutputFile("test_output.json");
        manager.LogMessage(LogLevel::Normal, "Should not log");
        manager.WriteSimulationOutput(DataLog());
        std::ifstream in("test_output.json");
        std::string content((std::istreambuf_iterator<char>(in)),
std::istreambuf_iterator<char>());
        // In silent mode, no messages should be present in the output file
        REQUIRE(content.find("messages") == std::string::npos);
        in.close();
        std::remove("test_output.json");
}

TEST_CASE("OutputManager writes simulation output with statistics",
"[OutputManager]") { DataLog log; log.AddEntry({{"agentId", "agent_1"},
{"actionType", "move"}, {"duration", 1.0}, {"summary", "Move"}});
        log.AddEntry({{"agentId", "agent_2"}, {"actionType", "turn"},
{"duration", 2.0}, {"summary", "Turn"}}); log.AddEntry({{"agentId", "agent_3"},
{"actionType", "move"}, {"duration", 3.0}, {"summary", "Move again"}});
        OutputManager manager;
        manager.SetOutputFile("test_output.json");
        manager.WriteSimulationOutput(log);
        std::ifstream in("test_output.json");
        std::string content((std::istreambuf_iterator<char>(in)),
std::istreambuf_iterator<char>()); REQUIRE(content.find("entries") !=
std::string::npos); REQUIRE(content.find("statistics") != std::string::npos);
        REQUIRE(content.find("mean") != std::string::npos);
        in.close();
        std::remove("test_output.json");
}
*/

namespace {

void remove_if_exists(const std::string& path) {
  std::error_code ec;
  std::filesystem::remove(path, ec);
}

}  // namespace

TEST_CASE("Constructor sets default path but does not create file until Flush",
          "[OutputManager]") {
  namespace fs = std::filesystem;
  const fs::path defaultPath =
      fs::current_path() / "logs" / "simulation_log.json";
  remove_if_exists(defaultPath.string());
  OutputManager manager(LogLevel::Normal);
  REQUIRE_FALSE(fs::exists(defaultPath));
  REQUIRE(manager.Flush());
  REQUIRE(fs::exists(defaultPath));
  remove_if_exists(defaultPath.string());
  const fs::path logsDir = fs::current_path() / "logs";
  if (fs::exists(logsDir) && fs::is_empty(logsDir)) {
    std::error_code ec;
    fs::remove(logsDir, ec);
  }
}

TEST_CASE("SetOutputFile switches output path", "[OutputManager]") {
  const std::string path = "test_om_set_output.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Normal, "after switch");
  REQUIRE(manager.Flush());
  std::ifstream in(path);
  REQUIRE(in.good());
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  REQUIRE(content.find("after switch") != std::string::npos);
  in.close();
  remove_if_exists(path);
}

TEST_CASE("Flush fails when output path parent is not a directory",
          "[OutputManager]") {
  namespace fs = std::filesystem;
  const fs::path blocker =
      fs::current_path() / "test_om_parent_is_file_not_dir";
  remove_if_exists(blocker.string());
  {
    std::ofstream out(blocker);
    out << "block";
  }
  const std::string badPath = (blocker / "nested.json").string();
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(badPath));
  REQUIRE_FALSE(manager.Flush());
  remove_if_exists(blocker.string());
}

TEST_CASE("SetLogLevel filters messages by level", "[OutputManager]") {
  const std::string path = "test_om_loglevel.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Verbose, "verbose filtered");
  manager.LogMessage(LogLevel::Normal, "normal kept");
  REQUIRE(manager.Flush());
  const auto& buf = manager.GetBufferedLog();
  const std::string dumped = buf.dump();
  REQUIRE(dumped.find("normal kept") != std::string::npos);
  REQUIRE(dumped.find("verbose filtered") == std::string::npos);
  remove_if_exists(path);
}

TEST_CASE("LogMessage writes to buffer at Verbose threshold",
          "[OutputManager]") {
  const std::string path = "test_om_logmessage.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Verbose);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Verbose, "with level");
  manager.LogMessage(LogLevel::Normal, "normal line");
  REQUIRE(manager.Flush());
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf.contains("messages"));
  REQUIRE(buf["messages"].is_array());
  REQUIRE(buf["messages"].size() >= 2);
  remove_if_exists(path);
}

TEST_CASE("LogMessage with LogLevel::Silent is never persisted",
          "[OutputManager]") {
  const std::string path = "test_om_msg_level_silent.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Silent, "silent message level");
  REQUIRE(manager.Flush());
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf.dump().find("silent message level") == std::string::npos);
  std::ifstream in(path);
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  in.close();
  REQUIRE(content.find("silent message level") == std::string::npos);
  remove_if_exists(path);
}

TEST_CASE("Flush prints log saved message on success", "[OutputManager]") {
  const std::string path = "test_om_flush_message.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Normal, "x");
  std::ostringstream captured;
  auto* old = std::clog.rdbuf(captured.rdbuf());
  const bool ok = manager.Flush();
  std::clog.rdbuf(old);
  REQUIRE(ok);
  REQUIRE(captured.str().find("log saved in") != std::string::npos);
  REQUIRE(captured.str().find(path) != std::string::npos);
  remove_if_exists(path);
}

TEST_CASE("Flush returns false when file cannot be opened", "[OutputManager]") {
  namespace fs = std::filesystem;
  const fs::path blocker =
      fs::current_path() / "test_om_flush_no_stream_blocker";
  remove_if_exists(blocker.string());
  {
    std::ofstream out(blocker);
    out << "x";
  }
  const std::string badPath = (blocker / "out.json").string();
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(badPath));
  REQUIRE_FALSE(manager.Flush());
  remove_if_exists(blocker.string());
}

TEST_CASE("WriteSimulationOutput writes entries and statistics",
          "[OutputManager]") {
  const std::string path = "test_om_simout.json";
  remove_if_exists(path);
  DataLog log;
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.WriteSimulationOutput(log);
  std::ifstream in(path);
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  REQUIRE(content.find("\"entries\"") != std::string::npos);
  REQUIRE(content.find("\"statistics\"") != std::string::npos);
  REQUIRE(content.find("\"count\"") != std::string::npos);
  in.close();
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf.contains("entries"));
  REQUIRE(buf.contains("statistics"));
  remove_if_exists(path);
}

TEST_CASE("WriteSimulationOutput still writes entries when Silent",
          "[OutputManager]") {
  const std::string path = "test_om_silent_simout.json";
  remove_if_exists(path);
  DataLog log;
  OutputManager manager(LogLevel::Silent);
  REQUIRE(manager.SetOutputFile(path));
  manager.WriteSimulationOutput(log);
  std::ifstream in(path);
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  in.close();
  REQUIRE(content.find("\"entries\"") != std::string::npos);
  REQUIRE(content.find("\"statistics\"") != std::string::npos);
  remove_if_exists(path);
}

TEST_CASE("GetBufferedLog reflects in-memory JSON", "[OutputManager]") {
  const std::string path = "test_om_buffered.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.LogMessage(LogLevel::Normal, "buffer check");
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf["messages"].is_array());
  bool found = false;
  for (const auto& item : buf["messages"]) {
    if (item["text"] == "buffer check") {
      found = true;
    }
  }
  REQUIRE(found);
  remove_if_exists(path);
}

TEST_CASE("WriteActionEvents buffers in memory; file written only on Flush",
          "[OutputManager]") {
  const std::string path = "test_om_actions.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  namespace fs = std::filesystem;
  REQUIRE_FALSE(fs::exists(path));

  std::vector<ActionEventBase> events;
  events.push_back(ActionEventBase{"agent_a", "move", LogLevel::Normal, 42ULL});
  events.push_back(
      ActionEventBase{"agent_b", "turn", LogLevel::Verbose, 99ULL});
  manager.WriteActionEvents(events);

  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf["action_events"].size() == 2);
  REQUIRE(buf["action_events"][0]["agentId"] == "agent_a");

  REQUIRE(manager.Flush());
  REQUIRE(fs::exists(path));
  std::ifstream in(path);
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  REQUIRE(content.find("action_events") != std::string::npos);
  REQUIRE(content.find("agent_a") != std::string::npos);
  in.close();
  remove_if_exists(path);
}

TEST_CASE(
    "WriteActionEvents empty vector yields empty action_events until Flush",
    "[OutputManager]") {
  const std::string path = "test_om_actions_empty.json";
  remove_if_exists(path);
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(path));
  manager.WriteActionEvents({});
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf["action_events"].is_array());
  REQUIRE(buf["action_events"].empty());
  REQUIRE(manager.Flush());
  remove_if_exists(path);
}

TEST_CASE("WriteActionEvents accumulates across calls", "[OutputManager]") {
  OutputManager manager(LogLevel::Normal);
  manager.WriteActionEvents(
      {ActionEventBase{"a", "m", LogLevel::Normal, 1ULL}});
  manager.WriteActionEvents(
      {ActionEventBase{"b", "t", LogLevel::Normal, 2ULL}});
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf["action_events"].size() == 2);
}

TEST_CASE("Buffered action events survive failed Flush", "[OutputManager]") {
  namespace fs = std::filesystem;
  const fs::path blocker =
      fs::current_path() / "test_om_wae_flush_fail_blocker";
  remove_if_exists(blocker.string());
  {
    std::ofstream out(blocker);
    out << "x";
  }
  const std::string badPath = (blocker / "actions.json").string();
  OutputManager manager(LogLevel::Normal);
  REQUIRE(manager.SetOutputFile(badPath));
  manager.WriteActionEvents(
      {ActionEventBase{"a", "b", LogLevel::Normal, 1ULL}});
  REQUIRE_FALSE(manager.Flush());
  const auto& buf = manager.GetBufferedLog();
  REQUIRE(buf["action_events"].size() == 1);
  remove_if_exists(blocker.string());
}

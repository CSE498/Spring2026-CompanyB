#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/tools/OutputManager.hpp"
#include "../../source/tools/DataLog.hpp"
#include <fstream>
#include <cstdio>

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

TEST_CASE("OutputManager logs messages according to verbosity", "[OutputManager]") {
	OutputManager manager;
	manager.SetLogLevel(LogLevel::Normal);
	manager.SetOutputFile("test_output.json");
	manager.LogMessage(LogLevel::Verbose, "Should not appear");
	manager.LogMessage(LogLevel::Normal, "Should appear");
	manager.WriteSimulationOutput(DataLog());
	std::ifstream in("test_output.json");
	std::string line;
	bool found = false;
	while (std::getline(in, line)) {
		if (line.find("Should appear") != std::string::npos) found = true;
		if (line.find("Should not appear") != std::string::npos) found = false;
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
	std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	// In silent mode, no messages should be present in the output file
	REQUIRE(content.find("messages") == std::string::npos);
	in.close();
	std::remove("test_output.json");
}

TEST_CASE("OutputManager writes simulation output with statistics", "[OutputManager]") {
	DataLog log;
	log.AddEntry({{"agentId", "agent_1"}, {"actionType", "move"}, {"duration", 1.0}, {"summary", "Move"}});
	log.AddEntry({{"agentId", "agent_2"}, {"actionType", "turn"}, {"duration", 2.0}, {"summary", "Turn"}});
	log.AddEntry({{"agentId", "agent_3"}, {"actionType", "move"}, {"duration", 3.0}, {"summary", "Move again"}});
	OutputManager manager;
	manager.SetOutputFile("test_output.json");
	manager.WriteSimulationOutput(log);
	std::ifstream in("test_output.json");
	std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	REQUIRE(content.find("entries") != std::string::npos);
	REQUIRE(content.find("statistics") != std::string::npos);
	REQUIRE(content.find("mean") != std::string::npos);
	in.close();
	std::remove("test_output.json");
}
*/
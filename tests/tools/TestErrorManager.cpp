#include <catch2/catch.hpp>
#include "Interfaces/gui/ErrorManager.hpp"

TEST_CASE("ErrorManager handles warnings correctly", "[ErrorManager][warning]") {
    ErrorManager em;

    SECTION("Empty warning message throws invalid_argument") {
        REQUIRE_THROWS_AS(em.reportWarning(""), std::invalid_argument);
    }

    SECTION("Valid warning message does not throw") {
        REQUIRE_NOTHROW(em.reportWarning("This is fine"));
    }

    SECTION("Custom warning handler is invoked") {
        bool handlerCalled = false;
        std::string capturedMessage;
        ErrorManager::Severity capturedSeverity;

        em.setWarningHandler([&](const std::string& msg, ErrorManager::Severity sev) {
            handlerCalled = true;
            capturedMessage = msg;
            capturedSeverity = sev;
        });

        em.reportWarning("test warning");

        REQUIRE(handlerCalled);
        REQUIRE(capturedMessage == "test warning");
        REQUIRE(capturedSeverity == ErrorManager::Severity::Warning);
    }
}

TEST_CASE("ErrorManager handles errors correctly", "[ErrorManager][error]") {
    ErrorManager em;

    SECTION("Empty error message throws invalid_argument") {
        REQUIRE_THROWS_AS(em.reportError(""), std::invalid_argument);
    }

    SECTION("Valid error message does not throw") {
        REQUIRE_NOTHROW(em.reportError("This is fine"));
    }

    SECTION("Custom error handler is invoked") {
        bool handlerCalled = false;
        std::string capturedMessage;
        ErrorManager::Severity capturedSeverity;

        em.setErrorHandler([&](const std::string& msg, ErrorManager::Severity sev) {
            handlerCalled = true;
            capturedMessage = msg;
            capturedSeverity = sev;
        });

        em.reportError("test error");

        REQUIRE(handlerCalled);
        REQUIRE(capturedMessage == "test error");
        REQUIRE(capturedSeverity == ErrorManager::Severity::Error);
    }
}

TEST_CASE("ErrorManager handles fatal errors correctly", "[ErrorManager][fatal]") {
    ErrorManager em;

    SECTION("Fatal error throws runtime_error") {
        REQUIRE_THROWS_AS(em.reportFatal("Fatal!"), std::runtime_error);
    }

    SECTION("Empty fatal message throws invalid_argument") {
        REQUIRE_THROWS_AS(em.reportFatal(""), std::invalid_argument);
    }

    SECTION("Custom fatal handler is called before exception is thrown") {
        bool handlerCalled = false;

        em.setFatalHandler([&](const std::string& msg, ErrorManager::Severity sev) {
            handlerCalled = true;
        });

        REQUIRE_THROWS_AS(em.reportFatal("fatal crash"), std::runtime_error);
        REQUIRE(handlerCalled);
    }
}

TEST_CASE("ErrorManager configuration", "[ErrorManager][config]") {
    ErrorManager em;

    SECTION("Output mode can be set via constructor") {
        ErrorManager guiEm(ErrorManager::OutputMode::GUI);
        REQUIRE_NOTHROW(guiEm.reportWarning("GUI warning"));
    }

    SECTION("Output mode can be changed at runtime") {
        REQUIRE_NOTHROW(em.setOutputMode(ErrorManager::OutputMode::Web));
        REQUIRE_NOTHROW(em.reportWarning("Web warning"));
    }

    SECTION("clearHandlers removes all custom handlers") {
        bool called = false;
        em.setWarningHandler([&](const std::string&, ErrorManager::Severity) {
            called = true;
        });

        em.clearHandlers();
        em.reportWarning("should use default handler");

        REQUIRE_FALSE(called);
    }
}
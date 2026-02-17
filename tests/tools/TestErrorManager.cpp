#include "TestErrorManager.hpp"
#include <QtTest/QtTest>

void TestErrorManager::testWarning() {
    ErrorManager em;
    // Should throw invalid_argument for empty message
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, em.reportWarning(""));
    // Should NOT throw for a valid message
    em.reportWarning("This is a test warning");
}

void TestErrorManager::testError() {
    ErrorManager em;
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, em.reportError(""));
    em.reportError("This is a test error");
}

void TestErrorManager::testFatal() {
    ErrorManager em;
    // Fatal should throw runtime_error
    QVERIFY_THROWS_EXCEPTION(std::runtime_error, em.reportFatal("Fatal! (also a test)"));
}

// Main entry point for Qt Test
QTEST_MAIN(TestErrorManager);

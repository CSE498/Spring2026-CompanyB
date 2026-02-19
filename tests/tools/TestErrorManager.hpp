#pragma once

#include <QtTest/QtTest>
#include "ErrorManager.hpp"

class TestErrorManager : public QObject {
    Q_OBJECT

private slots:
    void testWarning();
    void testError();
    void testFatal();
};

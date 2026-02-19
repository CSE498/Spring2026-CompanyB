#include "ErrorManager.hpp"
#include <iostream>
#include <stdexcept>

ErrorManager::ErrorManager()
    : m_outputMode(OutputMode::Terminal) {}

ErrorManager::ErrorManager(OutputMode mode)
    : m_outputMode(mode) {}

void ErrorManager::reportWarning(const std::string& message) {
    if (message.empty()) {
        throw std::invalid_argument("Warning message cannot be empty");
    }
    dispatch(message, Severity::Warning);
}

void ErrorManager::reportError(const std::string& message) {
    if (message.empty()) {
        throw std::invalid_argument("Error message cannot be empty");
    }
    dispatch(message, Severity::Error);
}

void ErrorManager::reportFatal(const std::string& message) {
    if (message.empty()) {
        throw std::invalid_argument("Fatal error message cannot be empty");
    }
    dispatch(message, Severity::Fatal);
}

void ErrorManager::setWarningHandler(ErrorHandler handler) {
    m_warningHandler = handler;
}

void ErrorManager::setErrorHandler(ErrorHandler handler) {
    m_errorHandler = handler;
}

void ErrorManager::setFatalHandler(ErrorHandler handler) {
    m_fatalHandler = handler;
}

void ErrorManager::setOutputMode(OutputMode mode) {
    m_outputMode = mode;
}

void ErrorManager::clearHandlers() {
    m_warningHandler = nullptr;
    m_errorHandler = nullptr;
    m_fatalHandler = nullptr;
}

void ErrorManager::dispatch(const std::string& message, Severity severity) {
    ErrorHandler* handler = nullptr;

    switch (severity) {
        case Severity::Warning:
            handler = &m_warningHandler;
            break;
        case Severity::Error:
            handler = &m_errorHandler;
            break;
        case Severity::Fatal:
            handler = &m_fatalHandler;
            break;
    }

    if (handler && *handler) {
        (*handler)(message, severity);
    } else {
        defaultHandler(message, severity);
    }

    if (severity == Severity::Fatal) {
        // Fatal errors stop execution by default
        throw std::runtime_error(message);
    }
}

void ErrorManager::defaultHandler(const std::string& message, Severity severity) {
    std::ostream& out = std::cerr;

    switch (severity) {
        case Severity::Warning:
            out << "[Warning] ";
            break;
        case Severity::Error:
            out << "[Error] ";
            break;
        case Severity::Fatal:
            out << "[Fatal] ";
            break;
    }

    out << message << std::endl;
}

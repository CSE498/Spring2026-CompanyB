#ifndef ERRORMANAGER_HPP
#define ERRORMANAGER_HPP

#include <functional>
#include <string>
#include <ostream>

class ErrorManager {
public:
    // Severity levels for errors
    enum class Severity {
        Warning,
        Error,
        Fatal
    };

    // Output environment
    enum class OutputMode {
        Terminal,
        GUI,
        Web,
        Custom
    };

    using ErrorHandler = std::function<void(const std::string&, Severity)>;

    // Constructors
    ErrorManager();
    explicit ErrorManager(OutputMode mode);

    // Reporting functions
    void reportWarning(const std::string& message);
    void reportError(const std::string& message);
    void reportFatal(const std::string& message);

    // Configuration
    void setErrorHandler(ErrorHandler handler);
    void setWarningHandler(ErrorHandler handler);
    void setFatalHandler(ErrorHandler handler);

    void setOutputMode(OutputMode mode);
    void clearHandlers();

private:
    // Internal dispatch
    void dispatch(const std::string& message, Severity severity);

    // Default behaviors
    void defaultHandler(const std::string& message, Severity severity);

    // State
    OutputMode m_outputMode;

    ErrorHandler m_warningHandler;
    ErrorHandler m_errorHandler;
    ErrorHandler m_fatalHandler;
};

#endif // ERRORMANAGER_HPP
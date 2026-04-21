#pragma once

#include <functional>
#include <ostream>
#include <string>

namespace cse498 {

class ErrorManager {
 public:
  // Severity levels for errors
  enum class Severity { Warning, Error, Fatal };

  // Output environment
  enum class OutputMode { Terminal, GUI, Web, Custom };

  using ErrorHandler = std::function<void(const std::string&, Severity)>;

  // Constructors
  ErrorManager() = default;
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
  [[nodiscard]] OutputMode getOutputMode() const;
  void clearHandlers();

 private:
  // Internal dispatch
  void dispatch(const std::string& message, Severity severity);

  // Retrieve the handler for a given severity (may be empty)
  const ErrorHandler& getHandler(Severity severity) const;

  // Default behaviors
  void defaultHandler(const std::string& message, Severity severity) const;

  // State
  OutputMode m_outputMode = OutputMode::Terminal;

  ErrorHandler m_warningHandler;
  ErrorHandler m_errorHandler;
  ErrorHandler m_fatalHandler;
};

}  // namespace cse498
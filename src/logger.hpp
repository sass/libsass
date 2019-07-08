#ifndef SASS_LOGGER_HPP
#define SASS_LOGGER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <sstream>
#include "strings.hpp"
#include "constants.hpp"
#include "backtrace.hpp"
#include "callstack.hpp"
#include "terminal.hpp"

namespace Sass {

  // The logger belongs to context
  class Logger {

  public:

    // Epsilon for precision
    double epsilon;

    // warning buffers
    sass::ostream logstrm;

    // The current callstack
    BackTraces callStack;

    // Available columns on tty
    size_t columns;

    // Flag for unicode and/or color
    enum SassLoggerStyle style;

    // Append the pstate if not already there
    // Only call this right before throwing errors
    void addFinalStackTrace(const SourceSpan& pstate)
    {
      if (callStack.empty()) {
        callStack.push_back(pstate);
      }
      else {
        BackTrace& trace(callStack.back());
        if (!(trace.pstate == pstate)) {
          callStack.push_back(pstate);
        }
      }
    }
    // EO addFinalStackTrace

  private:

    // Split the line to three parts for error reporting.
    // The `lhs_len` is the position where the error reporting
    // should start and `mid_len` how many characters should be
    // highlighted. It will fill the strings `lhs`, `mid` and `rhs`
    // accordingly so the total length is not bigger than `columns`.
    // Strings might be shortened and ellipsis are added as needed.
    void splitLine(
      sass::string line,
      size_t lhs_len,
      size_t mid_len,
      size_t columns,
      sass::string& lhs,
      sass::string& mid,
      sass::string& rhs);

    // Helper function to ease color output. Returns the
    // passed color if color output is enable, otherwise
    // it will simply return an empty string.
    inline const char* getColor(const char* color) {
      if (style & SASS_LOGGER_COLOR) {
        return color;
      }
      return Constants::String::empty;
    }

    // Write warning header to error stream
    void writeWarnHead(
      bool deprecation = false);
    
    // Print to stderr stream
    void printWarning(
      const sass::string& message,
      const SourceSpan& pstate,
      bool deprecation = false);

    void printSourceSpan(
      SourceSpan pstate,
      sass::ostream& stream,
      enum SassLoggerStyle logstyle);

  public:

    void writeStackTraces(sass::ostream& os,
													StackTraces traces,
													sass::string indent = "  ",
													bool showPos = true,
													size_t showTraces = sass::string::npos);

  public:

    Logger(enum SassLoggerStyle style = SASS_LOGGER_ASCII_MONO,
      int precision = SassDefaultPrecision, size_t columns = NPOS);

    void setLogPrecision(int precision);

    void setLogStyle(enum SassLoggerStyle style);

    void addWarning(const sass::string& message);

    void addDebug(const sass::string& message, const SourceSpan& pstate);

    void addWarning(const sass::string& message, const SourceSpan& pstate)
    {
      printWarning(message, pstate, false);
    }

    void addDeprecation(const sass::string& message, const SourceSpan& pstate)
    {
      printWarning(message, pstate, true);
    }

  public:

    operator BackTraces&() {
      return callStack; }

    // virtual ~Logger() {};

  };
  /*
  class StdLogger :
    public Logger {

  public:

    StdLogger(int precision, enum SassLoggerStyle style = SASS_LOGGER_AUTO);

    void warn(sass::string message) const override final;
    void debug(sass::string message) const override final;
    void error(sass::string message) const override final;

    ~StdLogger() override final {};

  };

  class NullLogger :
    public Logger {

  public:

    void warn(sass::string message) const override final {};
    void debug(sass::string message) const override final {};
    void error(sass::string message) const override final {};

    ~NullLogger() override final {};

  };
  */
}

#endif

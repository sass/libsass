/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_LOGGER_HPP
#define SASS_LOGGER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <bitset>
#include <sstream>
#include "strings.hpp"
#include "constants.hpp"
#include "backtrace.hpp"
#include "callstack.hpp"
#include "terminal.hpp"

namespace Sass {

  void print_wrapped(sass::string const& input, size_t width, sass::ostream& os);

  // The logger belongs to context
  class Logger {

  public:

    enum WarningType {
      WARN_MATH_DIV,
      WARN_CAPI_FN,
      WARN_ANGLE_CONVERT,
      WARN_NUMBER_ARG,
      WARN_NUMBER_PERCENT,
      WARN_GLOBAL_ASSIGN,
      WARN_COLOR_ITPL,
      WARN_EMPTY_SELECTOR,
      WARN_DOUBLE_PARENT,
      WARN_STRING_CALL,
      WARN_MOZ_DOC,
      WARN_RULE
    };

    // Epsilon for precision
    double epsilon;

    // warning buffers
    sass::ostream logstrm;

    // The current callstack
    BackTraces callStack;

    // Available columns on tty
    size_t columns;

    // Flags for unicode and color
    bool support_colors = false;
    bool support_unicode = false;

    // Helper function to ease color output. Returns the
    // passed color if color output is enable, otherwise
    // it will simply return an empty string.
    inline const char* getTerm(const char* color) {
      if (support_colors) {
        return color;
      }
      return Constants::String::empty;
    }

    // Remember which warnings have already reported.
    // Only report them once and just write an additional
    // line reporting how many additional warnings are waiting
    std::bitset<32> reported; // sizeof(Logger::WarningType)

    // How many warnings have been suppressed
    int suppressed = 0;

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

  private:

    // Write warning header to error stream
    void writeWarnHead(
      bool deprecation = false);

  public:

    // Print to stderr stream
    void printWarning(
      const sass::string& message,
      const SourceSpan& pstate,
      enum WarningType type,
      bool deprecation = false);

  private:

    // Format and print source-span
    void printSourceSpan(
      SourceSpan pstate,
      sass::ostream& stream,
      bool unicode = false);

  public:

    // Print `amount` of `traces` to output stream `os`.
    void writeStackTraces(sass::ostream& os,
													StackTraces traces,
													sass::string indent = "  ",
													bool showPos = true,
													size_t showTraces = sass::string::npos);

  public:

    // Default constructor
    Logger(bool colors = false, bool unicode = false,
      int precision = SassDefaultPrecision, size_t columns = NPOS);

    // Auto-detect if colors and unicode is supported
    // Mostly depending if a terminal is connected
    // Additionally fetches available terminal columns
    void autodetectCapabalities();

    // Enable terminal ANSI color support
    void setLogColors(bool enable);
    // Enable terminal unicode support
    void setLogUnicode(bool enable);
    // Set available columns to break debug text
    void setLogColumns(size_t columns = NPOS);

    // Precision for numbers to be printed
    void setPrecision(int precision);

    // Print a warning without any SourceSpan (used by @warn)
    void addWarning(const sass::string& message, enum WarningType);

    // Print a debug message without any SourceSpan (used by @debug)
    void addDebug(const sass::string& message, const SourceSpan& pstate);

    // Print a warning with SourceSpan attached (used internally)
    void addWarning(const sass::string& message, const SourceSpan& pstate, enum WarningType type)
    {
      printWarning(message, pstate, type, false);
    }

    // Print a deprecation warning with SourceSpan attached (used internally)
    void addDeprecation(const sass::string& message, const SourceSpan& pstate, enum WarningType type)
    {
      printWarning(message, pstate, type, true);
    }

  public:

    // Implicitly allow conversion
    operator BackTraces&() {
      return callStack; }

  };

}

#endif

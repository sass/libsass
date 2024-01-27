/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "logger.hpp"

#include <iomanip>
#include "file.hpp"
#include "source.hpp"
#include "utf8/checked.h"
#include "string_utils.hpp"

namespace Sass {

  // Default constructor
  Logger::Logger(bool colors, bool unicode, int precision, size_t columns) :
    epsilon(std::pow(0.1, precision + 1)),
    columns(columns),
    support_colors(colors),
    support_unicode(unicode)
  {}

  // Auto-detect if colors and unicode is supported
  // Mostly depending if a terminal is connected
  // Additionally fetches available terminal columns
  void Logger::autodetectCapabalities()
  {
    setLogColors(Terminal::hasColorSupport(true));
    setLogUnicode(Terminal::hasUnicodeSupport(true));
    setLogColumns(Terminal::getColumns(true));
  }
  // EO autodetectCapabalities

  // Enable terminal ANSI color support
  void Logger::setLogColors(bool enable)
  {
    support_colors = enable;
  }
  // EO setLogColors

  // Enable terminal unicode support
  void Logger::setLogUnicode(bool enable)
  {
    support_unicode = enable;
  }
  // EO setLogUnicode

  // Set available columns to break debug text
  void Logger::setLogColumns(size_t columns)
  {
    // Clamp into a sensible range
    if (columns > 800) { columns = 800; }
    else if (columns < 40) { columns = 40; }
    this->columns = columns;
  }
  // EO setLogColumns

  // Precision for numbers to be printed
  void Logger::setPrecision(int precision)
  {
    epsilon = std::pow(0.1, precision + 1);
  }
  // EO setPrecision

  // Write warning header to error stream
  void Logger::writeWarnHead(bool deprecation)
  {
    if (support_colors) {
      logstrm << getTerm(Terminal::yellow);
      if (!deprecation) logstrm << "Warning";
      else logstrm << "Deprecation Warning";
      logstrm << getTerm(Terminal::reset);
    }
    else {
      if (!deprecation) logstrm << "WARNING";
      else logstrm << "DEPRECATION WARNING";
    }
  }
  // EO writeWarnHead

  // Print the `input` string onto the output stream `os` and
  // wrap words around to fit into the given column `width`.
  void print_wrapped(sass::string const& input, size_t width, sass::ostream& os)
  {
    sass::istream in(input);

    size_t current = 0;
    sass::string word;

    while (in >> word) {
      if (current + word.size() > width) {
        os << STRMLF;
        current = 0;
      }
      os << word << ' ';
      current += word.size() + 1;
      while (Character::isNewline(in.peek())) {
        if (in.peek() == '\n') {
          os << STRMLF;
          current = 0;
        }
        in.ignore(1);
      }
      // Check if new line starts with white-space
      while (Character::isSpaceOrTab(in.peek())) {
        in.ignore(1);
        // Preserve if we have multiple white-space
        if (Character::isSpaceOrTab(in.peek())) os << ' ';
        while (Character::isSpaceOrTab(in.peek())) {
          in.ignore(1);
          os << ' ';
        }
      }
    }
    if (current != 0) {
      os << STRMLF;
    }
  }
  // EO wrap

  // Print a warning without any SourceSpan (used by @warn)
  void Logger::addWarning(const sass::string& message, enum WarningType type)
  {
    writeWarnHead(false);
    logstrm << ": ";

    print_wrapped(message, 80, logstrm);
    StackTraces stack(callStack.begin(), callStack.end());
    writeStackTraces(logstrm, stack, "    ", true, 0);
  }
  // EO addWarning

  // Print a debug message without any SourceSpan (used by @debug)
  void Logger::addDebug(const sass::string& message, const SourceSpan& pstate)
  {
    logstrm << pstate.getDebugPath() << ":" <<
      pstate.getLine() << " DEBUG: " << message;
    logstrm << STRMLF;
  }
  // EO addDebug

  // Print a regular warning or deprecation
  void Logger::printWarning(const sass::string& message, const SourceSpan& pstate, enum WarningType type, bool deprecation)
  {

    callStackFrame frame(*this, pstate);

    if (reported[type]) {
      if (type != WARN_RULE) {
        suppressed += 1;
        return;
      }
    }

    writeWarnHead(deprecation);
    logstrm << " on line " << pstate.getLine();
    logstrm << ", column " << pstate.getColumn();
    logstrm << " of " << pstate.getDebugPath() << ':' << STRMLF;

    // Capped at 80 to keep specs backward compatible
    print_wrapped(message, 80, logstrm);

    logstrm << STRMLF;

    StackTraces stack(callStack.begin(), callStack.end());
    writeStackTraces(logstrm, stack, "    ", true);

    reported[type] = true;
  }
  // EO printWarning

  // Format and print source-span
  void Logger::printSourceSpan(
    SourceSpan pstate,
    sass::ostream& stream,
    bool unicode)
  {

    // ASCII reporting
    sass::string top(",");
    sass::string upper("/");
    sass::string middle("|");
    sass::string lower("\\");
    sass::string runin("-");
    sass::string bottom("'");

    // Or use Unicode versions
    if (unicode) {
      top = "\xE2\x95\xB7";
      upper = "\xE2\x94\x8C";
      middle = "\xE2\x94\x82";
      lower = "\xE2\x94\x94";
      runin = "\xE2\x94\x80";
      bottom = "\xE2\x95\xB5";
    }

    // now create the code trace (ToDo: maybe have utility functions?)
    if (pstate.getContent() == nullptr) return;

    // Calculate offset positions
    Offset beg = pstate.position;
    Offset end = beg + pstate.span;

    // Dart-sass claims this might fail due to float errors
    size_t padding = (size_t)floor(log10(end.line + 1)) + 1;

    // Do multi-line reporting
    SourceData* source = pstate.getSource();
    if (pstate.span.line > 0 && source != nullptr) {

      sass::vector<sass::string> lines;

      // Fetch all lines we need to print the state //XOXOXO
      for (size_t i = 0; i <= pstate.span.line; i++) {
        sass::string line(source->getLine(pstate.position.line + i));
        lines.emplace_back(line);
      }

      // Write intro line
      stream
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << ' '
        << ' ' << top
        << getTerm(Terminal::reset)
        << STRMLF;

      for (size_t i = 0; i < lines.size(); i++) {

        // Right trim the line to report
        StringUtils::makeRightTrimmed(lines[i]);

        // Write the line number and the code
        stream
          << getTerm(Terminal::blue)
          << std::right << std::setfill(' ')
          << std::setw(padding) << (beg.line + i + 1)
          << ' ' << middle
          << getTerm(Terminal::reset)
          << ' ';

        // Report the first line
        // Gets a line underneath
        if (i == 0) {

          // Rewind position to last relevant to report cleaner if possible
          while (beg.column > 0 && Character::isWhitespace(lines[i][beg.column - 1])) beg.column--;

          // Only need a line if not at start
          if (beg.column > 0) {

            // Print the initial code line
            auto line_beg = lines[i].begin();
            utf8::advance(line_beg, beg.column, lines[i].end());
            lines[i].insert(line_beg - lines[i].begin(), getTerm(Terminal::red));
            stream << "  " << lines[i] << getTerm(Terminal::reset) << STRMLF;

            // Print the line beneath
            stream
              << getTerm(Terminal::blue)
              << std::right << std::setfill(' ')
              << std::setw(padding) << ' '
              << ' ' << middle << ' '
              << getTerm(Terminal::reset)
              << getTerm(Terminal::red)
              << upper;
            // This needs a loop unfortunately
            size_t cols = beg.column;
            for (size_t n = 0; n < cols + 2; n++) {
              stream << runin;
            }
            // Final indicator
            stream
              << '^'
              << getTerm(Terminal::reset)
              << STRMLF;
          }
          // Just print the code line
          else {
            stream
              << getTerm(Terminal::red)
              << upper << ' ' << lines[i]
              << getTerm(Terminal::reset)
              << STRMLF;
          }
        }
        // Last line might get another indicator line
        else if (i == lines.size() - 1) {

          if (end.column < lines[i].size()) {

            // Print the final code line
            auto line_beg = lines[i].begin();
            utf8::advance(line_beg, end.column, lines[i].end());
            lines[i].insert(line_beg - lines[i].begin(), getTerm(Terminal::reset));
            stream << getTerm(Terminal::red) << middle << ' ' << lines[i] << STRMLF;

            // Print the line beneath
            stream
              << getTerm(Terminal::blue)
              << std::right << std::setfill(' ')
              << std::setw(padding) << ' '
              << ' ' << middle << ' '
              << getTerm(Terminal::reset)
              << getTerm(Terminal::red)
              << lower;
            // This needs a loop unfortunately
            for (size_t n = 0; n < end.column; n++) {
              stream << runin;
            }
            // Final indicator
            stream
              << '^'
              << getTerm(Terminal::reset)
              << STRMLF;
          }
          else {
            // Just print the code line
            stream
              << getTerm(Terminal::red)
              << lower << ' ' << lines[i]
              << getTerm(Terminal::reset)
              << STRMLF;
          }
        }
        else {
          // Just print the code line
          stream
            << getTerm(Terminal::red)
            << middle << ' ' << lines[i]
            << getTerm(Terminal::reset)
            << STRMLF;
        }
      }

      // Write outro line
      stream
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << ' '
        << ' ' << bottom
        << getTerm(Terminal::reset)
        << STRMLF;

    }
    // Single line reporting
    else {

      sass::string raw = pstate.getSource()->getLine(pstate.position.line);
      sass::string line; utf8::replace_invalid(raw.begin(), raw.end(),
        std::back_inserter(line), unicode ? 0xfffd : '?');

      // Convert to ASCII only string
      if (!unicode) {
        std::replace_if(line.begin(), line.end(),
          Character::isUtf8StartByte, '?');
        line.erase(std::remove_if(line.begin(), line.end(),
          Character::isUtf8Continuation), line.end());
      }

      size_t lhs_len, mid_len; //, rhs_len;
      // Get the sizes (characters) for each part
      mid_len = pstate.span.column;
      lhs_len = pstate.position.column;

      // Normalize tab characters to spaces for better counting
      for (size_t i = line.length(); i != std::string::npos; i -= 1) {
        if (line[i] == '\t') {
          // Adjust highlight positions
          if (i < lhs_len) lhs_len += 3;
          else if (i < lhs_len + mid_len) mid_len += 3;
          // Replace tab with spaces
          line.replace(i, 1, "    ");
        }
      }

      // Split line into parts to report
      // They will be shortened if needed
      sass::string lhs, mid, rhs;
      splitLine(line, lhs_len, mid_len,
        columns - 4 - padding, lhs, mid, rhs);

      // Get character length of each part
      lhs_len = utf8::distance(lhs.begin(), lhs.end());
      mid_len = utf8::distance(mid.begin(), mid.end());
      // rhs_len = utf8::distance(rhs.begin(), rhs.end());

      // Report the trace
      stream

        // Write the leading line
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << ' '
        << ' ' << top
        << getTerm(Terminal::reset)

        << STRMLF

        // Write the line number
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << (beg.line + 1)
        << ' ' << middle
        << getTerm(Terminal::reset)

        << ' '

        // Write the parts
        << lhs
        << getTerm(Terminal::red)
        << mid
        << getTerm(Terminal::reset)
        << rhs

        << STRMLF

        // Write left part of marker line
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << ' '
        << ' ' << middle
        << getTerm(Terminal::reset)

        << ' '

        // Write the actual marker
        << sass::string(lhs_len, ' ')
        << getTerm(Terminal::red)
        << sass::string(mid_len ? mid_len : 1, '^')
        << getTerm(Terminal::reset)

        << STRMLF

        // Write the trailing line
        << getTerm(Terminal::blue)
        << std::right << std::setfill(' ')
        << std::setw(padding) << ' '
        << ' ' << bottom
        << getTerm(Terminal::reset)

        << STRMLF;

    }

  }
  // EO printSourceSpan

  // Helper function for `printSourceSpan` to split lines to be printed
  void Logger::splitLine(sass::string line, size_t lhs_len, size_t mid_len,
    size_t columns, sass::string& lhs, sass::string& mid, sass::string& rhs)
  {

    // Get the ellipsis character(s) either in unicode or ASCII
    size_t ellipsis_len = support_unicode ? 1 : 3;
    sass::string ellipsis(support_unicode ? "\xE2\x80\xA6" : "...");

    // Normalize tab characters to spaces for better counting
    for (size_t i = line.length(); i != std::string::npos; i -= 1) {
      if (line[i] == '\t') line.replace(i, 1, "    ");
    }

    // Get line iterators
    auto line_beg = line.begin();
    auto line_end = line.end();

    // Get the sizes (characters) for each part
    size_t line_len = utf8::distance(line_beg, line_end);
    size_t rhs_len = line_len - lhs_len - mid_len;

    // Prepare iterators for left side of highlighted string
    auto lhs_beg = line.begin(), lhs_end = lhs_beg;
    utf8::advance(lhs_end, lhs_len, line_end);
    // Prepare iterators for right side of highlighted string
    auto rhs_beg = lhs_end, rhs_end = line.end();
    utf8::advance(rhs_beg, mid_len, line_end);

    // Create substring of each part
    lhs.append(lhs_beg, lhs_end);
    rhs.append(rhs_beg, rhs_end);
    mid.append(lhs_end, rhs_beg);

    // Trim trailing spaces
    StringUtils::makeRightTrimmed(rhs);

    // Re-count characters after trimming is done
    lhs_len = utf8::distance(lhs.begin(), lhs.end());
    rhs_len = utf8::distance(rhs.begin(), rhs.end());

    // Get how much we want to show at least
    size_t min_left = 12, min_right = 12;
    // But we can't show more than we have
    min_left = std::min(min_left, lhs_len);
    min_right = std::min(min_right, rhs_len);

    // Calculate available size for highlighted part
    size_t mid_max = columns - min_left - min_right;

    // Check if middle parts needs shortening
    if (mid_len > mid_max) {
      // Calculate how much we must shorten
      size_t shorten = mid_len - mid_max + ellipsis_len;
      // Get the sizes for left and right parts and adjust for epsilon
      size_t lhs_size = size_t(std::ceil(0.5 * (mid_len - shorten)));
      size_t rhs_size = size_t(std::floor(0.5 * (mid_len - shorten)));
      // Prepare iterators for later substring operation
      auto lhs_start = mid.begin(), rhs_stop = mid.end();
      auto lhs_stop = lhs_start; utf8::advance(lhs_stop, lhs_size, rhs_stop);
      auto rhs_start = lhs_stop; utf8::advance(rhs_start, shorten, rhs_stop);
      // Recreate shortened middle (highlight) part
      mid = sass::string(lhs_start, lhs_stop) +
        ellipsis + sass::string(rhs_start, rhs_stop);
      // Update middle size (should be mid_max)
      mid_len = lhs_size + rhs_size + ellipsis_len;
    }
    else {
      // We can give some space back
      size_t leftover = mid_max - mid_len;
      // Distribute the leftovers
      // ToDo: do it with math only
      while (leftover) {
        if (min_left < lhs_len) {
          min_left += 1;
          leftover -= 1;
          if (min_right < rhs_len) {
            if (leftover > 0) {
              min_right += 1;
              leftover -= 1;
            }
          }
        }
        else if (min_right < rhs_len) {
          min_right += 1;
          leftover -= 1;
        }
        else {
          break;
        }
      }
    }

    // Shorten left side
    if (min_left < lhs_len) {
      min_left = min_left - ellipsis_len;
      auto beg = lhs.begin(), end = lhs.end();
      utf8::advance(beg, lhs_len - min_left, end);
      lhs = sass::string(beg, end);
      StringUtils::makeLeftTrimmed(lhs);
      lhs = ellipsis + lhs;
    }

    // Shorten right side
    if (min_right < rhs_len) {
      min_right = min_right - ellipsis_len;
      auto beg = rhs.begin(), end = rhs.begin();
      utf8::advance(end, min_right, rhs.end());
      rhs = sass::string(beg, end);
      StringUtils::makeRightTrimmed(rhs);
      rhs = rhs + ellipsis;
    }

  }
  // EO splitLine

  // Print `amount` of `traces` to output stream `os`.
  void Logger::writeStackTraces(sass::ostream& os, StackTraces traces,
    sass::string indent, bool showPos, size_t amount)
  {
    sass::sstream strm;

    // bool first = true;
    size_t max = 0;
    size_t i_beg = traces.size() - 1;
    size_t i_end = sass::string::npos;


    sass::string last("root stylesheet");
    sass::vector<std::pair<sass::string, sass::string>> traced;
    for (size_t i = 0; i < traces.size(); i++) {

      const StackTrace& trace = traces[i];

      // make path relative to the current directory
      sass::string rel_path(File::abs2rel(trace.pstate.getAbsPath(), CWD(), CWD()));

      strm.str(sass::string());
      strm << rel_path << ' ';
      strm << trace.pstate.getLine();
      strm << ":" << trace.pstate.getColumn();

      sass::string str(strm.str());
      max = std::max(max, str.length());

      if (i == 0) {
        traced.emplace_back(std::make_pair(
          str, last));
      }
      else if (!traces[i - 1].name.empty()) {
        last = traces[i - 1].name;
        if (traces[i - 1].fn) last += "()";
        traced.emplace_back(std::make_pair(str, last));
      }
      else {
        traced.emplace_back(std::make_pair(
          str, last));
      }

    }

    i_beg = traces.size() - 1;
    i_end = sass::string::npos;
    const StackTrace* prev = nullptr;
    for (size_t i = i_beg; i != i_end; i--) {

      const StackTrace& trace = traces[i];

      // make path relative to the current directory
      sass::string rel_path(File::abs2rel(trace.pstate.getAbsPath(), CWD(), CWD()));

      // skip functions on error cases (unsure why ruby sass does this)
      // if (trace.caller.substr(0, 6) == ", in f") continue;

      if (amount == sass::string::npos || amount > 0) {
        if (prev && *prev == trace) continue;
        printSourceSpan(trace.pstate, os, support_unicode);
        if (amount > 0) --amount;
        prev = &trace;
      }

      if (showPos) {
        os << indent;
        os << std::left << std::setfill(' ')
          << std::setw(max + 2) << traced[i].first;
        os << traced[i].second;
        os << STRMLF;
      }

    }

    os << STRMLF;

  }
  // EO writeStackTraces

}

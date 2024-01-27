/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "terminal.hpp"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#endif

// Minimal terminal abstraction for cross compatibility.
// Its main purpose is to let us print stuff with colors.
namespace Terminal {

  // Query number of available console columns
  // Useful to shorten our output to fit nicely
  short getColumns(bool error)
  {
    // First check if console is attached
    if (!isConsoleAttached(error)) {
      return SassDefaultColumns;
    }
    #ifdef _WIN32
    DWORD fd = error
      ? STD_ERROR_HANDLE
      : STD_OUTPUT_HANDLE;
    // Get information of the screen buffer
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE handle = GetStdHandle(fd);
    GetConsoleScreenBufferInfo(handle, &csbi);
    // csbi.srWindow.Right - csbi.srWindow.Left
    return csbi.dwMaximumWindowSize.X;
    #else
    int fd = open("/dev/tty", O_RDWR);
    struct winsize ws;
    if (fd < 0 || ioctl(fd, TIOCGWINSZ, &ws) < 0) {
      return SassDefaultColumns;
    }
    close(fd);
    return ws.ws_col;
    #endif
  }
  // EO getColumns

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool isConsoleAttached(bool error)
  {
    #ifdef _WIN32
    DWORD fd = error
      ? STD_ERROR_HANDLE
      : STD_OUTPUT_HANDLE;
    if (GetConsoleCP() == 0) return false;
    HANDLE handle = GetStdHandle(fd);
    if (handle == INVALID_HANDLE_VALUE) return false;
    DWORD filetype = GetFileType(handle);
    return filetype == FILE_TYPE_CHAR;
    #else
    return isatty(fileno(error ? stderr : stdout));
    #endif
  }
  // EO isConsoleAttached

  // Check that we print to a terminal with unicode support
  bool hasUnicodeSupport(bool error)
  {
    #ifdef _WIN32
    UINT cp = GetConsoleOutputCP();
    if (cp == 0) return false;
    return cp == 65001;
    #else
    return false;
    #endif
  }
  // EO hasUnicodeSupport

  // Check that we print to a terminal with color support
  bool hasColorSupport(bool error)
  {
    #ifdef _WIN32
    return isConsoleAttached(error);
    #else
    return isConsoleAttached(error);
    #endif
  }
  // EO hasColorSupport

  // This function is able to print a line with colors
  // It translates the ANSI terminal codes to windows
  void print(const char* output, bool error)
  {

    if (output == 0) return;

    #ifdef _WIN32

    DWORD fd = error
      ? STD_ERROR_HANDLE
      : STD_OUTPUT_HANDLE;

    HANDLE handle = GetStdHandle(fd);
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    GetConsoleScreenBufferInfo(handle, &info);
    WORD attribute = info.wAttributes;

    while (output[0] != '\0') {

      if (output[0] == '\x1B' && output[1] == '[') {

        output += 2;
        int one = 0;
        int two = 0;

        attribute &= ~(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        attribute &= ~(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);

        while (output[0] != '\0' && output[0] != ';' && output[0] != 'm') {
          one *= 10;
          switch (*output) {
          case '0': one += 0; break;
          case '1': one += 1; break;
          case '2': one += 2; break;
          case '3': one += 3; break;
          case '4': one += 4; break;
          case '5': one += 5; break;
          case '6': one += 6; break;
          case '7': one += 7; break;
          case '8': one += 8; break;
          case '9': one += 9; break;
          default: break;
          }
          output += 1;
        }

        if (one == 31 || one == 33 || one == 35 || one == 37 || one == 0) attribute |= FOREGROUND_RED;
        if (one == 32 || one == 33 || one == 36 || one == 37 || one == 0) attribute |= FOREGROUND_GREEN;
        if (one == 34 || one == 35 || one == 36 || one == 37 || one == 0) attribute |= FOREGROUND_BLUE;
        if (one == 41 || one == 43 || one == 45 || one == 47) attribute |= BACKGROUND_RED;
        if (one == 42 || one == 43 || one == 46 || one == 47) attribute |= BACKGROUND_GREEN;
        if (one == 44 || one == 45 || one == 46 || one == 47) attribute |= BACKGROUND_BLUE;

        if (output[0] == ';') {

          output += 1;

          while (output[0] != '\0' && output[0] != ';' && output[0] != 'm') {
            two *= 10;
            switch (*output) {
            case '0': two += 0; break;
            case '1': two += 1; break;
            case '2': two += 2; break;
            case '3': two += 3; break;
            case '4': two += 4; break;
            case '5': two += 5; break;
            case '6': two += 6; break;
            case '7': two += 7; break;
            case '8': two += 8; break;
            case '9': two += 9; break;
            default: break;
            }
            output += 1;
          }

          if (two == 31 || two == 33 || two == 35 || two == 37 || two == 0) attribute |= FOREGROUND_RED;
          if (two == 32 || two == 33 || two == 36 || two == 37 || two == 0) attribute |= FOREGROUND_GREEN;
          if (two == 34 || two == 35 || two == 36 || two == 37 || two == 0) attribute |= FOREGROUND_BLUE;
          if (two == 41 || two == 43 || two == 45 || two == 47) attribute |= BACKGROUND_RED;
          if (two == 42 || two == 43 || two == 46 || two == 47) attribute |= BACKGROUND_GREEN;
          if (two == 44 || two == 45 || two == 46 || two == 47) attribute |= BACKGROUND_BLUE;

        }

        if (one == 1 && two > 30 && two < 50) attribute |= FOREGROUND_INTENSITY;
        if (one == 1 && two > 40 && two < 50) attribute |= BACKGROUND_INTENSITY;

        // attribute = BACKGROUND_GREEN | BACKGROUND_INTENSITY;

        if (output[0] == 'm') output += 1;
        // Flush before setting new color
        // ToDo: there might be a better way
        fflush(error ? stderr : stdout);
        SetConsoleTextAttribute(handle, attribute);

      }
      // else if (output[0] == '\r' && output[1] == '\n') {
      //   output += 1;
      // }
      else {

        if (error) {
          std::cerr << output[0];
          if (output[0] == '\n') fflush(stderr);
        }
        else {
          std::cout << output[0];
          if (output[0] == '\n') fflush(stdout);
        }
        output += 1;
      }

    }

    fflush(error ? stderr : stdout);

    #else

    if (error) {
      std::cerr << output;
    }
    else {
      std::cout << output;
    }

    #endif

  }
  // EO print

  // Count number of printable bytes/characters
  size_t count_printable(const char* string)
  {
    size_t count = 0;
    while (string && *string) {
      if (string[0] == '\x1b' && string[1] == '[') {
        while (*string != 0 && *string != 'm') {
          string++;
        }
        string++;
      }
      else {
        string += 1;
        count += 1;
      }
    }
    return count;
  }
  // EO count_printable

  // Code for color testing rainbow for debugging
  // 
  // stream << getopt->compiler.getTerm(Terminal::red) << "red" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::green) << "green" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::yellow) << "yellow" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::blue) << "blue" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::magenta) << "magenta" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::cyan) << "cyan" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_red) << "bold_red" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_green) << "bold_green" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_yellow) << "bold_yellow" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_blue) << "bold_blue" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_magenta) << "bold_magenta" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bold_cyan) << "bold_cyan" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_red) << "bg_red" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_green) << "bg_green" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_yellow) << "bg_yellow" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_blue) << "bg_blue" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_magenta) << "bg_magenta" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_cyan) << "bg_cyan" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_red) << "bg_bold_red" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_green) << "bg_bold_green" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_yellow) << "bg_bold_yellow" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_blue) << "bg_bold_blue" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_magenta) << "bg_bold_magenta" << getopt->compiler.getTerm(Terminal::reset) << "\n";
  // stream << getopt->compiler.getTerm(Terminal::bg_bold_cyan) << "bg_bold_cyan" << getopt->compiler.getTerm(Terminal::reset) << "\n";


}

#include "terminal.hpp"

// Minimal terminal abstraction for cross compatibility.
// Its main purpose is to let us print stuff with colors.
namespace Terminal {

#ifdef _WIN32

  // Query number of available console columns
  // Useful to shorten our output to fit nicely
  short getColumns(bool error)
  {
    DWORD fd = error
      ? STD_ERROR_HANDLE
      : STD_OUTPUT_HANDLE;
    // First check if console is attached
    if (!isConsoleAttached(error)) {
      return SassDefaultColumns;
    }
    // Get information of the screen buffer
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE handle = GetStdHandle(fd);
    GetConsoleScreenBufferInfo(handle, &csbi);
    // csbi.srWindow.Right - csbi.srWindow.Left
    return csbi.dwMaximumWindowSize.X;
  }

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool isConsoleAttached(bool error)
  {
    DWORD fd = error
      ? STD_ERROR_HANDLE
      : STD_OUTPUT_HANDLE;
    if (GetConsoleCP() == 0) return false;
    HANDLE handle = GetStdHandle(fd);
    if (handle == INVALID_HANDLE_VALUE) return false;
    DWORD filetype = GetFileType(handle);
    return filetype == FILE_TYPE_CHAR;
  }

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool hasUnicodeSupport(bool error)
  {
    return false;
    UINT cp = GetConsoleOutputCP();
    if (cp == 0) return false;
    return cp == 65001;
  }

  bool hasColorSupport(bool error)
  {
    return false;
    return isConsoleAttached(true);
  }

  // This function is able to print a line with colors
  // It translates the Unix terminal codes to windows
  void print(const char* output, bool error)
  {

    if (output == 0) return;

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
        int fg = 0;
        int bg = 0;

        while (output[0] != '\0' && output[0] != ';' && output[0] != 'm') {
          fg *= 10;
          switch (*output) {
          case '0': fg += 0; break;
          case '1': fg += 1; break;
          case '2': fg += 2; break;
          case '3': fg += 3; break;
          case '4': fg += 4; break;
          case '5': fg += 5; break;
          case '6': fg += 6; break;
          case '7': fg += 7; break;
          case '8': fg += 8; break;
          case '9': fg += 9; break;
          default: break;
          }
          output += 1;
        }

        attribute &= ~(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        if (fg == 31 || fg == 33 || fg == 35 || fg == 37 || fg == 0) attribute |= FOREGROUND_RED;
        if (fg == 32 || fg == 33 || fg == 36 || fg == 37 || fg == 0) attribute |= FOREGROUND_GREEN;
        if (fg == 34 || fg == 35 || fg == 36 || fg == 37 || fg == 0) attribute |= FOREGROUND_BLUE;
        if (fg != 37 && fg != 0) attribute |= FOREGROUND_INTENSITY;
        if (output[0] == ';') {

          output += 1;

          while (output[0] != '\0' && output[0] != ';' && output[0] != 'm') {
            bg *= 10;
            switch (*output) {
            case '0': bg += 0; break;
            case '1': bg += 1; break;
            case '2': bg += 2; break;
            case '3': bg += 3; break;
            case '4': bg += 4; break;
            case '5': bg += 5; break;
            case '6': bg += 6; break;
            case '7': bg += 7; break;
            case '8': bg += 8; break;
            case '9': bg += 9; break;
            default: break;
            }
            output += 1;
          }

          attribute &= ~(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
          if (bg == 31 || bg == 33 || bg == 35 || bg == 37) attribute |= BACKGROUND_RED;
          if (bg == 32 || bg == 33 || bg == 36 || bg == 37) attribute |= BACKGROUND_GREEN;
          if (bg == 34 || bg == 35 || bg == 36 || bg == 37) attribute |= BACKGROUND_BLUE;
          if (fg != 37 && fg != 0) attribute |= BACKGROUND_INTENSITY;

        }

        if (output[0] == 'm') output += 1;
        // Flush before setting new color
        // ToDo: there might be a better way
        fflush(error ? stderr : stdout);
        SetConsoleTextAttribute(handle, attribute);

      }
      else {

        if (error) {
          std::cerr << output[0];
        }
        else {
          std::cout << output[0];
        }
        output += 1;
      }

    }

    fflush(error ? stderr : stdout);

  }

#else

  short getColumns(bool error)
  {
    return 80;
  }

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool isConsoleAttached(bool error) { return true; }
  bool hasUnicodeSupport(bool error) { return true; }
  bool hasColorSupport(bool error) { return true; }

  void print(const char* output, bool error)
  {
    if (error) {
      std::cerr << output;
    }
    else {
      std::cout << output;
    }
  }

#endif

}

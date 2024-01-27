/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ERROR_H
#define SASS_ERROR_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Error related getters (use after compiler was rendered)
  ADDAPI int ADDCALL sass_error_get_status(const struct SassError* error);

  // Getter for plain error message (use after compiler was rendered).
  ADDAPI const char* ADDCALL sass_error_get_string(const struct SassError* error);

  // Getter for error status as css (In order to show error in browser).
  // Memory returned by this function must be freed via `sass_free_c_string`.
  ADDAPI char* ADDCALL sass_error_get_css(const struct SassError* error);

  // Getter for error status as json object (Useful to pass to downstream).
  // Memory returned by this function must be freed via `sass_free_c_string`.
  ADDAPI char* ADDCALL sass_error_get_json(const struct SassError* error);

  // Getter for formatted error message. According to logger style this
  // may be in unicode and may contain ANSI escape codes for colors.
  ADDAPI const char* ADDCALL sass_error_get_formatted(const struct SassError* error);

  // Getter for line position where error occurred (starts from 1).
  ADDAPI size_t ADDCALL sass_error_get_line(const struct SassError* error);

  // Getter for column position where error occurred (starts from 1).
  ADDAPI size_t ADDCALL sass_error_get_column(const struct SassError* error);

  // Getter for source content referenced in line and column.
  ADDAPI const char* ADDCALL sass_error_get_content(const struct SassError* error);

  // Getter for path where the error occurred.
  ADDAPI const char* ADDCALL sass_error_get_path(const struct SassError* error);

  // Getter for number of traces attached to error object.
  ADDAPI size_t ADDCALL sass_error_count_traces(const struct SassError* error);

  // Getter for last trace (or nullptr if none are available).
  ADDAPI const struct SassTrace* ADDCALL sass_error_last_trace(const struct SassError* error);

  // Getter for nth trace (or nullptr if `n` is invalid).
  ADDAPI const struct SassTrace* ADDCALL sass_error_get_trace(const struct SassError* error, size_t n);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif

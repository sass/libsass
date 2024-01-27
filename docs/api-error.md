## LibSass C-API for errors

API to get additional information for errors occurring during custom functions.
Error object are not reference counted and are coupled to the compiler life-cycle.
You can also use them to inspect errors after the compiler failed at any phase.

### Basic Usage

```C
#include <sass/error.h>
```

## Sass Function API

```C
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Error related getters (use after compiler was rendered)
int sass_error_get_status(const struct SassError* error);

// Getter for plain error message (use after compiler was rendered).
const char* sass_error_get_string(const struct SassError* error);

// Getter for error status as css (In order to show error in browser).
// Memory returned by this function must be freed via `sass_free_c_string`.
char* sass_error_get_css(const struct SassError* error);

// Getter for error status as json object (Useful to pass to downstream).
// Memory returned by this function must be freed via `sass_free_c_string`.
char* sass_error_get_json(const struct SassError* error);

// Getter for formatted error message. According to logger style this
// may be in unicode and may contain ANSI escape codes for colors.
const char* sass_error_get_formatted(const struct SassError* error);

// Getter for line position where error occurred (starts from 1).
size_t sass_error_get_line(const struct SassError* error);

// Getter for column position where error occurred (starts from 1).
size_t sass_error_get_column(const struct SassError* error);

// Getter for source content referenced in line and column.
const char* sass_error_get_content(const struct SassError* error);

// Getter for path where the error occurred.
const char* sass_error_get_path(const struct SassError* error);

// Getter for number of traces attached to error object.
size_t sass_error_count_traces(const struct SassError* error);

// Getter for last trace (or nullptr if none are available).
const struct SassTrace* sass_error_last_trace(const struct SassError* error);

// Getter for nth trace (or nullptr if `n` is invalid).
const struct SassTrace* sass_error_get_trace(const struct SassError* error, size_t n);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
```

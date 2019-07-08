#ifndef SASS_C_ERROR_H
#define SASS_C_ERROR_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Error related getters (use after compiler was rendered)
  ADDAPI int ADDCALL sass_error_get_status(const struct SassError* error);
  ADDAPI char* ADDCALL sass_error_get_json(const struct SassError* error);
  ADDAPI const char* ADDCALL sass_error_get_what(const struct SassError* error);
  // ADDAPI const char* ADDCALL sass_error_get_messages(struct SassError* error);
  // ADDAPI const char* ADDCALL sass_error_get_warnings(struct SassError* error);
  ADDAPI const char* ADDCALL sass_error_get_formatted(const struct SassError* error);

  // These are here for convenience, could get them also indirectly
  ADDAPI size_t ADDCALL sass_error_get_line(const struct SassError* error);
  ADDAPI size_t ADDCALL sass_error_get_column(const struct SassError* error);
  ADDAPI const char* ADDCALL sass_error_get_path(const struct SassError* error);
  ADDAPI const char* ADDCALL sass_error_get_content(const struct SassError* error);

  // ADDAPI size_t ADDCALL sass_traces_get_size(struct SassTraces* traces);
  // ADDAPI struct SassTrace* ADDCALL sass_traces_get_last(struct SassTraces* traces);
  // ADDAPI struct SassTrace* ADDCALL sass_traces_get_trace(struct SassTraces* traces, size_t i);

  ADDAPI size_t ADDCALL sass_error_count_traces(const struct SassError* error);
  ADDAPI const struct SassTrace* ADDCALL sass_error_last_trace(const struct SassError* error);
  ADDAPI const struct SassTrace* ADDCALL sass_error_get_trace(const struct SassError* error, size_t i);

  ADDAPI size_t ADDCALL sass_compiler_count_traces(struct SassCompiler* compiler);
  ADDAPI const struct SassTrace* ADDCALL sass_compiler_last_trace(struct SassCompiler* compiler);
  ADDAPI const struct SassTrace* ADDCALL sass_compiler_get_trace(struct SassCompiler* compiler, size_t i);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif

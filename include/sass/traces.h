/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_TRACES_H
#define SASS_TRACES_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Traces must be got directly from the underlying object. We expose
  // traces during eval (BackTraces) and when handling error (StackTraces).

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassTrace
  /////////////////////////////////////////////////////////////////////////

  // Getter for name of this trace (normally the function name or empty).
  ADDAPI const char* ADDCALL sass_trace_get_name(struct SassTrace* trace);

  // Getter to check if trace is from a function call (otherwise import).
  ADDAPI bool ADDCALL sass_trace_was_fncall(struct SassTrace* trace);

  // Getter for the SourceSpan (aka ParserState) for further details
  ADDAPI const struct SassSrcSpan* ADDCALL sass_trace_get_srcspan(struct SassTrace* trace);

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassSrcSpan
  /////////////////////////////////////////////////////////////////////////

  // Getter for line position of trace (starting from 0)
  ADDAPI size_t ADDCALL sass_srcspan_get_src_ln(struct SassSrcSpan* pstate);

  // Getter for column position of trace (starting from 0)
  ADDAPI size_t ADDCALL sass_srcspan_get_src_col(struct SassSrcSpan* pstate);

  // Getter for line position of trace (starting from 1)
  ADDAPI size_t ADDCALL sass_srcspan_get_src_line(struct SassSrcSpan* pstate);

  // Getter for column position of trace (starting from 1)
  ADDAPI size_t ADDCALL sass_srcspan_get_src_column(struct SassSrcSpan* pstate);

  // Getter for line span of trace (starting from 0)
  ADDAPI size_t ADDCALL sass_srcspan_get_span_ln(struct SassSrcSpan* pstate);

  // Getter for column span of trace (starting from 0)
  ADDAPI size_t ADDCALL sass_srcspan_get_span_col(struct SassSrcSpan* pstate);

  // Getter for attached source of trace for further details
  ADDAPI struct SassSource* ADDCALL sass_srcspan_get_source(struct SassSrcSpan* pstate);

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassSource
  /////////////////////////////////////////////////////////////////////////

  // Getter for absolute path this source was loaded from. This path should
  // always be absolute  but there is no real hard requirement for it. Custom
  // importers may use different pattern for paths. LibSass tries to support
  // regular win/nix paths and urls. But we it also try to be agnostic here,
  // so anything a custom importer returns will be returned here.
  ADDAPI const char* ADDCALL sass_source_get_abs_path(struct SassSource* source);

  // Getter for import path this source was loaded from. This path should
  // be as it was found when the import was parsed. This is merely useful
  // for debugging purposes, but we keep it around anyway.
  ADDAPI const char* ADDCALL sass_source_get_imp_path(struct SassSource* source);

  // Getter for the loaded content attached to the source.
  ADDAPI const char* ADDCALL sass_source_get_content(struct SassSource* source);

  // Getter for the loaded srcmap attached to the source.
  // Note: not used yet, only here for future improvements.
  ADDAPI const char* ADDCALL sass_source_get_srcmap(struct SassSource* source);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif

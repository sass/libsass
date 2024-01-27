/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_traces.hpp"

#include "source_map.hpp"

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassTrace
  /////////////////////////////////////////////////////////////////////////

  // Getter for name of this trace (normally the function name or empty).
  const char* ADDCALL sass_trace_get_name(struct SassTrace* trace)
  {
    return Traced::unwrap(trace).getName().c_str();
  }

  // Getter to check if trace is from a function call (otherwise import).
  bool ADDCALL sass_trace_was_fncall(struct SassTrace* trace)
  {
    return Traced::unwrap(trace).isFn();
  }

  // Getter for the SourceSpan (aka ParserState) for further details
  const struct SassSrcSpan* ADDCALL sass_trace_get_srcspan(struct SassTrace* trace)
  {
    return SourceSpan::wrap(&Traced::unwrap(trace).getPstate());
  }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassSrcSpan
  /////////////////////////////////////////////////////////////////////////

  // Getter for line position of trace (starting from 0)
  size_t ADDCALL sass_srcspan_get_src_ln(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).position.line;
  }

  // Getter for column position of trace (starting from 0)
  size_t ADDCALL sass_srcspan_get_src_col(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).position.column;
  }

  // Getter for line position of trace (starting from 1)
  size_t ADDCALL sass_srcspan_get_src_line(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).getLine();
  }

  // Getter for column position of trace (starting from 1)
  size_t ADDCALL sass_srcspan_get_src_column(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).getColumn();
  }

  // Getter for line span of trace (starting from 0)
  size_t ADDCALL sass_srcspan_get_span_ln(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).span.line;
  }

  // Getter for column span of trace (starting from 0)
  size_t ADDCALL sass_srcspan_get_span_col(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).span.column;
  }

  // Getter for attached source of trace for further details
  struct SassSource* ADDCALL sass_srcspan_get_source(struct SassSrcSpan* pstate)
  {
    return SourceData::wrap(SourceSpan::unwrap(pstate).getSource());
  }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to struct SassSource
  /////////////////////////////////////////////////////////////////////////

  // Getter for absolute path this source was loaded from. This path should
  // always be absolute  but there is no real hard requirement for it. Custom
  // importers may use different pattern for paths. LibSass tries to support
  // regular win/nix paths and urls. But we it also try to be agnostic here,
  // so anything a custom importer returns will be returned here.
  const char* ADDCALL sass_source_get_abs_path(struct SassSource* source)
  {
    return SourceData::unwrap(source).getAbsPath();
  }

  // Getter for import path this source was loaded from. This path should
  // be as it was found when the import was parsed. This is merely useful
  // for debugging purposes, but we keep it around anyway.
  const char* ADDCALL sass_source_get_imp_path(struct SassSource* source)
  {
    return SourceData::unwrap(source).getImpPath();
  }

  // Getter for the loaded content attached to the source.
  const char* ADDCALL sass_source_get_content(struct SassSource* source)
  {
    return SourceData::unwrap(source).content();
  }

  // Getter for the loaded srcmap attached to the source.
  // Note: not used yet, only here for future improvements.
  const char* ADDCALL sass_source_get_srcmap(struct SassSource* source)
  {
    return SourceData::unwrap(source).srcmaps();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

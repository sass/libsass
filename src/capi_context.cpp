#include "capi_context.hpp"

#include "file.hpp"
#include "source.hpp"
#include "capi_error.hpp"

using namespace Sass;

extern "C" {

  struct SassTraces* ADDCALL sass_error_get_traces(struct SassError* error)
  {
    return reinterpret_cast<struct SassTraces*>(&error->traces);
  }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to imports
  /////////////////////////////////////////////////////////////////////////

  const char* SASS_PATH_DATA = "sass://data";

  struct SassImport* ADDCALL sass_make_data_import(char* content, const char* imp_path)
  {
    // Create the import struct
    return sass_make_import(
      imp_path ? imp_path : SASS_PATH_DATA,
      imp_path ? imp_path : SASS_PATH_DATA,
      content, nullptr,
      SASS_IMPORT_AUTO
    );

    return nullptr;
  }

  // struct SassImport* ADDCALL sass_make_file_import(struct SassCompiler* compiler, const char* imp_path)
  // {
  //   // Check if entry file-path is given
  //   if (imp_path == nullptr) return nullptr;
  // 
  //   // Create absolute path from input filename
  //   sass::string abs_path(File::rel2abs(imp_path, CWD));
  // 
  //   // Try to load the entry file
  //   char* content = File::slurp_file(abs_path, CWD);
  // 
  //   // Check if something was read
  //   if (content == nullptr) {
  //     return nullptr;
  //   }
  // 
  //   // Create the import struct
  //   return sass_make_import(
  //     imp_path,
  //     abs_path.c_str(),
  //     content, nullptr,
  //     SASS_IMPORT_AUTO
  //   );
  // }

  void ADDCALL sass_import_set_format(struct SassImport* import, enum SassImportFormat format)
  {
    Import::unwrap(import).syntax = format;
  }

  // void ADDCALL sass_entry_set_abs_path(struct SassImport* import, const char* path)
  // {
  //   Import::unwrap(import).setAbsPath(path);
  // }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to traces
  /////////////////////////////////////////////////////////////////////////

  // size_t ADDCALL sass_traces_get_size(struct SassTraces* traces)
  // {
  //   return reinterpret_cast<StackTraces*>(traces)->size();
  // }
  // 
  // struct SassTrace* ADDCALL sass_traces_get_trace(struct SassTraces* traces, size_t i)
  // {
  //   return reinterpret_cast<StackTraces*>(traces)->at(i).wrap();
  // }

  const char* ADDCALL sass_trace_get_name(struct SassTrace* trace)
  {
    return Traced::unwrap(trace).getName().c_str();
  }

  bool ADDCALL sass_trace_get_was_fncall(struct SassTrace* trace)
  {
    return Traced::unwrap(trace).isFn();
  }

  const struct SassSrcSpan* ADDCALL sass_trace_get_srcspan(struct SassTrace* trace)
  {
    return SourceSpan::wrap(&Traced::unwrap(trace).getPstate());
  }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to source-spans
  /////////////////////////////////////////////////////////////////////////

  size_t ADDCALL sass_srcspan_get_src_ln(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).position.line;
  }
  size_t ADDCALL sass_srcspan_get_src_col(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).position.column;
  }
  size_t ADDCALL sass_srcspan_get_src_line(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).getLine();
  }
  size_t ADDCALL sass_srcspan_get_src_column(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).getColumn();
  }
  size_t ADDCALL sass_srcspan_get_span_ln(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).span.line;
  }
  size_t ADDCALL sass_srcspan_get_span_col(struct SassSrcSpan* pstate)
  {
    return SourceSpan::unwrap(pstate).span.column;
  }

  struct SassSource* ADDCALL sass_srcspan_get_source(struct SassSrcSpan* pstate)
  {
    return SourceData::wrap(SourceSpan::unwrap(pstate).getSource());
  }

  /////////////////////////////////////////////////////////////////////////
  // Implementation related to sources
  /////////////////////////////////////////////////////////////////////////

  const char* ADDCALL sass_source_get_abs_path(struct SassSource* source)
  {
    return SourceData::unwrap(source).getAbsPath();
  }

  const char* ADDCALL sass_source_get_imp_path(struct SassSource* source)
  {
    return SourceData::unwrap(source).getImpPath();
  }

  const char* ADDCALL sass_source_get_content(struct SassSource* source)
  {
    return SourceData::unwrap(source).content();
  }

  const char* ADDCALL sass_source_get_srcmap(struct SassSource* source)
  {
    return SourceData::unwrap(source).srcmaps();
  }

}

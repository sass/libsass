#include "capi_error.hpp"

#include "json.hpp"

#ifdef __cplusplus
using namespace Sass;
extern "C" {
#endif

    int ADDCALL sass_error_get_status(const struct SassError* error) { return error->status; }
    char* ADDCALL sass_error_get_json(const struct SassError* error) { return error->getJson(true); }
    const char* ADDCALL sass_error_get_what(const struct SassError* error) { return error->what.c_str(); }
    // const char* ADDCALL sass_error_get_messages(struct SassError* error) { return error->messages86.c_str(); }
    // const char* ADDCALL sass_error_get_warnings(struct SassError* error) { return error->warnings86.c_str(); }
    const char* ADDCALL sass_error_get_formatted(const struct SassError* error) { return error->formatted.c_str(); }

    const char* ADDCALL sass_error_get_path(const struct SassError* error)
    {
      if (error->traces.empty()) return nullptr;
      return error->traces.back().pstate.getAbsPath();
    }

    size_t ADDCALL sass_error_get_line(const struct SassError* error)
    {
      if (error->traces.empty()) return 0;
      return error->traces.back().pstate.getLine();
    }
    size_t ADDCALL sass_error_get_column(const struct SassError* error)
    {
      if (error->traces.empty()) return 0;
      return error->traces.back().pstate.getColumn();
    }
    const char* ADDCALL sass_error_get_content(const struct SassError* error)
    {
      if (error->traces.empty()) return 0;
      return error->traces.back().pstate.getContent();
    }

    size_t ADDCALL sass_error_count_traces(const struct SassError* error)
    {
      return error->traces.size();
    }

    const struct SassTrace* ADDCALL sass_error_last_trace(const struct SassError* error)
    {
      if (error->traces.empty()) return nullptr;
      return error->traces.back().wrap();
    }

    const struct SassTrace* ADDCALL sass_error_get_trace(const struct SassError* error, size_t i)
    {
      if (error->traces.size() < i) return nullptr;
      return error->traces.at(i).wrap();
    }

#ifdef __cplusplus
} // __cplusplus defined.
#endif


// Create error formatted as serialized json.
// You must free the data returned from here.
// You must do so by using `sass_free_c_string`
char* SassError::getJson(bool include_sources) const
{

  // Create json root object
  JsonNode* json = json_mkobject();

  // Attach all stack traces
  if (traces.size() > 0) {
    JsonNode* json_traces = json_mkarray();
    for (const  Sass::StackTrace& trace : traces) {
      JsonNode* json_trace = json_mkobject();
      const Sass::SourceSpan& pstate(trace.pstate);
      json_append_member(json_trace, "file", json_mkstring(pstate.getAbsPath()));
      json_append_member(json_trace, "line", json_mknumber((double)(pstate.getLine())));
      json_append_member(json_trace, "column", json_mknumber((double)(pstate.getColumn())));
      if (include_sources) json_append_member(json_trace, "source", json_mkstring(pstate.getContent()));
      json_append_element(json_traces, json_trace);
    }
    json_append_member(json, "traces", json_traces);
  }

  // Attach the generic error reporting items
  json_append_member(json, "status", json_mknumber(status));
  json_append_member(json, "error", json_mkstring(what.c_str()));
  // json_append_member(json, "messages", json_mkstring(messages86.c_str()));
  // json_append_member(json, "warnings", json_mkstring(warnings86.c_str()));
  json_append_member(json, "formatted", json_mkstring(formatted.c_str()));

  char* serialized = nullptr;
  // Stringification may fail for strange reason
  try { serialized = json_stringify(json, "  "); }
  // If it fails at least return a valid json with special status 9999
  catch (...) { serialized = sass_copy_c_string("{\"status\":9999}"); }

  // Delete json tree
  json_delete(json);

  // Return new memory
  return serialized;

}

#ifndef SASS_EMITTER_H
#define SASS_EMITTER_H

#include <string>
#include "context.hpp"
#include "source_map.hpp"
#include "ast_fwd_decl.hpp"

namespace Sass {
  using namespace std;
  using namespace Sass;

  class OutputBuffer {
    public:
      string buffer;
      SourceMap srcmap;
      /*
      void operator+=(const string& txt)
      {
        buffer += txt;
      }
      */
  };

  class Emitter {
    public:
      Emitter(OutputBuffer& buf, Context* ctx, Output_Style style = NESTED);
      ~Emitter();

      // To_String* to_string;
      OutputBuffer* output;
      size_t indentation;
      Context* ctx;

      SourceMap source_map;

    public:
      void append_indent_to_buffer();
      void append_to_buffer(const string& text);
      void append_to_buffer(const string& text, AST_Node* node);
      void append_to_buffer(const string& text, AST_Node* node, const string& tail);

      void append_scope_opener();
      void append_scope_closer();
      void append_delimiter();

      string get_buffer() { return output->buffer; }

      virtual void append_open_bracket();
      virtual void append_close_bracket();
      virtual void append_open_parenthesis();
      virtual void append_close_parenthesis();

      virtual void append_colon_separator();
      virtual void append_space_separator();
      virtual void append_comma_separator();

      virtual void append_optional_space();
      virtual void append_mandatory_space();
      virtual void append_optional_linefeed();

      Output_Style output_style;

    protected:
      bool space_scheduled;
      bool linefeed_scheduled;
      bool delimiter_scheduled;

  };
}

#endif

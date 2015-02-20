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
      Emitter(OutputBuffer& buf, Context* ctx, bool tostr = false);
      ~Emitter();

      // To_String* to_string;
      OutputBuffer* output;
      size_t indentation;
      Context* ctx;
      string charset;
      SourceMap source_map;
      bool tostr;

    public:
      void append_indent_to_buffer();
      void append_to_buffer(const string& text);
      void append_to_buffer2(const string& text);
      void append_to_buffer(const string& text, AST_Node* node);
      void append_to_buffer(const string& text, AST_Node* node, const string& tail);

      void append_scope_opener();
      void append_scope_closer();
      void append_delimiter();
      void append_double_lf();

bool allow_before;
    bool in_media;
    bool in_raw_list;
    bool in_declaration;
    bool in_declaration_list;

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
      bool double_lf_scheduled;
      bool delimiter_scheduled;
      string wspace_scheduled;

  };
}

#endif

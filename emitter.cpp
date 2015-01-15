#include <iostream>
#include <stdexcept>

#include "output.hpp"
#include "emitter.hpp"

namespace Sass {
  using namespace std;

  Emitter::Emitter(OutputBuffer& buf, Context* ctx, Output_Style style)
  : output(&buf),
    indentation(0),
    ctx(ctx),
    output_style(style),
    space_scheduled(false),
    linefeed_scheduled(false),
    delimiter_scheduled(false)
  {
  }

  Emitter::~Emitter()
  {
  }

  void Emitter::append_indent_to_buffer()
  {
    if (output_style == NESTED) {
      string indent = "";
      for (size_t i = 0; i < indentation; i++)
        indent += ctx ? ctx->indent : "  ";
      append_to_buffer(indent);
    }
  }

  void Emitter::append_delimiter()
  {
    if (output_style == COMPRESSED) {
      delimiter_scheduled = true;
    } else {
      append_to_buffer(";");
      append_optional_linefeed();
    }
  }

  void Emitter::append_scope_opener()
  {
    append_to_buffer("{");
  }

  void Emitter::append_scope_closer()
  {
    delimiter_scheduled = false;
    append_to_buffer("}");
  }


  void Emitter::append_to_buffer(const string& text)
  {
    // forget scheduled space if space is added
    if (text.size() > 0 && text[0] == ' ') {
      space_scheduled = false;
    }
    if (text.size() > 0 && text[0] == '}') {
      if (output->buffer.size() && *output->buffer.rbegin() == '{') {
      space_scheduled = false;
    }
    }

    if (delimiter_scheduled) {
      delimiter_scheduled = false;
      append_to_buffer(";");
    }
    if (space_scheduled) {
      space_scheduled = false;
      if (!linefeed_scheduled)
        append_to_buffer(" ");
    }
    if (linefeed_scheduled && text != " }") {
      linefeed_scheduled = false;
      if (output_style == NESTED) {
        if (!ctx) append_to_buffer("\n");
        else append_to_buffer(ctx->linefeed);
      }
    }

    output->buffer += text;
    output->srcmap.update_column(text);
  }

  void Emitter::append_to_buffer(const string& text, AST_Node* node)
  {
    output->srcmap.add_open_mapping(node);
    append_to_buffer(text);
    output->srcmap.add_close_mapping(node);
  }

  void Emitter::append_to_buffer(const string& text, AST_Node* node, const string& tail)
  {
    append_to_buffer(text, node);
    append_to_buffer(tail);
  }

  void Emitter::append_optional_space()
  {

    if (linefeed_scheduled) return;
    if (output->buffer.size() == 0) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;

    if (output_style == NESTED)
      space_scheduled = true;
    else if (output_style == COMPACT)
      space_scheduled = true;
  }

  void Emitter::append_optional_linefeed()
  {
    linefeed_scheduled = true;
  }

  void Emitter::append_open_bracket()
  {
    append_optional_space();
    append_to_buffer("{");
    append_optional_linefeed();
    ++ indentation;
  }

  void Emitter::append_close_bracket()
  {
    -- indentation;
    linefeed_scheduled = false;
    append_optional_space();
    delimiter_scheduled = false;
    append_to_buffer("}");
    append_optional_linefeed();
    // append_indent_to_buffer();
  }
  void Emitter::append_open_parenthesis()
  {
    append_to_buffer("(");
  }
  void Emitter::append_close_parenthesis()
  {
    append_to_buffer(")");
  }

  void Emitter::append_colon_separator()
  {
    append_to_buffer(":");
    append_optional_space();
  }
  void Emitter::append_comma_separator()
  {
    append_to_buffer(",");
    append_optional_space();
  }
  void Emitter::append_space_separator()
  {
    append_to_buffer(" ");
  }


}

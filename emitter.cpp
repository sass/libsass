#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "output.hpp"
#include "emitter.hpp"
#include "utf8.h"
#include "utf8_string.hpp"

namespace Sass {
  using namespace std;

  Emitter::Emitter(OutputBuffer& buf, Context* ctx, bool asd)
  : output(&buf),
    indentation(0),
    ctx(ctx),
    charset(""),
    tostr(asd),
    allow_before(false),
    in_media(false),
    in_raw_list(false),
    in_declaration(false),
    in_declaration_list(false),
    output_style(ctx ? ctx->output_style : COMPRESSED),
    space_scheduled(false),
    linefeed_scheduled(false),
    double_lf_scheduled(false),
    delimiter_scheduled(false),
    wspace_scheduled("")
  {
  }

  Emitter::~Emitter()
  {
  }

  void Emitter::append_double_lf()
  {
  	if (output_style == COMPRESSED) return;
    if (output->buffer.size() == 0) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;

if (output_style == COMPACT) {
  	space_scheduled = false;
  	linefeed_scheduled =false;
  	double_lf_scheduled = false;
        if (!ctx) append_to_buffer("\n");
        else append_to_buffer(ctx->linefeed);
} else {
  	space_scheduled = false;
  	linefeed_scheduled =false;
  	double_lf_scheduled = true;
  }
  }

  void Emitter::append_indent_to_buffer()
  {

    if (output_style == NESTED || output_style == EXPANDED) {
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
    } else if (output_style == COMPACT) {
      append_to_buffer("; ");
    } else {
      // space_scheduled = false;
      // delimiter_scheduled = false;
//       linefeed_scheduled = false;
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
    if (output_style == EXPANDED) {
      append_optional_linefeed();
      append_indent_to_buffer();
    }
    append_to_buffer("}");
  }


  void Emitter::append_to_buffer2(const string& text)
  {
    bool space = false;
    bool linefeed = false;
    for(auto chr : text) {
      if (chr == ' ') space = true;
      if (chr == '\t') space = true;
      if (chr == '\r') linefeed = true;
      if (chr == '\n') linefeed = true;
    }

    if (in_raw_list) {
      space_scheduled = false;
      linefeed_scheduled = false;
      double_lf_scheduled = false;
      wspace_scheduled += text;
    } else if (in_declaration_list) {
      if (linefeed) append_optional_space();
      else if (space) append_optional_space();
    } else {
  //    space_scheduled = false;
      linefeed_scheduled = false;
      double_lf_scheduled = false;
      if (linefeed) append_optional_linefeed();
      else if (space) append_optional_space();
    }
    // output->buffer += text;
  }

  void Emitter::append_to_buffer(const string& text)
  {

    if (text.size() > 0 && text[0] == '{') {
    	wspace_scheduled = "";
   }

  	if (wspace_scheduled.size() > 0) {
  	  output->buffer += wspace_scheduled;
  	  wspace_scheduled = "";
  	}
    // cerr << "write [" << text << "]" << endl;

    char lst = output->buffer.size() ? *output->buffer.rbegin() : '\0';

    if (lst == ' ' && text == " ") return;

    // forget scheduled space if space is added
    if (text.size() > 0 && text[0] == ' ') {
      space_scheduled = false;
    }
    if (text.size() > 0 && text[0] == ',') {
      space_scheduled = false;
    }
    if (text.size() > 0 && text[0] == ':') {
    //  space_scheduled = false;
    }
    if (output_style == COMPRESSED) {
      if (text.size() > 0 && text[0] == '+') {
        space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == '-') {
        space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == '>') {
        space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == '~') {
      //  space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == '(') {
        space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == '{') {
        space_scheduled = false;
      }
      if (text.size() > 0 && text[0] == ':') {
        space_scheduled = false;
      }
    }
    if (text.size() > 0 && text[0] == '}') {
      if (output_style == COMPRESSED)
        space_scheduled = false;
    }
    if (text.size() > 0 && text[0] == '}') {
      if (output->buffer.size() && lst == '{') {
      space_scheduled = false;
    }
    }
/*
    cerr << "write [" << text << "]" << endl;
    if (text == ";") {
      space_scheduled = false;
      delimiter_scheduled = false;
      linefeed_scheduled = false;
    }
*/
    if (delimiter_scheduled) {
  //    space_scheduled = false;
      delimiter_scheduled = false;
      append_to_buffer(";");
      if (output_style == COMPACT) {
        space_scheduled = true;
        append_to_buffer(" ");
      }
    }
    if (text == " " && space_scheduled) {
      space_scheduled = true; return;
    }

    if (space_scheduled) {
      space_scheduled = false;
      if (output_style == COMPRESSED) {
      if (lst != ' ' && lst != '(' && lst != '[' && lst != ',' && !linefeed_scheduled)
        append_to_buffer(" ");
      } else {
      if (lst != ' ' && lst != '(' && lst != '[' && !linefeed_scheduled)
        append_to_buffer(" ");
      }
    }

    if (text[0] == ' ') {
     if (double_lf_scheduled) {
      double_lf_scheduled = false;
      linefeed_scheduled = true;
    }
    }

    if (linefeed_scheduled) { //  && text != " }"
      linefeed_scheduled = false;
      double_lf_scheduled = false;
      if (output_style != COMPRESSED) {
      // if (output_style != COMPRESSED) {
        if (!ctx) append_to_buffer("\n");
        else append_to_buffer(ctx->linefeed);
      } else { /* space_scheduled = true; */ }
    }

    if (double_lf_scheduled) { //  && text != " }"
      linefeed_scheduled = false;
      double_lf_scheduled = false;
      if (output_style == NESTED || output_style == EXPANDED) {
        if (!ctx) append_to_buffer("\n");
        else append_to_buffer(ctx->linefeed);
        // linefeed_scheduled = true;
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
    if (output_style == COMPRESSED && ends_with(output->buffer, "+")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, ">")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, "(")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, ":")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, ",")) return;
    // if (output_style == COMPRESSED && ends_with(output->buffer, "~")) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;

    space_scheduled = true;

  }

  void Emitter::append_mandatory_space()
  {

    if (output->buffer.size() == 0) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, "+")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, ">")) return;
    if (output_style == COMPRESSED && ends_with(output->buffer, ",")) return;
    if ( ends_with(output->buffer, "(")) return;
//    if ( ends_with(output->buffer, ")")) return;
    if ( ends_with(output->buffer, "}")) return;
    if (output_style != COMPACT && ends_with(output->buffer, "{")) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;

  	if (output_style != COMPRESSED) {
  	space_scheduled = true;
  	append_to_buffer("");
  } else {
  	space_scheduled = true;
  	append_to_buffer(" ");
  }
  	return;

    if (linefeed_scheduled) return;
    // if (output_style == COMPRESSED && ends_with(output->buffer, "~")) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;

    if (output->buffer.size() == 0 ||
        !ends_with(output->buffer, " "))
            space_scheduled = true;
  }

  void Emitter::append_optional_linefeed()
  {
    if (output->buffer.size() == 0) return;
    if (ends_with(output->buffer, "\r")) return;
    if (ends_with(output->buffer, "\n")) return;
    // append_to_buffer("\n");
    linefeed_scheduled = true;
  }

  void Emitter::append_open_bracket()
  {
    append_optional_space();
    append_to_buffer("{");
    if (output_style == COMPACT)
      append_mandatory_space();
    else
      append_optional_linefeed();
    ++ indentation;
  }

  void Emitter::append_close_bracket()
  {
    -- indentation;
    linefeed_scheduled = false;
    if (output_style == EXPANDED) {
      append_optional_linefeed();
      append_indent_to_buffer();
    } else {
      append_optional_space();

    }
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
    if (in_media) {
      append_to_buffer(": ");
    } else {
      append_to_buffer(":");
      append_optional_space();
    }
  }
  void Emitter::append_comma_separator()
  {
    if (in_media) {
      append_to_buffer(", ");
    } else {
      append_to_buffer(",");
      append_optional_space();
    }
  }
  void Emitter::append_space_separator()
  {
    append_to_buffer(" ");
  }


}

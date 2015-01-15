#include <string>
#include <vector>
#include "output.hpp"

bool outdbg = false;

namespace Sass {
  using namespace std;

  Output::Output(OutputBuffer& buf, Context* ctx, Output_Style style)
  : Inspect(Emitter(buf, ctx, style)),
    ctx(ctx),
    top_imports(0),
    top_comments(0),
    source_comments(false),
    in_directive(false),
    in_keyframes(false)
  {
  	// cerr << "new output " << style << endl;

  	}

  Output::~Output() { };

  void Output::operator()(Import* imp)
  {
    top_imports.push_back(imp);
  };

  void Output::operator()(Comment* c)
  {
    if (output->buffer.size() + top_imports.size() == 0) {
      top_comments.push_back(c);
    } else {
      To_String to_string;
      string txt = c->text()->perform(&to_string);
      Emitter emitter(*output, ctx, output_style);
      append_optional_linefeed();
      append_indent_to_buffer();
      Inspect i(emitter);
      c->perform(&i);
      append_optional_linefeed();
      // append_to_buffer(i.get_buffer());
    }
  };

  void Output::fallback_impl(AST_Node* n)
  {
    if (outdbg) cerr << "Output fallback\n";
    Inspect i(*this);
    // this needs to use the same buffer?
    // or at least a chance to pass one!
    // cerr << "perform " << *this->buffer << endl;
    n->perform(&i);
    // append_to_buffer(i.get_buffer());
  }

  string Output::get_buffer(void)
  {
    string charset("");

    OutputBuffer buffer2;
    Emitter emitter(buffer2, ctx, output_style);

    Inspect comments(emitter);
    size_t size_com = top_comments.size();
    for (size_t i = 0; i < size_com; i++) {
      top_comments[i]->perform(&comments);
      comments.append_optional_linefeed();
    }
    comments.append_to_buffer("");

    Inspect imports(emitter);
    size_t size_imp = top_imports.size();
    for (size_t i = 0; i < size_imp; i++) {
      top_imports[i]->perform(&imports);
      imports.append_optional_linefeed();
    }
    comments.append_to_buffer("");
    imports.append_to_buffer("");

    // create combined buffer string
    string buffer = emitter.output->buffer
                  + this->output->buffer;
// cerr << "get buffer for " << output_style << endl;
    // make sure we end with a linefeed
    if (!ends_with(buffer, ctx->linefeed)) {
      // if the output is not completely empty
      if (!buffer.empty()) buffer += ctx->linefeed;
    }

    // search for unicode char
    for(const char& chr : buffer) {
      // skip all ascii chars
      if (chr >= 0) continue;
      // declare the charset
      charset = "@charset \"UTF-8\";";
      // abort search
      break;
    }

    // add charset as the very first line, before top comments and imports
    return (charset.empty() ? "" : charset + ctx->linefeed) + buffer;
  }


  void Output::operator()(Ruleset* r)
  {
    if (outdbg) cerr <<  "Output Ruleset " << r->tabs() << "\n";
    Selector* s     = r->selector();
    Block*    b     = r->block();
    bool      decls = false;

    // disabled to avoid clang warning [-Wunused-function]
    // Selector_List* sl = static_cast<Selector_List*>(s);

    // Filter out rulesets that aren't printable (process its children though)
    if (!Util::isPrintable(r)) {
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (dynamic_cast<Has_Block*>(stm)) {
          stm->perform(this);
        }
      }
      return;
    }
    if (b->has_non_hoistable()) {
      decls = true;
      indentation += r->tabs();
      if (source_comments) {
        stringstream ss;
        append_indent_to_buffer();
        ss << "/* line " << r->pstate().line+1 << ", " << r->pstate().path << " */";
        append_to_buffer(ss.str());
        append_optional_linefeed();
        // append_indent_to_buffer();
      }
      s->perform(this);
      append_open_bracket();
      // append_to_buffer(" {" + ctx->linefeed);
      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        bool bPrintExpression = true;
        // Check print conditions
        if (typeid(*stm) == typeid(Declaration)) {
          Declaration* dec = static_cast<Declaration*>(stm);
          if (dec->value()->concrete_type() == Expression::STRING) {
            String_Constant* valConst = static_cast<String_Constant*>(dec->value());
            string val(valConst->value());
            if (val.empty()) {
              bPrintExpression = false;
            }
          }
          else if (dec->value()->concrete_type() == Expression::LIST) {
            List* list = static_cast<List*>(dec->value());
            bool all_invisible = true;
            for (size_t list_i = 0, list_L = list->length(); list_i < list_L; ++list_i) {
              Expression* item = (*list)[list_i];
              if (!item->is_invisible()) all_invisible = false;
            }
            if (all_invisible) bPrintExpression = false;
          }
        }
        // Print if OK
        if (!stm->is_hoistable() && bPrintExpression) {
          // if (!stm->block()) append_indent_to_buffer();
          stm->perform(this);
          // if (i < L-1) append_optional_linefeed();
        }
      }
      // --indentation;
      indentation -= r->tabs();

//      while (buffer.substr(buffer.length()-ctx->linefeed.length()) == ctx->linefeed) {
//        buffer.erase(buffer.length()-1);
//        if (ctx) ctx->source_map.remove_line();
//      }

      // append_to_buffer(" }");
      append_close_bracket();
      // if (r->group_end()) append_optional_linefeed();
      // Match Sass 3.4.9 behaviour
      // if (in_directive && !in_keyframes) append_optional_linefeed();
    }

    if (b->has_hoistable()) {
      if (decls) ++indentation;
      // append_indent_to_buffer();
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (stm->is_hoistable()) {
          stm->perform(this);
        }
      }
      if (decls) --indentation;
    }
  }

  void Output::operator()(Feature_Block* f)
  {
    if (outdbg) cerr <<  "Output Feature_Block\n";
    if (f->is_invisible()) return;

    Feature_Query* q    = f->feature_queries();
    Block* b            = f->block();

    // Filter out feature blocks that aren't printable (process its children though)
    if (!Util::isPrintable(f)) {
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (dynamic_cast<Has_Block*>(stm)) {
          stm->perform(this);
        }
      }
      return;
    }

    indentation += f->tabs();
    append_indent_to_buffer();
    append_to_buffer("@supports", f);
    append_optional_space();
    q->perform(this);
append_open_bracket();
    // append_to_buffer(" ");
    // append_to_buffer("{");
    // append_optional_linefeed();

    bool old_in_directive = in_directive;
    in_directive = true;

    Selector* e = f->selector();
    if (e && b->has_non_hoistable()) {
      // JMA - hoisted, output the non-hoistable in a nested block, followed by the hoistable
      // ++indentation;
      // append_indent_to_buffer();
      e->perform(this);
      append_open_bracket();
      // append_to_buffer(" {" + ctx->linefeed);

      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable()) {
          // if (!stm->block()) append_indent_to_buffer();
          stm->perform(this);
          if (i < L-1) append_optional_linefeed();
        }
      }
      // --indentation;

      in_directive = old_in_directive;

      // buffer.erase(buffer.length()-1);
      // if (ctx) ctx->source_map.remove_line();
      append_close_bracket();
      // append_to_buffer(" }" + ctx->linefeed);
      // --indentation;

      // ++indentation;
      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (stm->is_hoistable()) {
          stm->perform(this);
        }
      }
      // --indentation;
      // --indentation;
    }
    else {
      // JMA - not hoisted, just output in order
      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable()) {
          // if (!stm->block()) append_indent_to_buffer();
        }
        stm->perform(this);
        if (!stm->is_hoistable()) append_optional_linefeed();
      }
      // --indentation;
    }

//    while (buffer.substr(buffer.length()-ctx->linefeed.length()) == ctx->linefeed) {
//      buffer.erase(buffer.length()-1);
//      if (ctx) ctx->source_map.remove_line();
//    }

    in_directive = old_in_directive;

    append_close_bracket();
    // append_to_buffer(" }");
    // if (f->group_end() || in_directive) append_optional_linefeed();

    indentation -= f->tabs();
  }

  void Output::operator()(Media_Block* m)
  {
    if (outdbg) cerr <<  "Output Media_Block\n";
    if (m->is_invisible()) return;

    List*  q     = m->media_queries();
    Block* b     = m->block();

    // Filter out media blocks that aren't printable (process its children though)
    if (!Util::isPrintable(m)) {
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (dynamic_cast<Has_Block*>(stm)) {
          stm->perform(this);
        }
      }
      return;
    }
// m->tabs(4);
    indentation += m->tabs();
    append_indent_to_buffer();
    append_to_buffer("@media", m);
    append_to_buffer(" ");
    q->perform(this);
    append_open_bracket();
    // append_to_buffer(" ");
    // append_to_buffer("{");
    // append_optional_linefeed();

    bool old_in_directive = in_directive;
    in_directive = true;

    Selector* e = m->selector();
    if (e && b->has_non_hoistable()) {
      // JMA - hoisted, output the non-hoistable in a nested block, followed by the hoistable
      // ++indentation;
      // append_indent_to_buffer();
      e->perform(this);
      append_open_bracket();
      // append_to_buffer(" {" + ctx->linefeed);

      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable()) {
          // if (!stm->block()) append_indent_to_buffer();
          stm->perform(this);
          if (i < L-1) append_optional_linefeed();
        }
      }
      // --indentation;

      in_directive = old_in_directive;

      // buffer.erase(buffer.length()-1);
      // if (ctx) ctx->source_map.remove_line();
      append_close_bracket();
      // append_to_buffer(" }" + ctx->linefeed);
      // --indentation;

      // ++indentation;
      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (stm->is_hoistable()) {
          stm->perform(this);
        }
      }
      // --indentation;
      // --indentation;
    }
    else {
      // JMA - not hoisted, just output in order
      // ++indentation;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable()) {
          // if (!stm->block()) append_indent_to_buffer();
        }
        stm->perform(this);
        if (!stm->is_hoistable()) append_optional_linefeed();
      }
      // --indentation;
    }

//    while (buffer.substr(buffer.length()-ctx->linefeed.length()) == ctx->linefeed) {
//      buffer.erase(buffer.length()-1);
//      if (ctx) ctx->source_map.remove_line();
//    }

    in_directive = old_in_directive;

    append_close_bracket();
    // append_to_buffer(" }");
    // if (m->group_end() || in_directive) append_optional_linefeed();

    indentation -= m->tabs();
  }

  void Output::operator()(At_Rule* a)
  {
    if (outdbg) cerr <<  "Output At_Rule\n";
    string      kwd   = a->keyword();
    Selector*   s     = a->selector();
    Expression* v     = a->value();
    Block*      b     = a->block();
    // bool        decls = false;

    in_keyframes = kwd.compare("@keyframes") == 0;
    if (!in_keyframes) indentation += a->tabs();

    // append_indent_to_buffer();
    append_to_buffer(kwd);
    if (s) {
      append_optional_space();
      s->perform(this);
    }
    else if (v) {
      append_optional_space();
      v->perform(this);
    }

    if (!b) {
      append_delimiter();
      return;
    }

    append_open_bracket();
    // append_to_buffer(" {" + ctx->linefeed);

    bool old_in_directive = in_directive;
    in_directive = true;

    // ++indentation;
    // decls = true;
    for (size_t i = 0, L = b->length(); i < L; ++i) {
      Statement* stm = (*b)[i];
      if (!stm->is_hoistable()) {
        // if (!stm->block()) append_indent_to_buffer();
        stm->perform(this);
        append_optional_linefeed();
      }
    }
    // --indentation;

    // if (decls) ++indentation;
    for (size_t i = 0, L = b->length(); i < L; ++i) {
      Statement* stm = (*b)[i];
      if (stm->is_hoistable()) {
        stm->perform(this);
        // append_optional_linefeed();
      }
    }
    // if (decls) --indentation;

//    while (buffer.substr(buffer.length()-ctx->linefeed.length()) == ctx->linefeed) {
//      buffer.erase(buffer.length()-1);
//      if (ctx) ctx->source_map.remove_line();
//    }

    if (!in_keyframes) indentation -= a->tabs();

    in_directive = old_in_directive;
    in_keyframes = false;

    append_close_bracket();
    // append_to_buffer(" }");

    // Match Sass 3.4.9 behaviour
    if (kwd.compare("@font-face") != 0 && kwd.compare("@keyframes") != 0) append_optional_linefeed();
  }

}

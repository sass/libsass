#include "inspect.hpp"
#include "ast.hpp"
#include "util.hpp"
#include "context.hpp"
#include "utf8/checked.h"
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>

namespace Sass {
  using namespace std;

  Inspect::Inspect(Emitter emi, bool output)
  : Emitter(emi), is_output(output)
  {
    disable_quotes = false;
    in_wrapped = false;

    // if (emi.buffer) buffer = emi.buffer;
    // if (emi) Emitter(*emi);
  }
  Inspect::~Inspect() { }

  // statements
  void Inspect::operator()(Block* block)
  {
    // cerr << "INSPECT Block\n";
    if (!block->is_root()) {
      append_open_bracket();
    }
    if (output_style == NESTED) indentation += block->tabs();
    for (size_t i = 0, L = block->length(); i < L; ++i) {
      // append_indent_to_buffer();
      (*block)[i]->perform(this);
      // extra newline at the end of top-level statements
//      if (block->is_root()) append_optional_linefeed();
//      append_optional_linefeed();
    }
    if (output_style == NESTED) indentation -= block->tabs();
    if (!block->is_root()) {
      append_close_bracket();
    }

  }

  void Inspect::operator()(Ruleset* ruleset)
  {
    ruleset->selector()->perform(this);
    ruleset->block()->perform(this);
  }

  void Inspect::operator()(Keyframe_Rule* rule)
  {
    append_indent_to_buffer();
    if (rule->rules()) rule->rules()->perform(this);
    rule->block()->perform(this);
  }

  void Inspect::operator()(Propset* propset)
  {
    propset->property_fragment()->perform(this);
    append_colon_separator();
    propset->block()->perform(this);
  }

  void Inspect::operator()(Bubble* bubble)
  {
    append_indent_to_buffer();
    append_to_buffer("Bubble");
    append_optional_space();
    append_to_buffer("(");
    append_optional_space();
    bubble->node()->perform(this);
    append_optional_space();
    append_to_buffer(")");
    append_optional_space();
  }

  void Inspect::operator()(Media_Block* media_block)
  {
    append_indent_to_buffer();
    append_to_buffer("@media", media_block);
    append_mandatory_space();
    in_media = true;
    media_block->media_queries()->perform(this);
    in_media = false;
    media_block->block()->perform(this);
  }

  void Inspect::operator()(Feature_Block* feature_block)
  {
    append_indent_to_buffer();
    append_to_buffer("@supports", feature_block);
    append_mandatory_space();
    feature_block->feature_queries()->perform(this);
    feature_block->block()->perform(this);
  }

  void Inspect::operator()(At_Root_Block* at_root_block)
  {
    append_indent_to_buffer();
    append_to_buffer("@at-root ", at_root_block);
    append_mandatory_space();
    if(at_root_block->expression()) at_root_block->expression()->perform(this);
    at_root_block->block()->perform(this);
  }

  void Inspect::operator()(At_Rule* at_rule)
  {
  	in_raw_list = true;
    append_indent_to_buffer();
    append_to_buffer(at_rule->keyword());
    if (at_rule->selector()) {
      append_mandatory_space();
      at_rule->selector()->perform(this);
    }
    if (at_rule->block()) {
      at_rule->block()->perform(this);
    }
    else {
      append_delimiter();
    }
  	in_raw_list = false;
  }

  void Inspect::operator()(Declaration* dec)
  {
    if (dec->value()->concrete_type() == Expression::NULL_VAL) return;
    in_declaration = true;
    if (output_style == NESTED)
      indentation += dec->tabs();
    append_indent_to_buffer();
    source_map.add_open_mapping(dec->property());
    dec->property()->perform(this);
    source_map.add_close_mapping(dec->property());
    append_colon_separator();
    source_map.add_open_mapping(dec->value());
// debug_ast(dec);
    dec->value()->perform(this);

    if (dec->is_important()) {
      append_optional_space();
      append_to_buffer("!important");
    }
    source_map.add_close_mapping(dec->value());
    append_delimiter();
    if (output_style == NESTED) indentation -= dec->tabs();
    in_declaration = false;
  }

  void Inspect::operator()(Assignment* assn)
  {
    append_to_buffer(assn->variable());
    append_colon_separator();
    assn->value()->perform(this);
    if (assn->is_guarded()) {
      append_optional_space();
      append_to_buffer("!default");
    }
    append_delimiter();
  }

  void Inspect::operator()(Import* import)
  {
    if (!import->urls().empty()) {
      append_to_buffer("@import", import);
      append_mandatory_space();

      if (String_Quoted* strq = dynamic_cast<String_Quoted*>(import->urls().front())) {
        // should make a copy or sometrhing
        // strq->value(quote(strq->value(), '"'));
        // strq->was_quoted(false);
        strq->is_delayed(false);
      }

      import->urls().front()->perform(this);
      // import->urls()[i]->value(quote(import->urls()[i]->value(), '"'));

      append_delimiter();
      for (size_t i = 1, S = import->urls().size(); i < S; ++i) {
        append_optional_linefeed();
        append_to_buffer("@import", import);
        append_mandatory_space();
        // force quotes for inspect
        if (String_Quoted* strq = dynamic_cast<String_Quoted*>(import->urls()[i])) {
          // should make a copy or sometrhing
          // strq->value(quote(strq->value(), '"'));
          // strq->was_quoted(false);
          strq->is_delayed(false);
        }

        import->urls()[i]->perform(this);
        append_delimiter();
      }
    }
    // cerr << output->buffer << endl;
  }

  void Inspect::operator()(Import_Stub* import)
  {
    append_indent_to_buffer();
    append_to_buffer("@import", import);
    append_mandatory_space();
    append_to_buffer(import->file_name());
    append_delimiter();
  }

  void Inspect::operator()(Warning* warning)
  {
    append_indent_to_buffer();
    append_to_buffer("@warn", warning);
    append_mandatory_space();
    warning->message()->perform(this);
    append_delimiter();
  }

  void Inspect::operator()(Error* error)
  {
    append_indent_to_buffer();
    append_to_buffer("@error", error);
    append_mandatory_space();
    error->message()->perform(this);
    append_delimiter();
  }

  void Inspect::operator()(Debug* debug)
  {
    append_indent_to_buffer();
    append_to_buffer("@debug", debug);
    append_mandatory_space();
    debug->value()->perform(this);
    append_delimiter();
  }

  void Inspect::operator()(Comment* comment)
  {
    append_optional_linefeed();
    append_indent_to_buffer();
    comment->text()->perform(this);
  }

  void Inspect::operator()(If* cond)
  {
    append_indent_to_buffer();
    append_to_buffer("@if", cond);
    append_mandatory_space();
    cond->predicate()->perform(this);
    cond->consequent()->perform(this);
    if (cond->alternative()) {
      append_optional_linefeed();
      append_indent_to_buffer();
      append_to_buffer("else");
      cond->alternative()->perform(this);
    }
  }

  void Inspect::operator()(For* loop)
  {
    append_indent_to_buffer();
    append_to_buffer("@for", loop);
    append_mandatory_space();
    append_to_buffer(loop->variable());
    append_to_buffer(" from ");
    loop->lower_bound()->perform(this);
    append_to_buffer((loop->is_inclusive() ? " through " : " to "));
    loop->upper_bound()->perform(this);
    loop->block()->perform(this);
  }

  void Inspect::operator()(Each* loop)
  {
    append_indent_to_buffer();
    append_to_buffer("@each", loop);
    append_mandatory_space();
    append_to_buffer(loop->variables()[0]);
    for (size_t i = 1, L = loop->variables().size(); i < L; ++i) {
      append_comma_separator();
      append_to_buffer(loop->variables()[i]);
    }
    append_to_buffer(" in ");
    loop->list()->perform(this);
    loop->block()->perform(this);
  }

  void Inspect::operator()(While* loop)
  {
    append_indent_to_buffer();
    append_to_buffer("@while", loop);
    append_mandatory_space();
    loop->predicate()->perform(this);
    loop->block()->perform(this);
  }

  void Inspect::operator()(Return* ret)
  {
    append_indent_to_buffer();
    append_to_buffer("@return", ret);
    append_mandatory_space();
    ret->value()->perform(this);
    append_delimiter();
  }

  void Inspect::operator()(Extension* extend)
  {
    append_indent_to_buffer();
    append_to_buffer("@extend", extend);
    append_mandatory_space();
    extend->selector()->perform(this);
    append_delimiter();
  }

  void Inspect::operator()(Definition* def)
  {
    if (def->type() == Definition::MIXIN) {
      append_to_buffer("@mixin", def);
      append_mandatory_space();
    } else {
      append_to_buffer("@function", def);
      append_mandatory_space();
    }
    append_to_buffer(def->name());
    def->parameters()->perform(this);
    def->block()->perform(this);
  }

  void Inspect::operator()(Mixin_Call* call)
  {
    append_indent_to_buffer();
    append_to_buffer("@include", call);
    append_mandatory_space();
    append_to_buffer(call->name());
    if (call->arguments()) {
      call->arguments()->perform(this);
    }
    if (call->block()) {
      append_optional_space();
      call->block()->perform(this);
    }
    if (!call->block()) append_delimiter();
  }

  void Inspect::operator()(Content* content)
  {
    append_indent_to_buffer();
    append_to_buffer("@content", content, ";");
  }

  void Inspect::operator()(Map* map)
  {
    if (map->empty()) return;
    if (map->is_invisible()) return;
    bool items_output = false;
    append_to_buffer("(");
    for (auto key : map->keys()) {
      if (key->is_invisible()) continue;
      if (map->at(key)->is_invisible()) continue;
      if (items_output) append_to_buffer(", ");
      // if (items_output) append_comma_separator();
      key->perform(this);
      // append_colon_separator();
      append_to_buffer(": ");
      map->at(key)->perform(this);
      items_output = true;
    }
    append_to_buffer(")");
  }

  void Inspect::operator()(List* list)
  {
  	// cerr << "INSPECT LIST ==\n";
  	bool add = false;
    string sep(list->separator() == List::SPACE ? " " : ",");
    if (in_media && sep == "," && !in_declaration_list) sep = ", ";
    else if (list->is_inspected() && list->separator() == List::COMMA) sep = ", ";
    else if (list->is_inspecting() && list->separator() == List::COMMA && !in_declaration_list) sep = ",";
    	if (list->is_inspecting() && list->separator() == List::COMMA && !in_declaration_list) add = true;
    if (list->empty()) return;
    bool items_output = false;
    in_declaration_list = in_declaration;
    for (size_t i = 0, L = list->length(); i < L; ++i) {
      Expression* list_item = (*list)[i];
      if (list_item->is_invisible()) {
        continue;
      }
      // debug_ast(list_item, "LI: ");
      if (items_output) {
      	if (!list_item->pstate().token.ws_before().empty()) {
      		append_to_buffer(sep);
          append_to_buffer2(list_item->pstate().token.ws_before());
        } else {
        	// if (sep != " ") {
          append_to_buffer(sep + (add ? "" : ""));
          // }
        }
/*
      	if (list_item->pstate().wspace_before.size() == 0) {
 // append_optional_space();
      	} else {
      	  append_to_buffer2(list_item->pstate().wspace_before);
      	}
*/
      }
      if (items_output && sep != " ") append_optional_space();
      // if (items_output && sep != " ") append_optional_linefeed();
      // if (items_output && sep != " ") append_indent_to_buffer();
      list_item->perform(this);
      items_output = true;
    }
    in_declaration_list = false;
  }

  void Inspect::operator()(Binary_Expression* expr)
  {
    expr->left()->perform(this);
    switch (expr->type()) {
      case Binary_Expression::AND: append_to_buffer(" and "); break;
      case Binary_Expression::OR:  append_to_buffer(" or ");  break;
      case Binary_Expression::EQ:  append_to_buffer(" == ");  break;
      case Binary_Expression::NEQ: append_to_buffer(" != ");  break;
      case Binary_Expression::GT:  append_to_buffer(" > ");   break;
      case Binary_Expression::GTE: append_to_buffer(" >= ");  break;
      case Binary_Expression::LT:  append_to_buffer(" < ");   break;
      case Binary_Expression::LTE: append_to_buffer(" <= ");  break;
      case Binary_Expression::ADD: append_to_buffer(" + ");   break;
      case Binary_Expression::SUB: append_to_buffer(" - ");   break;
      case Binary_Expression::MUL: append_to_buffer(" * ");   break;
      case Binary_Expression::DIV: append_to_buffer("/");     break;
      case Binary_Expression::MOD: append_to_buffer(" % ");   break;
      default: break; // shouldn't get here
    }
    expr->right()->perform(this);
  }

  void Inspect::operator()(Unary_Expression* expr)
  {
    if (expr->type() == Unary_Expression::PLUS) append_to_buffer("+");
    else                                        append_to_buffer("-");
    expr->operand()->perform(this);
  }

  void Inspect::operator()(Function_Call* call)
  {
    append_to_buffer(call->name());
    in_media = true;
    call->arguments()->perform(this);
    in_media = false;
  }

  void Inspect::operator()(Function_Call_Schema* call)
  {
    call->name()->perform(this);
    call->arguments()->perform(this);
  }

  void Inspect::operator()(Variable* var)
  {
    append_to_buffer(var->name());
  }

  void Inspect::operator()(Textual* txt)
  {
    append_to_buffer(txt->value());
  }

  // helper functions for serializing numbers
  // string frac_to_string(double f, size_t p) {
  //   stringstream ss;
  //   ss.setf(ios::fixed, ios::floatfield);
  //   ss.precision(p);
  //   ss << f;
  //   string result(ss.str().substr(f < 0 ? 2 : 1));
  //   size_t i = result.size() - 1;
  //   while (result[i] == '0') --i;
  //   result = result.substr(0, i+1);
  //   return result;
  // }
  // string double_to_string(double d, size_t p) {
  //   stringstream ss;
  //   double ipart;
  //   double fpart = std::modf(d, &ipart);
  //   ss << ipart;
  //   if (fpart != 0) ss << frac_to_string(fpart, 5);
  //   return ss.str();
  // }

  void Inspect::operator()(Number* n)
  {
    stringstream ss;
    ss.precision(ctx ? ctx->precision : 5);
    ss << fixed << n->value();
    string d(ss.str());
    // store if the value did not equal zero
    // if after applying precsision, the value gets
    // truncated to zero, sass emits 0.0 instead of 0
    bool nonzero = n->value() != 0;
    for (size_t i = d.length()-1; d[i] == '0'; --i) {
      d.resize(d.length()-1);
    }
    if (d[d.length()-1] == '.') d.resize(d.length()-1);
    if (n->numerator_units().size() > 1 ||
        n->denominator_units().size() > 0 ||
        (n->numerator_units().size() && n->numerator_units()[0].find_first_of('/') != string::npos) ||
        (n->numerator_units().size() && n->numerator_units()[0].find_first_of('*') != string::npos)
    ) {
      error(d + n->unit() + " isn't a valid CSS value.", n->pstate());
    }
    if (!n->zero() && !in_declaration_list) {
      if (d.substr(0, 3) == "-0.") d.erase(1, 1);
      if (d.substr(0, 2) == "0.") d.erase(0, 1);
    }
    // remove the leading minus
    if (d == "-0") d.erase(0, 1);
    // use fractional output if we had
    // a value before it got truncated
    if (d == "0" && nonzero) d = "0.0";
    // append number and unit
    append_to_buffer(d);
    append_to_buffer(n->unit());
  }

  // helper function for serializing colors
  template <size_t range>
  static double cap_channel(double c) {
    if      (c > range) return range;
    else if (c < 0)     return 0;
    else                return c;
  }

  inline bool is_doublet(double n) {
    return n == 0x00 || n == 0x11 || n == 0x22 || n == 0x33 ||
           n == 0x44 || n == 0x55 || n == 0x66 || n == 0x77 ||
           n == 0x88 || n == 0x99 || n == 0xAA || n == 0xBB ||
           n == 0xCC || n == 0xDD || n == 0xEE || n == 0xFF ;
  }

  inline bool is_color_doublet(double r, double g, double b) {
    return is_doublet(r) && is_doublet(g) && is_doublet(b);
  }

  void Inspect::operator()(Color* c)
  {
    stringstream ss;

    // original color name
    // maybe an unknown token
    string name = c->disp();

    // resolved color
    string res_name = name;

    double r = round(cap_channel<0xff>(c->r()));
    double g = round(cap_channel<0xff>(c->g()));
    double b = round(cap_channel<0xff>(c->b()));
    double a = cap_channel<1>   (c->a());

    if (name != "" && ctx && ctx->names_to_colors.count(name)) {
      Color* n = ctx->names_to_colors[name];
      r = round(cap_channel<0xff>(n->r()));
      g = round(cap_channel<0xff>(n->g()));
      b = round(cap_channel<0xff>(n->b()));
      a = cap_channel<1>   (n->a());
      if (output_style != NESTED || output_style != EXPANDED) name = "";
    } else {

    int numval = r * 0x10000 + g * 0x100 + b;
    if (ctx && ctx->colors_to_names.count(numval))
      res_name = ctx->colors_to_names[numval];
}

    stringstream hexlet;
    hexlet << '#' << setw(1) << setfill('0');
    // create a short color hexlet if not in nested output mode
    if (output_style != NESTED && output_style != EXPANDED&& is_color_doublet(r, g, b) && a == 1) {
      hexlet << hex << setw(1) << (static_cast<unsigned long>(r) >> 4);
      hexlet << hex << setw(1) << (static_cast<unsigned long>(g) >> 4);
      hexlet << hex << setw(1) << (static_cast<unsigned long>(b) >> 4);
    } else {
      hexlet << hex << setw(2) << static_cast<unsigned long>(r);
      hexlet << hex << setw(2) << static_cast<unsigned long>(g);
      hexlet << hex << setw(2) << static_cast<unsigned long>(b);
    }

    // retain the originally specified color definition if unchanged
    if (name != "") {
        ss << name;
    }
    else if (r == 0 && g == 0 && b == 0 && a == 0) {
        ss << "transparent";
    }
    else if (a >= 1) {
      if (output_style == COMPRESSED)
        if (hexlet.str().size() < res_name.size())
          res_name = "";
      if (res_name != "") {
        ss << res_name;
      }
      else {
        ss << hexlet.str();
      }
    }
    else {
      ss << "rgba(";
      ss << static_cast<unsigned long>(r) << ",";
      if (output_style == NESTED || output_style == EXPANDED) ss << " ";
      ss << static_cast<unsigned long>(g) << ",";
      if (output_style == NESTED || output_style == EXPANDED) ss << " ";
      ss << static_cast<unsigned long>(b) << ",";
      if (output_style == NESTED || output_style == EXPANDED) ss << " ";
      ss << a << ')';
    }
    append_to_buffer(ss.str());
  }

  void Inspect::operator()(Boolean* b)
  {
    append_to_buffer(b->value() ? "true" : "false");
  }

  void Inspect::operator()(String_Schema* ss)
  {
    // Evaluation should turn these into String_Constants, so this method is
    // only for inspection purposes.
    for (size_t i = 0, L = ss->length(); i < L; ++i) {
      if ((*ss)[i]->is_interpolant()) append_to_buffer("#{");
      (*ss)[i]->perform(this);
      if ((*ss)[i]->is_interpolant()) append_scope_closer();
    }
  }

  void Inspect::operator()(String_Constant* s)
  {
    if (String_Quoted* quoted = dynamic_cast<String_Quoted*>(s)) {
      return Inspect::operator()(quoted);
    }

    string value = s->value();
    // cerr << "OUTPUT String_Constant [" << value << "]" <<
            // endl << "from [" << s->value() << "]" << endl;
    append_to_buffer(value);

  }

  void Inspect::operator()(String_Quoted* s)
  {
    if (s->was_quoted()) {
      append_to_buffer(quote(s->value(), s->quote_mark()));
    } else {
      append_to_buffer((s->value()));
    }


  }

  void Inspect::operator()(Feature_Query* fq)
  {
    size_t i = 0;
    (*fq)[i++]->perform(this);
    for (size_t L = fq->length(); i < L; ++i) {
      (*fq)[i]->perform(this);
    }
  }

  void Inspect::operator()(Feature_Query_Condition* fqc)
  {
    if (fqc->operand() == Feature_Query_Condition::AND) {
      append_mandatory_space();
      append_to_buffer("and");
      append_mandatory_space();
    } else if (fqc->operand() == Feature_Query_Condition::OR) {
      append_mandatory_space();
      append_to_buffer("or");
      append_mandatory_space();
    } else if (fqc->operand() == Feature_Query_Condition::NOT) {
      append_mandatory_space();
      append_to_buffer("not");
      append_mandatory_space();
    }

    if (!fqc->is_root()) append_to_buffer("(");

    if (!fqc->length()) {
      fqc->feature()->perform(this);
      append_colon_separator();
      fqc->value()->perform(this);
    }
    // else
    for (size_t i = 0, L = fqc->length(); i < L; ++i)
      (*fqc)[i]->perform(this);

    if (!fqc->is_root()) append_to_buffer(")");
  }

  void Inspect::operator()(Media_Query* mq)
  {
    size_t i = 0;
    if (mq->media_type()) {
      if      (mq->is_negated())    append_to_buffer("not ");
      else if (mq->is_restricted()) append_to_buffer("only ");
      mq->media_type()->perform(this);
    }
    else {
      (*mq)[i++]->perform(this);
    }
    for (size_t L = mq->length(); i < L; ++i) {
      append_to_buffer(" and ");
      (*mq)[i]->perform(this);
    }
  }

  void Inspect::operator()(Media_Query_Expression* mqe)
  {
    if (mqe->is_interpolated()) {
      source_map.add_open_mapping(mqe->feature());
      mqe->feature()->perform(this);
      source_map.add_close_mapping(mqe->feature());
    }
    else {
      append_to_buffer("(");
      source_map.add_open_mapping(mqe->feature());
      mqe->feature()->perform(this);
      source_map.add_close_mapping(mqe->feature());
      if (mqe->value()) {
        append_colon_separator();
        // append_optional_space();
        source_map.add_open_mapping(mqe->value());
        mqe->value()->perform(this);
        source_map.add_close_mapping(mqe->value());
      }
      append_to_buffer(")");
    }
  }

  void Inspect::operator()(At_Root_Expression* ae)
  {
    if (ae->is_interpolated()) {
      ae->feature()->perform(this);
    }
    else {
      append_to_buffer("(");
      ae->feature()->perform(this);
      if (ae->value()) {
        append_colon_separator();
        ae->value()->perform(this);
      }
      append_to_buffer(")");
    }
  }

  void Inspect::operator()(Null* n)
  {
    append_to_buffer("null");
  }

  // parameters and arguments
  void Inspect::operator()(Parameter* p)
  {
    append_to_buffer(p->name());
    if (p->default_value()) {
      append_colon_separator();
      p->default_value()->perform(this);
    }
    else if (p->is_rest_parameter()) {
      append_to_buffer("...");
    }
  }

  void Inspect::operator()(Parameters* p)
  {
    append_to_buffer("(");
    // in_media = true;
    if (!p->empty()) {
      (*p)[0]->perform(this);
      for (size_t i = 1, L = p->length(); i < L; ++i) {
        append_comma_separator();
        (*p)[i]->perform(this);
      }
    }
    // in_media = false;
    append_to_buffer(")");
  }

  void Inspect::operator()(Argument* a)
  {
    in_argument = true;
    if (!a->name().empty()) {
      append_to_buffer(a->name());
      append_colon_separator();
    }
    // Special case: argument nulls can be ignored
    if (a->value()->concrete_type() == Expression::NULL_VAL) {
      return;
    }
    if (a->value()->concrete_type() == Expression::STRING) {
      String_Constant* s = static_cast<String_Constant*>(a->value());
      // interpolated argument needs auto detection
      // if (s->was_quoted()) s->value(quote(s->value(), '*', 41));
      s->perform(this);
    } else a->value()->perform(this);
    if (a->is_rest_argument()) {
      append_to_buffer("...");
    }
    in_argument = false;
  }

  void Inspect::operator()(Arguments* a)
  {
    append_to_buffer("(");
    if (!a->empty()) {
      // in_media = true;
      (*a)[0]->perform(this);
      // in_media = false;
      for (size_t i = 1, L = a->length(); i < L; ++i) {
        append_to_buffer(", ");
        (*a)[i]->perform(this);
      }
    }
    append_to_buffer(")");
  }

  // selectors
  void Inspect::operator()(Selector_Schema* s)
  {
    s->contents()->perform(this);
  }

  void Inspect::operator()(Selector_Reference* ref)
  {
    if (ref->selector()) ref->selector()->perform(this);
    else                 append_to_buffer("&");
  }

  void Inspect::operator()(Selector_Placeholder* s)
  {
    append_to_buffer(s->name(), s);
    if (s->has_line_break()) append_optional_linefeed();
    if (s->has_line_break()) append_indent_to_buffer();

  }

  void Inspect::operator()(Type_Selector* s)
  {
    // else append_optional_space();
    if (allow_before && in_raw_list) append_to_buffer2(s->pstate().token.ws_before());
    allow_before = false;
    append_to_buffer(s->name(), s);
    append_to_buffer2(s->pstate().token.ws_after());
    // if (s->has_line_break()) append_optional_linefeed();
    // if (s->has_line_break()) append_indent_to_buffer();
  }

  void Inspect::operator()(Selector_Qualifier* s)
  {
    append_to_buffer(s->name(), s);
    if (s->has_line_break()) append_optional_linefeed();
    if (s->has_line_break()) append_indent_to_buffer();
  }

  void Inspect::operator()(Attribute_Selector* s)
  {
    append_to_buffer("[");
    source_map.add_open_mapping(s);
    append_to_buffer(s->name());
    if (!s->matcher().empty()) {
      append_to_buffer(s->matcher());
      if (s->value()) {
        s->value()->perform(this);
      }
      // append_to_buffer(s->value());
    }
    source_map.add_close_mapping(s);
    append_to_buffer("]");
  }

  void Inspect::operator()(Pseudo_Selector* s)
  {
    append_to_buffer(s->name(), s);
    if (s->expression()) {
      s->expression()->perform(this);
      append_to_buffer(")");
    }
  }

  void Inspect::operator()(Wrapped_Selector* s)
  {
    append_to_buffer(s->name(), s);
    in_wrapped = true;
    s->selector()->perform(this);
    in_wrapped = false;
    append_to_buffer(")");
  }

  void Inspect::operator()(Compound_Selector* s)
  {
    // append_to_buffer(s->wspace);
    for (size_t i = 0, L = s->length(); i < L; ++i) {
      (*s)[i]->perform(this);
    }
    if (!in_raw_list) {
      if (s->has_line_break()) append_optional_linefeed();
      // if (s->has_line_break()) append_indent_to_buffer();
    }
  }

  void Inspect::operator()(Complex_Selector* c)
  {
    Compound_Selector*           head = c->head();
    Complex_Selector*            tail = c->tail();
    Complex_Selector::Combinator comb = c->combinator();
    if (head && !head->is_empty_reference()) head->perform(this);
    // if (head && !head->is_empty_reference() && head->linebreak && tail) append_optional_linefeed();
    if (head && !head->is_empty_reference() && tail) append_optional_space();

    switch (comb) {
      case Complex_Selector::ANCESTOR_OF: if(tail) append_mandatory_space();  break;
      case Complex_Selector::PARENT_OF:   append_to_buffer(">"); break;
      case Complex_Selector::PRECEDES:    append_to_buffer("~"); break;
      case Complex_Selector::ADJACENT_TO: append_to_buffer("+"); break;
    }

    if (tail) { //  && comb != Complex_Selector::ANCESTOR_OF) {
      // render line break after combinator
      if (c->has_line_break()) {
        // append_to_buffer("\n");
        append_optional_linefeed();
        // append_indent_to_buffer();
      } else
        append_optional_space();
    }
    if (tail) tail->perform(this);
  }

  void Inspect::operator()(Selector_List* g)
  {
    if (g->empty()) return;
        allow_before = false;

    for (size_t i = 0, L = g->length(); i < L; ++i) {

      if (i == 0 && !in_wrapped) append_indent_to_buffer();


      // if (i == 0) append_indent_to_buffer();
      if (g->wspace().size() > i)
      append_to_buffer2(g->wspace().at(i));

      source_map.add_open_mapping((*g)[i]);
      (*g)[i]->perform(this);
      source_map.add_close_mapping((*g)[i]);

      if (i < L - 1) {
        append_to_buffer(",");
        append_optional_space();
        allow_before = true;
        if ((*g)[i]->has_line_feed()) {
          append_optional_linefeed();
          append_indent_to_buffer();
        }
      }

    }
  }

  void Inspect::fallback_impl(AST_Node* n)
  {
  }

  string unquote(const string& s, char* qd)
  {

    // not enough room for quotes
    // no possibility to unquote
    if (s.length() < 2) return s;

    char q;
    bool skipped = false;

    // this is no guarantee that the unquoting will work
    // what about whitespace before/after the quote_mark?
    if      (*s.begin() == '"'  && *s.rbegin() == '"')  q = '"';
    else if (*s.begin() == '\'' && *s.rbegin() == '\'') q = '\'';
    else                                                return s;

    string unq;
    unq.reserve(s.length()-2);

    for (size_t i = 1, L = s.length() - 1; i < L; ++i) {

      // implement the same strange ruby sass behavior
      // an escape sequence can also mean a unicode char
      if (s[i] == '\\' && !skipped) {
        // remember
        skipped = true;

        // skip it
        // ++ i;

        // if (i == L) break;

        // escape length
        size_t len = 1;

        // parse as many sequence chars as possible
        // ToDo: Check if ruby aborts after possible max
        while (i + len < L && s[i + len] && isxdigit(s[i + len])) ++ len;

        // hex string?
        if (len > 1) {

          // convert the extracted hex string to code point value
          // ToDo: Maybe we could do this without creating a substring
          uint32_t cp = strtol(s.substr (i + 1, len - 1).c_str(), nullptr, 16);

          // assert invalid code points
          if (cp == 0) cp = 0xFFFD;
          // replace bell character
          // if (cp == 10) cp = 32;

          // use a very simple approach to convert via utf8 lib
          // maybe there is a more elegant way; maybe we shoud
          // convert the whole output from string to a stream!?
          // allocate memory for utf8 char and convert to utf8
          unsigned char u[5] = {0,0,0,0,0}; utf8::append(cp, u);
          for(size_t m = 0; u[m] && m < 5; m++) unq.push_back(u[m]);

          // skip some more chars?
          i += len - 1; skipped = false;

        }


      }
      // check for unexpected delimiter
      // be strict and throw error back
      else if (!skipped && q == s[i]) {
        // don't be that strict
        return s;
        // this basically always means an internal error and not users fault
        error("Unescaped delimiter in string to unquote found. [" + s + "]", ParserState("[UNQUOTE]", -1));
      }
      else {
        skipped = false;
        unq.push_back(s[i]);
      }

    }
    if (skipped) { return s; }
    if (qd) *qd = q;
    return unq;

  }

  // find best quote_mark by detecting if the string contains any single
  // or double quotes. When a single quote is found, we not we want a double
  // quote as quote_mark. Otherwise we check if the string cotains any double
  // quotes, which will trigger the use of single quotes as best quote_mark.
  static char detect_best_quotemark(const char* s, char qm = '"')
  {
    // ensure valid fallback quote_mark
    char quote_mark = qm && qm != '*' ? qm : '"';
    while (*s) {
      // force double quotes as soon
      // as one single quote is found
      if (*s == '\'') { return '"'; }
      // a single does not force quote_mark
      // maybe we see a double quote later
      else if (*s == '"') { quote_mark = '\''; }
      ++ s;
    }
    return quote_mark;
  }


  string quote(const string& s, char q)
  {

    if (s.empty()) return string(2, q == String_Constant::auto_quote() ?
      String_Constant::double_quote() : q);

    // autodetect with fallback to given quote
    q = detect_best_quotemark(s.c_str(), q);

    string quoted;
    quoted.reserve(s.length()+2);
    quoted.push_back(q);

    const char* it = s.c_str();
    const char* end = it + strlen(it) + 1;
    while (*it && it < end) {
      const char* now = it;

      if (*it == q) {
        quoted.push_back('\\');
      } else if (*it == '\\') {
        quoted.push_back('\\');
      }

      int cp = utf8::next(it, end);

      if (cp == 10) {
        quoted.push_back('\\');
        quoted.push_back('a');
      } else if (cp < 127) {
        quoted.push_back((char) cp);
      } else {
        while (now < it) {
          quoted.push_back(*now);
          ++ now;
        }
      }
    }

    quoted.push_back(q);
    return quoted;
  }

}

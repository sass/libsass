/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "inspect.hpp"

#include <iomanip>
#include "file.hpp"
#include "ast_css.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"
#include "ast_selectors.hpp"
#include "fn_utils.hpp"

#include "debugger.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  sass::string PrintNumber(double nr, const OutputOptions& outopt) {

    // Avoid streams
    char buf[255];
    snprintf(buf, 255,
      outopt.nr_sprintf,
      nr);

    // Operate from behind
    char* end = buf;

    // Move to last position
    while (*end != 0) ++end;
    if (end != buf) end--;
    // Delete trailing zeros
    while (*end == '0') {
      *end = 0;
      end--;
    }
    // Delete trailing decimal separator
    if (*end == '.') *end = 0;

    // Some final cosmetics
    if (buf[0] == '-' && buf[1] == '0' && buf[2] == 0) {
      buf[0] = '0'; buf[1] = 0;
    }

    // add unit now
    return sass::string(buf);

  }

  Inspect::Inspect(const OutputOptions& outopt)
    : Emitter(outopt), quotes(true), inspect(false)
  {
  }

  Inspect::Inspect(Logger& logger, const OutputOptions& outopt)
    : Emitter(outopt), quotes(true), inspect(false)
  {
  }

  void Inspect::acceptCssString(const CssString* node)
  {
    append_token(node->text(), node);
  }

  void Inspect::visitBlockStatements(CssNodeVector children)
  {
    append_scope_opener();
    for (CssNode* stmt : children) {
      stmt->accept(this);
    }
    append_scope_closer();
  }

  void Inspect::renderUnquotedString(const sass::string& text)
  {
    bool afterNewline = false;
    for (size_t i = 0; i < text.size(); i++) {
      uint8_t chr = text[i];
      switch (chr) {
      case $lf:
        append_char($space);
        afterNewline = true;
        break;

      case $space:
        if (!afterNewline) {
          append_char($space);
        }
        break;

      default:
        append_char(chr);
        afterNewline = false;
        break;
      }
    }
  }

  template <typename octet_iterator>
  bool Inspect::_tryPrivateUseCharacter(const octet_iterator& begin, const octet_iterator& end, size_t& offset) {
    octet_iterator it = begin + offset;
    if (output_style() == SASS_STYLE_COMPRESSED) return false;
    // check if char is utf8 character
    auto asd = utf8::internal::sequence_length(it);
    if (asd > 1) {
      uint32_t code_point;
      /*auto foo =*/ utf8::internal::validate_next(it, end, code_point);
      if (code_point >= 0xE000 && code_point <= 0xF8FF) {
        append_char($backslash);
        // ToDo: do without sstream
        sass::sstream is;
        is << std::hex << code_point;
        append_string(is.str());
        offset += asd;
        return true;
      }
    }
    return false;
  }

  void Inspect::renderQuotedString(const sass::string& text, uint8_t quotes)
  {

    // Scan the string first, dart-sass seems to do some fancy
    // trick by calling itself recursively when it encounters a
    // conflicting quote during the output, throwing away buffers.
    bool includesSingleQuote = text.find($apos) != sass::string::npos;
    bool includesDoubleQuote = text.find($quote) != sass::string::npos;

    // If both quotes are encountered
    if (quotes == $nul) {
      if (includesSingleQuote) quotes = $quote;
      else if (includesDoubleQuote) quotes = $apos;
      else quotes = $quote;
    }

    append_char(quotes);

    uint8_t chr, next;
    for (size_t i = 0, iL = text.size(); i < iL; i++) {
      chr = text[i];
      switch (chr) {
      case $apos:
        if (quotes == $apos) {
          append_char($backslash);
        }
        append_char($apos);
        break;
      case $quote:
        if (quotes == $quote) {
          append_char($backslash);
        }
        append_char($quote);
        break;
      // Write newline characters and unprintable ASCII characters as escapes.
      case $nul:
      case $soh:
      case $stx:
      case $etx:
      case $eot:
      case $enq:
      case $ack:
      case $bel:
      case $bs:
      case $lf:
      case $vt:
      case $ff:
      case $cr:
      case $so:
      case $si:
      case $dle:
      case $dc1:
      case $dc2:
      case $dc3:
      case $dc4:
      case $nak:
      case $syn:
      case $etb:
      case $can:
      case $em:
      case $sub:
      case $esc:
      case $fs:
      case $gs:
      case $rs:
      case $us:
      case $del:
        append_char($backslash);
        if (chr > 0xF) append_char(hexCharFor(chr >> 4));
        append_char(hexCharFor(chr & 0xF));
        if (iL == i + 1) break;
        next = text[i+1];
        if (isHex(next) || next == $space || next == $tab) {
          append_mandatory_space();
        }
        break;
      case $backslash:
        append_char($backslash);
        append_char($backslash);
        break;
      default:
        if (_tryPrivateUseCharacter(text.begin(), text.end(), i)) {
          
        }
        else {
          append_char(chr);
        }
        break;
      }
    }

    append_char(quotes);

  }
  // EO renderQuotedString


  void Inspect::visitCssMediaRule(CssMediaRule* node)
  {
    append_indentation();
    append_token("@media", node);
    append_mandatory_space();
    bool joinIt = false;
    for (auto query : node->queries()) {
      if (joinIt) {
        append_comma_separator();
        append_optional_space();
      }
      acceptCssMediaQuery(query);
      joinIt = true;
    }
    visitBlockStatements(node->elements());
  }
  // EO visitCssMediaRule

  void Inspect::visitCssStyleRule(CssStyleRule* node)
  {
    SelectorListObj s = node->selector();

    if (!s || s->empty()) return;
    if (!node || node->isInvisibleCss()) return;

    // if (output_style() == SASS_STYLE_NESTED) {
    //   indentation += node->tabs();
    // }

    if (outopt.source_comments) {
      sass::sstream ss;
      append_indentation();
      sass::string path(File::abs2rel(node->pstate().getAbsPath(), ".", CWD())); // ToDo: optimize
      ss << "/* line " << node->pstate().getLine() << ", " << path << " */";
      append_string(ss.str());
      append_optional_linefeed();
    }

    // scheduled_crutch = s;
    if (s) visitSelectorList(s);
    append_scope_opener(node);

    for (size_t i = 0, L = node->size(); i < L; ++i) {
      node->get(i)->accept(this); // XX
    }

    // if (output_style() == SASS_STYLE_NESTED) {
    //   indentation -= node->tabs();
    // }
    append_scope_closer(node);
  }
  // EO visitCssStyleRule

  void Inspect::visitCssSupportsRule(CssSupportsRule* rule)
  {
    if (rule == nullptr) return;
    if (rule->isInvisibleCss()) return;

    // if (output_style() == SASS_STYLE_NESTED) {
    //   indentation += rule->tabs();
    // }
    append_indentation();
    append_token("@supports", rule);
    append_mandatory_space();
//    if (const auto& cond = rule->condition()) {
//      if (const auto& str = cond->isaString()) {
//        const auto& text = str->value();
//        if (text.size() > 0 && text[0] == $lparen) {
//          // Do not add any space in this case
//        } else append_optional_space();
//      } if (cond->isaList()) {
//        // Do not add any space in this case
//      }
//      else append_optional_space();
//    } else
    rule->condition()->accept(this);
    append_scope_opener();

    size_t L = rule->size();
    for (size_t i = 0; i < L; ++i) {
      rule->get(i)->accept(this);
      if (i < L - 1) append_special_linefeed();
    }

    // if (output_style() == SASS_STYLE_NESTED) {
    //   indentation -= rule->tabs();
    // }

    append_scope_closer();
  }

  void Inspect::acceptCssMediaQuery(CssMediaQuery* query)
  {
    bool joinIt = false;
    if (!query->modifier().empty()) {
      append_string(query->modifier());
      append_mandatory_space();
    }
    if (!query->type().empty()) {
      append_string(query->type());
      joinIt = true;
    }
    bool isFirst = true;
    for (const sass::string& feature : query->features()) {
      if (joinIt) {
        append_mandatory_space();
        append_string(query->conjunction() ? "and" : "or");
        append_mandatory_space();
      }
      if (isFirst && StringUtils::startsWith(feature, "(not ")) {
        append_string(feature.substr(1, feature.size() - 2));
      }
      else {
        append_string(feature);
      }
      joinIt = true;
      isFirst = false;
    }
  }

  void Inspect::visitCssComment(CssComment* c)
  {
    bool important = c->isPreserved();
    if (output_style() == SASS_STYLE_COMPRESSED || output_style() == SASS_STYLE_COMPACT) {
      if (!important) return;
    }
    if (output_style() != SASS_STYLE_COMPRESSED || important) {
      append_indentation();
      append_string(c->text());
      if (indentation == 0) {
        append_mandatory_linefeed();
      }
      else {
        append_optional_linefeed();
      }
    }
  }
  // EO visitCssComment

  void Inspect::visitCssDeclaration(CssDeclaration* node)
  {
    RAII_FLAG(in_declaration, true);
    RAII_FLAG(in_custom_property,
      node->is_custom_property());
    // if (output_style() == SASS_STYLE_NESTED)
    //   indentation += node->tabs();
    append_indentation();
    if (node->name()) {
      force_next_mapping = true;
      acceptCssString(node->name());
      force_next_mapping = false;
     }
    append_colon_separator();
    if (node->value()) {
      force_next_mapping = true;
      node->value()->accept(this);
      force_next_mapping = false;
    }
    append_delimiter();
    // if (output_style() == SASS_STYLE_NESTED)
    //   indentation -= node->tabs();
  }
  // EO visitCssDeclaration

  bool Inspect::_IsInvisible(CssNode* node) {
    if (inspect) return false;
    if (output_style() == SASS_STYLE_COMPRESSED) {
      return node->isInvisibleHidingComments();
    }
    else {
      return node->isInvisible();
    }
  }

  // statements // visitCssStylesheet
  void Inspect::visitCssRoot(CssRoot* block)
  {

    for (size_t i = 0, L = block->size(); i < L; ++i) {
      auto& child = block->get(i);
      if (_IsInvisible(block)) continue;
      child->accept(this); // XX
    }

  }

  void Inspect::visitCssKeyframeBlock(CssKeyframeBlock* node)
  {
    if (node->selector()) {

      const sass::vector<sass::string>& selector
        = node->selector()->texts();

      if (!selector.empty()) {
        append_indentation();
        bool addComma = false;
        for (sass::string sel : selector) {
          if (addComma) {
            append_comma_separator();
          }
          append_string(sel);
          addComma = true;
        }
      }

    }
    // StringLiteralObj v2 = node->name2();
    // 
    // if (!v2.isNull()) {
    //   append_indentation();
    //   v2->accept(this);
    // }
    // 
    // append_scope_opener();
    // for (size_t i = 0, L = r->size(); i < L; ++i) {
    //   Statement_Obj stm = r->get(i);
    //   stm->accept(this);
    //   if (i < L - 1) append_special_linefeed();
    // }
    // append_scope_closer();
    if (!node->isInvisibleCss()) {
      append_scope_opener();
      for (CssNode* child : node->elements()) {
        child->accept(this);
      }
      append_scope_closer();
    }
 }

  void Inspect::visitCssAtRule(CssAtRule* node)
  {
    append_indentation();
    if (node->name()) {
      append_char($at);
      acceptCssString(node->name());
    }
    if (node->value()) {
      append_mandatory_space();
      acceptCssString(node->value());
    }
    if (node->isChildless()) {
      append_delimiter();
    }
    else {
      visitBlockStatements(node->elements());
    }
  }

  void Inspect::visitCssImport(CssImport* import)
  {
    append_indentation();
    add_open_mapping(import, true);
    append_string("@import");
    append_mandatory_space();
    CssString* url(import->url());
    append_token(url->text(), url);
    if (import->modifiers()) {
      append_mandatory_space();
      CssString* text(import->modifiers());
      append_token(text->text(), text);
    }
    // if (!import->media().empty()) {
    //   bool first = true;
    //   append_mandatory_space();
    //   for (CssMediaQueryObj query : import->media()) {
    //     if (first == false) {
    //       append_comma_separator();
    //       append_optional_space();
    //     }
    //     acceptCssMediaQuery(query);
    //     first = false;
    //   }
    // }
    add_close_mapping(import, true);
    append_delimiter();
  }

  void Inspect::_writeMapElement(Interpolant* itpl)
  {
    if (Value * value = itpl->isaValue()) {
      bool needsParens = false;
      if (List * list = value->isaList()) {
        needsParens = list->separator() == SASS_COMMA;
        if (list->hasBrackets()) needsParens = false;
      }
      if (needsParens) {
        append_char($lparen);
        value->accept(this);
        append_char($rparen);
      }
      else {
        value->accept(this);
      }
    }
    else if (ItplString* str = itpl->isaItplString()) {
      append_token(str->text(), str);
    }
    else {
      throw std::runtime_error("Expression not evaluated");
    }
  }

  void Inspect::acceptNameSpaceSelector(SelectorNS* selector)
  {
    flush_schedules();
    if (selector->hasNs()) {
      write_string(selector->ns());
      write_char($pipe);
    }
    write_string(selector->name());
  }

  void Inspect::visitAttributeSelector(AttributeSelector* attribute)
  {
    append_string("[");
    acceptNameSpaceSelector(attribute);
    if (!attribute->op().empty()) {
      append_string(attribute->op());
      if (attribute->isIdentifier() && !StringUtils::startsWith(attribute->value(), "--", 2)) {
        append_string(attribute->value());
        if (attribute->modifier() != 0) {
          append_optional_space();
        }
      }
      else {
        renderQuotedString(attribute->value());
        if (attribute->modifier() != 0) {
          append_optional_space();
        }
      }
    }
    // add_close_mapping(attribute);
    if (attribute->modifier() != 0) {
      append_mandatory_space();
      append_char(attribute->modifier());
    }
    append_string("]");
  }

  void Inspect::visitClassSelector(ClassSelector* klass)
  {
    // Skip over '.' character
    move_next_mapping(1, 1);
    add_open_mapping(klass, true);
    append_string(klass->name());
    add_close_mapping(klass, true);
  }

  void Inspect::visitComplexSelector(ComplexSelector* complex)
  {
    bool many = false;

    // debug_ast(complex, "visit: ");

    // schedule_mapping(complex->last());

    for (SelectorCombinator* combinator : complex->leadingCombinators()) {
      visitSelectorCombinator(combinator);
      append_mandatory_space();
    }

    for (const CplxSelComponentObj& item : complex->elements()) {
      if (many) append_mandatory_space();
      visitSelectorComponent(item);
      many = true;
    }

    schedule_mapping(nullptr);

  }

  void Inspect::visitSelectorComponent(CplxSelComponent* comp)
  {
    if (CompoundSelector* compound = comp->selector()) {
      visitCompoundSelector(compound);
    }
    for (SelectorCombinator* combinator : comp->combinators()) {
      visitSelectorCombinator(combinator);
    }
  }

  void Inspect::visitCompoundSelector(CompoundSelector* compound)
  {

    size_t position = wbuf.buffer.size();

    if (compound->withExplicitParent()) {
      if (inspect == true) {
        append_string("&");
      }
    }

    for (const SimpleSelectorObj& item : compound->elements()) {
      item->accept(this);
    }

    // If we emit an empty compound, it's because all of the
    // components got optimized out because they match all
    // selectors, so we just emit the universal selector.
    if (position == wbuf.buffer.size()) {
      write_char($asterisk);
    }

    // Add the post line break (from ruby sass)
    // Dart sass uses another logic for newlines
    if (compound->hasPostLineBreak()) {
      if (output_style() != SASS_STYLE_COMPACT) {
        append_optional_linefeed();
      }
    }
  }

  void Inspect::visitSelectorCombinator(SelectorCombinator* combinator)
  {
    append_optional_space();
     switch (combinator->combinator()) {
       case SelectorPrefix::CHILD: append_string(">"); break;
       case SelectorPrefix::SIBLING: append_string("+"); break;
       case SelectorPrefix::FOLLOWING: append_string("~"); break;
     }
     append_optional_space();
    //  // Add the post line break (from ruby sass)
  //  // Dart sass uses another logic for newlines
  //  // if (combinator->hasPostLineBreak()) {
  //  //   if (output_style() != COMPACT) {
  //  //     // append_optional_linefeed();
  //  //   }
  //  // }
  }

  void Inspect::visitIDSelector(IDSelector* id)
  {
    // Skip over '#' character
    move_next_mapping(1, 1);
    append_token(id->name(), id);
  }

  void Inspect::visitPlaceholderSelector(PlaceholderSelector* placeholder)
  {
    append_token(placeholder->name(), placeholder);
  }

  void Inspect::visitPseudoSelector(PseudoSelector* pseudo)
  {

    if (auto sel = pseudo->selector()) {
      if (pseudo->name() == "not") {
        if (sel->empty()) {
          return;
        }
      }
    }

    if (pseudo->name() != "") {
      append_string(":");
      if (pseudo->isSyntacticElement()) {
        append_string(":");
      }
      append_token(pseudo->name(), pseudo);
      // this whole logic can be done simpler!? copy object?
      if (pseudo->selector() || !pseudo->argument().empty()) {
        append_string("(");
        parentheses_opened = true;
        append_string(pseudo->argument());
        if (pseudo->selector() && !pseudo->argument().empty()) {
          if (!pseudo->selector()->empty() &&
              !pseudo->argument().empty()) {
            append_mandatory_space();
          }
        }
        bool was_comma_array = in_comma_array;
        in_comma_array = false;
        if (pseudo->selector()) {
          visitSelectorList(pseudo->selector());
        }
        in_comma_array = was_comma_array;
        append_string(")");
      }
    }
  }

  void Inspect::visitSelectorList(SelectorList* list)
  {
    if (list->empty()) {
      return;
    }

    bool was_comma_array = in_comma_array;
    // probably ruby sass equivalent of element_needs_parens
    if (!in_declaration && in_comma_array) {
      append_string("(");
      parentheses_opened = true;
    }

    if (in_declaration) in_comma_array = true;
    bool first = true;
    for (size_t i = 0, L = list->size(); i < L; ++i) {


      if (list->get(i) == nullptr) continue;
      if (!inspect && list->get(i)->isInvisible()) continue;
      if (first) append_indentation();
      else {
        scheduled_space = 0;
        append_comma_separator();
      }
      first = false;
      if (list->get(i)->hasPreLineFeed()) {
        append_optional_linefeed();
        if (output_style() != SASS_STYLE_COMPACT)
          append_indentation();
      }
      visitComplexSelector(list->get(i));
    }

    in_comma_array = was_comma_array;
    // probably ruby sass equivalent of element_needs_parens
    if (!in_declaration && in_comma_array) {
      append_string(")");
    }
  }

  void Inspect::visitTypeSelector(TypeSelector* type)
  {
    add_open_mapping(type, true);
    acceptNameSpaceSelector(type);
    add_close_mapping(type, true);
  }

  // Returns whether [value] needs parentheses as an
  // element in a list with the given [separator].
  bool _elementNeedsParens(SassSeparator separator, const Value* value) {
    if (const List * list = value->isaList()) {
      if (list->size() < 2) return false;
      if (list->hasBrackets()) return false;
      switch (separator) {
      case SASS_COMMA:
        return list->separator() == SASS_COMMA;
      case SASS_DIV:
        return list->separator() == SASS_COMMA ||
          list->separator() == SASS_DIV;
      default:
        return list->separator() != SASS_UNDEF;
      }
    }
    return false;
  }

  sass::string _separatorString(SassSeparator separator, bool compressed) {
    switch (separator) {
    case SASS_SPACE:
      return " ";
    case SASS_COMMA:
      return compressed ? "," : ", ";
    case SASS_DIV:
      return compressed ? "/" : " / ";
    default:
      return "";
    }
  }

  void Inspect::visitList(List* list)
  {
    // Handle empty case
    if (list->empty()) {
      if (list->hasBrackets()) {
        append_char($lbracket);
        append_char($rbracket);
      }
      else {
        append_char($lparen);
        append_char($rparen);
      }
      return;
    }

    bool preserveComma = inspect &&
      list->size() == 1 &&
      (list->separator() == SASS_COMMA ||
        list->separator() == SASS_DIV);

    if (list->hasBrackets()) {
      append_char($lbracket);
    }
    else if (preserveComma) {
      append_char($lparen);
    }

    const sass::vector<ValueObj>& values(list->elements());

    bool first = true;
    sass::string joiner = _separatorString(list->separator(),
      output_style() == SASS_STYLE_COMPRESSED);

    for (Value* value : values) {
      // Only print `null` when inspecting
      if (!inspect && value->isBlank()) continue;
      if (first == false) {
        append_string(joiner);
      }
      else {
        first = false;
      }
      if (inspect) {
        bool needsParens = _elementNeedsParens(
          list->separator(), value);
        if (needsParens) {
          append_char($lparen);
        }
        value->accept(this);
        if (needsParens) {
          append_char($rparen);
        }
      }
      else {
        value->accept(this);
      }
    }

    if (preserveComma) {
      if (list->separator() == SASS_DIV) {
        append_char($slash);
      }
      else {
        append_char($comma);
      }
      if (!list->hasBrackets()) {
        append_char($rparen);
      }
    }

    if (list->hasBrackets()) {
      append_char($rbracket);
    }
  }

  void Inspect::acceptInterpolation(Interpolation* node)
  {
    // throw std::runtime_error("Interpolation");
    for (Interpolant* itpl : node->elements()) {
      if (ItplString* str = itpl->isaItplString()) {
        append_token(str->text(), str);
      }
      else if (Value* value = itpl->isaValue()) {
        value->accept(this);
      }
      else {
        throw std::runtime_error("Expression not evaluated");
      }
    }
  }

  void Inspect::visitFunction(Function* value)
  {
    append_token("get-function", value);
    append_string("(");
    parentheses_opened = true;
    Callable* fn = value->callable();
    // Function names are safe to quote!
    append_token("\""+ fn->name() + "\"", fn);
    append_string(")");
  }


  // T visitBoolean(Boolean value);
  void Inspect::visitBoolean(Boolean* value)
  {
    // output the final token
    append_token(value->value() ? "true" : "false", value);
  }

  bool is_hex_doublet(double n)
  {
    return n == 0x00 || n == 0x11 || n == 0x22 || n == 0x33 ||
      n == 0x44 || n == 0x55 || n == 0x66 || n == 0x77 ||
      n == 0x88 || n == 0x99 || n == 0xAA || n == 0xBB ||
      n == 0xCC || n == 0xDD || n == 0xEE || n == 0xFF;
  }

  bool is_color_doublet(double r, double g, double b)
  {
    return is_hex_doublet(r) && is_hex_doublet(g) && is_hex_doublet(b);
  }

  // T visitColorRGBA(SassColor value);
  void Inspect::visitColor(Color* color)
  {
    // output the final token
    sass::sstream ss;

    if (color->parsed() && !color->isaColorHwba()) { //&& color->a() < 1

      if (color->disp().empty()) {
        double epsilon = std::pow(0.1, outopt.precision);
        if (ColorHsla* hsla = color->isaColorHsla()) {
          if (hsla->a() >= 1) {
            ss << "hsl(";
            ss << PrintNumber(hsla->h(), outopt) << ", ";
            ss << PrintNumber(hsla->s(), outopt) << "%, ";
            ss << PrintNumber(hsla->l(), outopt) << "%)";
          }
          else {
            ss << "hsla(";
            ss << PrintNumber(hsla->h(), outopt) << ", ";
            ss << PrintNumber(hsla->s(), outopt) << "%, ";
            ss << PrintNumber(hsla->l(), outopt) << "%, ";
            ss << PrintNumber(clamp<double>(hsla->a(), 0, 1), outopt) << ")";
          }
        }
        else if (ColorRgba* rgba = color->isaColorRgba()) {
          if (rgba->a() >= 1) {
            ss << "rgb(";
            ss << PrintNumber(rgba->r(), outopt) << ", ";
            ss << PrintNumber(rgba->g(), outopt) << ", ";
            ss << PrintNumber(rgba->b(), outopt) << ")";
          }
          else {
            ss << "rgba(";
            ss << PrintNumber(rgba->r(), outopt) << ", ";
            ss << PrintNumber(rgba->g(), outopt) << ", ";
            ss << PrintNumber(rgba->b(), outopt) << ", ";
            ss << PrintNumber(clamp<double>(rgba->a(), 0, 1), outopt) << ")";
          }
        }
        //else if (ColorHwba* hwba = color->isaColorHwba()) {
        //  auto rgba = hwba->toRGBA();
        //  if (rgba->a() >= 1) {
        //    ss << "hwb(";
        //    ss << round64(rgba->r(), epsilon) << ", ";
        //    ss << round64(rgba->g(), epsilon) << ", ";
        //    ss << round64(rgba->b(), epsilon) << ")";
        //  }
        //  else {
        //    ss << "hwba(";
        //    ss << round64(rgba->r(), epsilon) << ", ";
        //    ss << round64(rgba->g(), epsilon) << ", ";
        //    ss << round64(rgba->b(), epsilon) << ", ";
        //    ss << clamp<double>(rgba->a(), 0, 1) << ")";
        //  }
        //}
        append_token(ss.str(), color);
        // append_token(color->toString(), color);
      }
      else {
        append_token(color->disp(), color);
      }
      return;

    }

    ColorRgbaObj c = color->toRGBA();

    // original color name
    // maybe an unknown token
    sass::string name = c->disp();

    // resolved color
    sass::string res_name = name;

    double epsilon = std::pow(0.1, outopt.precision);
    double r = round64(clamp(c->r(), 0.0, 255.0), epsilon);
    double g = round64(clamp(c->g(), 0.0, 255.0), epsilon);
    double b = round64(clamp(c->b(), 0.0, 255.0), epsilon);
    double a = clamp<double>(c->a(), 0, 1);

    // get color from given name (if one was given at all)
    if (name != "" && name_to_color(name)) {
      const ColorRgba* n = name_to_color(name);
      r = round64(clamp(n->r(), 0.0, 255.0), epsilon);
      g = round64(clamp(n->g(), 0.0, 255.0), epsilon);
      b = round64(clamp(n->b(), 0.0, 255.0), epsilon);
      a = clamp(n->a(), 0.0, 1.0);
    }
    // otherwise get the possible resolved color name
    else {
      double numval = r * 0x10000 + g * 0x100 + b;
      if (color_to_name((int)numval))
        res_name = color_to_name((int)numval);
    }

    sass::sstream hexlet;
    // dart sass compressed all colors in regular css always
    // ruby sass and libsass does it only when not delayed
    // since color math is going to be removed, this can go too
    bool compressed = outopt.output_style == SASS_STYLE_COMPRESSED;
    hexlet << '#' << std::setw(1) << std::setfill('0');
    // create a short color hexlet if there is any need for it
    if (compressed && is_color_doublet(r, g, b) && a >= 1.0) {
      hexlet << std::hex << std::setw(1) << (static_cast<unsigned long>(r) >> 4);
      hexlet << std::hex << std::setw(1) << (static_cast<unsigned long>(g) >> 4);
      hexlet << std::hex << std::setw(1) << (static_cast<unsigned long>(b) >> 4);
      if (a != 1) hexlet << std::hex << std::setw(1) << (static_cast<unsigned long>(a * 255) >> 4);
    }
    else {
      hexlet << std::hex << std::setw(2) << static_cast<unsigned long>(r);
      hexlet << std::hex << std::setw(2) << static_cast<unsigned long>(g);
      hexlet << std::hex << std::setw(2) << static_cast<unsigned long>(b);
      if (a != 1) hexlet << std::hex << std::setw(2) << (static_cast<unsigned long>(a * 255) >> 4);
    }

    if (compressed) name = "";

    // retain the originally specified color definition if unchanged
    if (name != "") {
      ss << name;
    }
    else if (a >= 1.0) {
      if (res_name != "") {
        if (compressed && hexlet.str().size() < res_name.size()) {
          ss << hexlet.str();
        }
        else {
          ss << res_name;
        }
      }
      else {
        ss << hexlet.str();
      }
    }

    else {
      ss << "rgba(";
      ss << PrintNumber(r, outopt) << ",";
      if (!compressed) ss << " ";
      ss << PrintNumber(g, outopt) << ",";
      if (!compressed) ss << " ";
      ss << PrintNumber(b, outopt) << ",";
      if (!compressed) ss << " ";
      ss << PrintNumber(clamp<double>(a, 0, 1), outopt) << ')';
    }

    append_token(ss.str(), c);
  }
  // EO visitColorRGBA

  // T visitFunction(Function value);

  // T visitMap(Map value);
  void Inspect::visitMap(Map* value)
  {
    if (value->empty()) {
      append_string("()");
      return;
    }
    bool items_output = false;
    append_string("(");
    parentheses_opened = true;
    #if SassSortMapKeysOnOutput
    auto keys = value->keys();
    std::sort(keys.begin(), keys.end(),
      [](const ValueObj& a, const ValueObj& b) {
        return a->toString() < b->toString();
      });
    for (auto key : keys) {
      const auto& kv = *value->find(key);
    #else
    for (const auto& kv : value->elements()) {
    #endif
      if (items_output) append_comma_separator();
      _writeMapElement(kv.first);
      append_colon_separator();
      _writeMapElement(kv.second);
      items_output = true;
    }
    append_string(")");
  }

  void Inspect::visitNull(Null* value)
  {
    if (output_style() == SASS_STYLE_TO_CSS) return;
    // output the final token
    append_token("null", value);
  }

  void Inspect::visitCalculation(Calculation* value)
  {
    // if (output_style() == SASS_STYLE_TO_CSS) return;
    append_string(value->name());
    append_string("(");
    parentheses_opened = true;
    bool first = true;
    for (auto node : value->arguments()) {
      if (node == nullptr) continue;
      if (!first) {
        append_comma_separator();
        append_optional_space();
      }
      _writeCalculationValue(node, false);
      first = false;
    }
    append_string(")");
  }

  void Inspect::_writeCalculationUnits(Units* units)
  {
    append_string(units->unit());
  }

  static int OpPrecedence(SassOperator op) {
    switch (op) {
      case ADD: return 1;
      case SUB: return 1;
      case MUL: return 2;
      case DIV: return 2;
      default: return 0;
    }
  }

  static bool _parenthesizeCalculationRhs(
    SassOperator outer, SassOperator right)
  {
    if (outer == DIV) return true;
    if (outer == ADD) return false;
    return right == ADD || right == SUB;

  }

  static bool _parenthesizeCalculationRhsDiv(AstNode* rhs)
  {
    if (auto nr = dynamic_cast<Number*>(rhs)) {
      return std::isfinite(nr->value())
        ? nr->isValidCssUnit() == false
        : nr->hasUnits();
    }
    return false;
  }

  void Inspect::_writeCalculationValue(AstNode* node, bool wrap)
  {
    if (auto nr = dynamic_cast<Number*>(node)) {

      if (output_style() == SASS_STYLE_TO_CSS) {
        if (!nr->isValidCssUnit()) {
          throw Exception::SassScriptException(
            "isn't a valid CSS value.",
            {}, node->pstate()
          );
        }
      }
      // ToDo: implement cssize
      //if (nr->isValidCssUnit() == false) {
      //  throw std::runtime_error(
      //    "isn't a valid CSS value");
      //}

      if (std::isnan(nr->value())) {
        // if (wrap) append_string("calc(");
        if (nr->value() < 0) {
          append_string("-NaN");
        }
        else {
          append_string("NaN");
        }
        if (nr->hasUnits()) {
          append_string(" * 1");
          _writeCalculationUnits(nr);
        }
        // if (wrap) append_string(")");
        //_writeCalculationUnits(nr);
        return;
      }

      if (std::isinf(nr->value())) {
        // if (wrap) append_string("calc(");
        if (nr->value() < 0) {
          append_string("-infinity");
        }
        else {
          append_string("infinity");
        }
        if (nr->hasUnits()) {
          append_string(" * 1");
          _writeCalculationUnits(nr);
        }
        // if (wrap) append_string(")");
        //_writeCalculationUnits(nr);
        return;
      }

      if (nr->isValidCssUnit()) {
        nr->accept(this);
      }
      else {
        nr->accept(this);
      }

    }
    else if (auto op = dynamic_cast<CalcOperation*>(node)) {
      auto lcalc = dynamic_cast<CalcOperation*>(op->left().ptr());
      auto rcalc = dynamic_cast<CalcOperation*>(op->right().ptr());
      bool pl = lcalc != nullptr && OpPrecedence(lcalc->op()) < OpPrecedence(op->op());

      if (pl) append_string("(");
      parentheses_opened = pl;
      _writeCalculationValue(op->left());
      if (pl) append_string(")");

      // ToDo: compress white-space
      append_optional_space();
      switch (op->op()) {
        case ADD: append_string("+"); break;
        case SUB: append_string("-"); break;
        case MUL: append_string("*"); break;
        case DIV: append_string("/"); break;
        default: break;
      }
      append_optional_space();

      bool pr = rcalc != nullptr && _parenthesizeCalculationRhs(op->op(), rcalc->op());
      pr |= op->op() == DIV && _parenthesizeCalculationRhsDiv(op->right());

      if (pr) append_string("(");
      parentheses_opened = pl;
      _writeCalculationValue(op->right());
      if (pr) append_string(")");

    }
    else if (auto val = dynamic_cast<Value*>(node)) {
      val->accept(this);
    }
    else if (auto val = dynamic_cast<CalcOperation*>(node)) {
      val->accept(this);
    }
    else {
      append_string("[NOSUP]");
    }

  }

  void Inspect::visitMixin(Mixin* mixin)
  {
    if (output_style() == SASS_STYLE_TO_CSS) {
      throw Exception::InvalidCssValue({}, *mixin);
    }
    add_open_mapping(mixin, false);
    append_string("get-mixin(\"");
    if (mixin->callable() != nullptr)
      append_string(mixin->callable()->name());
    append_string("\")");
    add_close_mapping(mixin, false);
  }

  void Inspect::visitCalcOperation(CalcOperation* op)
  {
    //if (output_style() == SASS_STYLE_TO_CSS) return;
    append_token(op->left()->toString(), op->left());
    if (op->op() == ADD) append_string(" + ");
    else if (op->op() == SUB) append_string(" - ");
    else if (op->op() == MUL) append_string(" * ");
    else if (op->op() == DIV) append_string(" / ");
    append_token(op->right()->toString(), op->right());
  }

  void Inspect::visitNumber(Number* value)
  {
    if (value->lhsAsSlash() && value->rhsAsSlash()) {
      visitNumber(value->lhsAsSlash());
      append_string("/");
      visitNumber(value->rhsAsSlash());
      return;
    }

    if (std::isnan(value->value())) {
      append_string("calc(");
      if (value->value() < 0) {
        append_string("-NaN");
      }
      else {
        append_string("NaN");
      }
      if (!value->isUnitless()) {
        if (value->numerators.size() > 0)
          append_string(" * 1");
        append_string(value->unit2());
      }
      append_string(")");
      return;
    }

    if (std::isinf(value->value())) {
      append_string("calc(");
      if (value->value() < 0) {
        append_string("-infinity");
      }
      else {
        append_string("infinity");
      }
      if (!value->isUnitless()) {
        if (value->numerators.size() > 0)
          append_string(" * 1");
        append_string(value->unit2());
      }
      append_string(")");
      return;
    }

    // Avoid streams
    char buf[255];
    snprintf(buf, 255,
      outopt.nr_sprintf,
      value->value());

    // Operate from behind
    char* end = buf;

    // Move to last position
    while (*end != 0) ++end;
    if (end != buf) end--;
    // Delete trailing zeros
    while (*end == '0') {
      *end = 0;
      end--;
    }
    // Delete trailing decimal separator
    if (*end == '.') *end = 0;

    // Some final cosmetics
    if (buf[0] == '-' && buf[1] == '0' && buf[2] == 0) {
      buf[0] = '0'; buf[1] = 0;
    }

    // add unit now
    sass::string res(buf);

    if (true)
    {
      size_t iL = value->numerators.size();
      size_t nL = value->denominators.size();
      if (iL != 0) res += value->numerators[0];
      for (size_t i = 1; i < iL; i += 1) {
        res += " * 1";
        res += value->numerators[i];
      }
      for (size_t i = 0; i < nL; i += 1) {
        res += " / 1";
        res += value->denominators[i];
      }
    }
    else {
      res += value->unit();
    }

    if (value->isValidCssUnit()) {
      // output the final token
      append_token(res, value);
    }
    else {
      append_string("calc(");
      append_token(res, value);
      append_string(")");
    }
  }

  void Inspect::visitString(String* value)
  {
    add_open_mapping(value, true);
    if (quotes && value->hasQuotes()) {
      renderQuotedString(value->value());
    }
    else {
      renderUnquotedString(value->value());
    }
    add_close_mapping(value, true);
  }




















}

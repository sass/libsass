#include "inspect.hpp"

#include <iomanip>
#include "file.hpp"
#include "ast_css.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"
#include "ast_selectors.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  Inspect::Inspect(SassOutputOptionsCpp& opt, bool srcmap_enabled)
    : Emitter(opt, srcmap_enabled), quotes(true), inspect(false)
  {
  }

  Inspect::Inspect(Logger& logger, SassOutputOptionsCpp& opt, bool srcmap_enabled)
    : Emitter(opt, srcmap_enabled), quotes(true), inspect(false)
  {
  }

  void Inspect::acceptCssString(CssString* node)
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

  void Inspect::renderQuotedString(const sass::string& text, uint8_t quotes)
  {

    // Scan the string first, dart-sass seems to do some fancy
    // trick by calling itself recursively when it encounters a
    // conflicting quote during the output, throwing away buffers.
    bool includesSingleQuote = text.find($single_quote) != sass::string::npos;
    bool includesDoubleQuote = text.find($double_quote) != sass::string::npos;

    // If both quotes are encountered
    if (quotes == $nul) {
      if (includesSingleQuote) quotes = $double_quote;
      else if (includesDoubleQuote) quotes = $single_quote;
      else quotes = $double_quote;
    }

    append_char(quotes);

    uint8_t chr, next;
    for (size_t i = 0, iL = text.size(); i < iL; i++) {
      chr = text[i];
      switch (chr) {
      case $single_quote:
        if (quotes == $single_quote) {
          append_char($backslash);
        }
        append_char($single_quote);
        break;
      case $double_quote:
        if (quotes == $double_quote) {
          append_char($backslash);
        }
        append_char($double_quote);
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
        append_char($backslash);
        if (chr > 0xF) append_char(hexCharFor(chr >> 4));
        append_char(hexCharFor(chr & 0xF));
        if (iL == i + 1) break;
        next = text[i+1];
        if (isHex(next) || next == $space || next == $tab) {
          append_char($space);
        }
        break;
      case $backslash:
        append_char($backslash);
        append_char($backslash);
        break;
      default:
        append_char(chr);
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

    if (opt.source_comments) {
      sass::sstream ss;
      append_indentation();
      sass::string path(File::abs2rel(node->pstate().getAbsPath(), ".", CWD)); // ToDo: optimize
      ss << "/* line " << node->pstate().getLine() << ", " << path << " */";
      append_string(ss.str());
      append_optional_linefeed();
    }

    scheduled_crutch = s;
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
    for (auto feature : query->features()) {
      if (joinIt) {
        append_mandatory_space();
        append_string("and");
        append_mandatory_space();
      }
      append_string(feature);
      joinIt = true;
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
    LOCAL_FLAG(in_declaration, true);
    LOCAL_FLAG(in_custom_property,
      node->is_custom_property());
    // if (output_style() == SASS_STYLE_NESTED)
    //   indentation += node->tabs();
    append_indentation();
    if (node->name()) {
      acceptCssString(node->name());
    }
    append_colon_separator();
    if (node->value()) {
      node->value()->accept(this);
    }
    append_delimiter();
    // if (output_style() == SASS_STYLE_NESTED)
    //   indentation -= node->tabs();
  }
  // EO visitCssDeclaration

  // statements
  void Inspect::visitCssRoot(CssRoot* block)
  {
    for (size_t i = 0, L = block->size(); i < L; ++i) {
      (*block)[i]->accept(this); // XX
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
    add_open_mapping(import);
    append_string("@import");
    append_mandatory_space();
    CssString* url(import->url());
    append_token(url->text(), url);
    if (import->supports()) {
      append_mandatory_space();
      CssString* text(import->supports());
      append_token(text->text(), text);
    }
    if (!import->media().empty()) {
      bool first = true;
      append_mandatory_space();
      for (CssMediaQueryObj query : import->media()) {
        if (first == false) {
          append_char($comma);
          append_optional_space();
        }
        acceptCssMediaQuery(query);
        first = false;
      }
    }
    add_close_mapping(import);
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

  void Inspect::acceptNameSpaceSelector(NameSpaceSelector* selector)
  {
    flush_schedules();
    add_open_mapping(selector);
    if (selector->hasNs()) {
      write_string(selector->ns());
      write_char($pipe);
    }
    write_string(selector->name());
    add_close_mapping(selector);
  }

  void Inspect::visitAttributeSelector(AttributeSelector* attribute)
  {
    append_string("[");
    add_open_mapping(attribute);
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
    add_close_mapping(attribute);
    if (attribute->modifier() != 0) {
      append_mandatory_space();
      append_char(attribute->modifier());
    }
    append_string("]");
  }

  void Inspect::visitClassSelector(ClassSelector* klass)
  {
    flush_schedules();
    add_open_mapping(klass);
    // hotfix for browser issues
    // this is pretty ugly indeed
    if (scheduled_crutch) {
      add_open_mapping(scheduled_crutch);
      scheduled_crutch = 0;
    }
    append_string(klass->name());
    add_close_mapping(klass);
  }

  void Inspect::visitComplexSelector(ComplexSelector* complex)
  {
    bool many = false;
    if (complex->hasPreLineFeed()) {
      append_optional_linefeed();
    }
    for (SelectorComponentObj& item : complex->elements()) {
      if (many) append_optional_space();
      if (SelectorCombinator* combinator = item->isaSelectorCombinator()) {
        visitSelectorCombinator(combinator);
      }
      else if (CompoundSelector* compound = item->isaCompoundSelector()) {
        visitCompoundSelector(compound);
      }
      many = true;
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

    for (SimpleSelectorObj& item : compound->elements()) {
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
    case SelectorCombinator::Combinator::CHILD: append_string(">"); break;
    case SelectorCombinator::Combinator::GENERAL: append_string("~"); break;
    case SelectorCombinator::Combinator::ADJACENT: append_string("+"); break;
    }
    append_optional_space();
    // Add the post line break (from ruby sass)
    // Dart sass uses another logic for newlines
    // if (combinator->hasPostLineBreak()) {
    //   if (output_style() != COMPACT) {
    //     // append_optional_linefeed();
    //   }
    // }
  }

  void Inspect::visitIDSelector(IDSelector* id)
  {
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
        append_string(pseudo->argument());
        if (pseudo->selector() && !pseudo->argument().empty()) {
          if (!pseudo->selector()->empty()) {
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
    }

    if (in_declaration) in_comma_array = true;

    for (size_t i = 0, L = list->size(); i < L; ++i) {

      if (i == 0) append_indentation();
      if ((*list)[i] == nullptr) continue;
      schedule_mapping(list->get(i)->last());
      // add_open_mapping(list->get(i)->last());
      visitComplexSelector(list->get(i));
      // add_close_mapping(list->get(i)->last());
      if (i < L - 1) {
        scheduled_space = 0;
        append_comma_separator();
      }
    }

    in_comma_array = was_comma_array;
    // probably ruby sass equivalent of element_needs_parens
    if (!in_declaration && in_comma_array) {
      append_string(")");
    }
  }

  void Inspect::visitTypeSelector(TypeSelector* type)
  {
    acceptNameSpaceSelector(type);
  }

  // Returns whether [value] needs parentheses as an
  // element in a list with the given [separator].
  bool _elementNeedsParens(SassSeparator separator, const Value* value) {
    if (const List * list = value->isaList()) {
      if (list->size() < 2) return false;
      if (list->hasBrackets()) return false;
      return separator == SASS_COMMA
        ? list->separator() == SASS_COMMA
        : list->separator() != SASS_UNDEF;
    }
    return false;
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
      list->separator() == SASS_COMMA;

    if (list->hasBrackets()) {
      append_char($lbracket);
    }
    else if (preserveComma) {
      append_char($lparen);
    }

    add_open_mapping(list);

    const sass::vector<ValueObj>& values(list->elements());

    bool first = true;
    sass::string joiner =
      list->separator() == SASS_SPACE ? " " :
      output_style() == SASS_STYLE_COMPRESSED ? "," : ", ";

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

    add_close_mapping(list);

    if (preserveComma) {
      append_char($comma);
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

    ColorRgbaObj c = color->toRGBA();

    // original color name
    // maybe an unknown token
    sass::string name = c->disp();

    // resolved color
    sass::string res_name = name;

    double r = round32(clamp(c->r(), 0.0, 255.0), opt.precision);
    double g = round32(clamp(c->g(), 0.0, 255.0), opt.precision);
    double b = round32(clamp(c->b(), 0.0, 255.0), opt.precision);
    double a = clamp<double>(c->a(), 0, 1);

    // get color from given name (if one was given at all)
    if (name != "" && name_to_color(name)) {
      const ColorRgba* n = name_to_color(name);
      r = round32(clamp(n->r(), 0.0, 255.0), opt.precision);
      g = round32(clamp(n->g(), 0.0, 255.0), opt.precision);
      b = round32(clamp(n->b(), 0.0, 255.0), opt.precision);
      a = clamp(n->a(), 0.0, 1.0);
    }
    // otherwise get the possible resolved color name
    else {
      double numval = r * 0x10000 + g * 0x100 + b;
      if (color_to_name(numval))
        res_name = color_to_name(numval);
    }

    sass::sstream hexlet;
    // dart sass compressed all colors in regular css always
    // ruby sass and libsass does it only when not delayed
    // since color math is going to be removed, this can go too
    bool compressed = opt.output_style == SASS_STYLE_COMPRESSED;
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
      ss << static_cast<unsigned long>(r) << ",";
      if (!compressed) ss << " ";
      ss << static_cast<unsigned long>(g) << ",";
      if (!compressed) ss << " ";
      ss << static_cast<unsigned long>(b) << ",";
      if (!compressed) ss << " ";
      ss << a << ')';
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
    for (auto kv : value->elements()) {
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


  void Inspect::visitNumber(Number* value)
  {
    if (value->lhsAsSlash() && value->rhsAsSlash()) {
      visitNumber(value->lhsAsSlash());
      append_string("/");
      visitNumber(value->rhsAsSlash());
      return;
    }

    if (std::isnan(value->value())) {
      append_string("NaN");
      return;
    }

    if (std::isinf(value->value())) {
      append_string("Infinity");
      return;
    }

    sass::sstream ss;
    ss.precision(opt.precision);
    // This can be a bottleneck
    ss << std::fixed << value->value();

    sass::string res = ss.str();
    size_t s = res.size();

    // delete trailing zeros
    for (s = s - 1; s > 0; --s)
    {
      if (res[s] == '0') {
        res.erase(s, 1);
      }
      else break;
    }

    // delete trailing decimal separator
    if (res[s] == '.') res.erase(s, 1);

    // some final cosmetics
    if (res == "0.0") res = "0";
    else if (res == "") res = "0";
    else if (res == "-0") res = "0";
    else if (res == "-0.0") res = "0";

    // add unit now
    res += value->unit();

    // output the final token
    append_token(res, value);
  }

  void Inspect::visitString(String* value)
  {
    if (quotes && value->hasQuotes()) {
      renderQuotedString(value->value());
    }
    else {
      renderUnquotedString(value->value());
      // append_token(s->value(), s);
    }
  }




















}

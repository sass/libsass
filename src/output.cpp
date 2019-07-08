#include "output.hpp"

#include "ast_css.hpp"
#include "ast_values.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "exceptions.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  Output::Output(
    SassOutputOptionsCpp& opt,
    bool srcmap_enabled) :
    Cssize(opt, srcmap_enabled)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // local helper function
  void trim_trailing_lines(sass::string& text)
  {
    auto start = text.begin();
    auto lastlf = text.end();
    auto end = lastlf - 1;
    while (end != start) {
      if (Character::isNewline(*end)) {
        lastlf = end;
        end--;
      }
      else if (Character::isWhitespace(*end)) {
        end--;
      }
      else {
        break;
      }
    }
    if (lastlf != text.end()) {
      text = sass::string(start, lastlf) + " ";
    }
  }
  // EO string_trim_trailing_lines

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  OutputBuffer Output::getBuffer(void)
  {
    // This needs saving
    Inspect inspect(opt, wbuf.smap ? true : false);

    size_t size_nodes = top_nodes.size();
    for (size_t i = 0; i < size_nodes; i++) {
      top_nodes[i]->accept(&inspect);
      inspect.append_mandatory_linefeed();
    }

    // flush scheduled outputs
    // maybe omit semicolon if possible
    inspect.finalize(wbuf.buffer.size() == 0);
    // prepend buffer on top
    prepend_output(inspect.output());
    // make sure we end with a linefeed
    if (!StringUtils::endsWith(wbuf.buffer, opt.linefeed)) {
      // if the output is not completely empty
      if (!wbuf.buffer.empty()) append_string(opt.linefeed);
    }

    // search for unicode char
    for (const char& chr : wbuf.buffer) {
      // skip all ASCII chars
      if (static_cast<unsigned>(chr) < 128) continue;
      // declare the charset
      if (output_style() != SASS_STYLE_COMPRESSED)
        charset = "@charset \"UTF-8\";"
        + sass::string(opt.linefeed);
      else charset = "\xEF\xBB\xBF";
      // abort search
      break;
    }

    // add charset as first line, before comments and imports
    if (!charset.empty()) prepend_string(charset);

    return std::move(wbuf);

  }
  // EO getBuffer

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void Output::visitCssImport(CssImport* imp)
  {
    if (imp->outOfOrder()) {
      top_nodes.emplace_back(imp);
    }
    else {
      Cssize::visitCssImport(imp);
    }
  }
  // EO visitCssImport

  void Output::visitCssComment(CssComment* c)
  {
    bool important = c->isPreserved();
    if (output_style() == SASS_STYLE_COMPRESSED || output_style() == SASS_STYLE_COMPACT) {
      if (!important) return;
    }
    if (output_style() != SASS_STYLE_COMPRESSED || important) {
      if (wbuf.buffer.size() == 0) {
        top_nodes.emplace_back(c);
      }
      else {
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
  }
  // EO visitCssComment

  void Output::visitCssMediaRule(CssMediaRule* rule)
  {
    // Avoid null pointer exception
    if (rule == nullptr) return;
    // Skip empty or invisible rule
    if (rule->isInvisibleCss()) return;
    // Let inspect do its magic
    Cssize::visitCssMediaRule(rule);
  }
  // EO visitCssMediaRule

  void Output::visitMap(Map* m)
  {
    // should be handle in check_expression
    throw Exception::InvalidCssValue({}, *m);
  }
  // EO visitMap

  void Output::visitString(String* s)
  {
    if (!in_custom_property) {
      if (s->hasQuotes()) {
        renderQuotedString(s->value());
      }
      else {
        renderUnquotedString(s->value());
      }
    }
    else {
      sass::string value(s->value());
      trim_trailing_lines(value);
      append_token(value, s);
    }
  }
  // EO visitString

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

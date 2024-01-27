/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
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
    OutputOptions& opt) :
    Cssize(opt)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Local helper function to right-trim multi-line text.
  // Only the last line is right trimmed in an optimized way.
  // Note: this is merely cosmetic to match dart-sass output.
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

  // Get the output buffer for what has been rendered.
  // This will actually invoke quite a bit under the hood.
  // First we will render the hoisted imports with comments.
  // Then we will prepend this string to the already rendered one.
  // This involves adjusting the source-map so mappings still match.
  // Note: this is less complicated than one could initially think, as
  // Note: source-maps are easy to manipulate with full lines only.
  OutputBuffer Output::getBuffer(void)
  {
    // Create an identical rendered for the hoisted part
    Inspect inspect(outopt);

    // Render the hoisted imports with comments
    for (size_t i = 0; i < imports.size(); i++) {
      imports[i]->accept(&inspect);
      inspect.append_mandatory_linefeed();
    }

    // maybe omit semicolon if possible
    inspect.finalize(wbuf.buffer.size() == 0);
    // prepend buffer on top
    prepend_output(inspect.output());
    // flush trailing comments
    flushCssComments();

    // make sure we end with a linefeed
    if (!StringUtils::endsWith(wbuf.buffer, outopt.linefeed)) {
      // if the output is not completely empty
      if (!wbuf.buffer.empty()) append_string(outopt.linefeed);
    }

    // search for unicode char
    for (const char& chr : wbuf.buffer) {
      // skip all ASCII chars
      if (static_cast<unsigned>(chr) < 128) continue;
      // declare the charset
      if (output_style() != SASS_STYLE_COMPRESSED)
        charset = "@charset \"UTF-8\";"
        + sass::string(outopt.linefeed);
      else charset = "\xEF\xBB\xBF";
      // abort search
      break;
    }

    // add charset as first line, before comments and imports
    // adding any unicode BOM should not alter source-maps
    if (!charset.empty()) prepend_string(charset);

    // pass the buffer back
    return std::move(wbuf);
  }
  // EO getBuffer

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Helper function to append comment to output
  void Output::printCssComment(CssComment* comment)
  {
    // Ignore sourceMappingURL and sourceURL comments (ToDo: use regexp?).
    if (StringUtils::startsWith(comment->text(), "/*# sourceMappingURL=")) return;
    else if (StringUtils::startsWith(comment->text(), "/*# sourceURL=")) return;
    bool important = comment->isPreserved();
    if (output_style() == SASS_STYLE_COMPRESSED || output_style() == SASS_STYLE_COMPACT) {
      if (!important) return;
    }
    if (output_style() != SASS_STYLE_COMPRESSED || important) {
      append_indentation();
      append_string(comment->text());
      if (indentation == 0) {
        append_mandatory_linefeed();
      }
      else {
        append_optional_linefeed();
      }
    }
  }
  // EO printCssComment

  // Flushes all queued comments to output
  // Also resets and clears the queue
  void Output::flushCssComments()
  {
    for (CssComment* comment : comments) {
      printCssComment(comment);
    }
    comments.clear();
  }
  // EO flushCssComments

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void Output::visitCssImport(CssImport* imp)
  {
    if (imp->outOfOrder()) {
      imports.insert(imports.end(),
        comments.begin(), comments.end());
      imports.emplace_back(imp);
      comments.clear();
    }
    else {
      // This case is possible if an `@import` within
      // an imported css file is inside a `CssStyleRule`.
      flushCssComments();
      Cssize::visitCssImport(imp);
    }
  }
  // EO visitCssImport

  void Output::visitCssComment(CssComment* comment)
  {
    if (hoistComments) {
      comments.emplace_back(comment);
    }
    else {
      printCssComment(comment);
    }
  }
  // EO visitCssComment

  void Output::visitCssStyleRule(CssStyleRule* rule)
  {
    // Prepend previous comments
    flushCssComments();
    // Never hoist nested comments
    RAII_FLAG(hoistComments, false);
    // Let inspect do its magic
    Cssize::visitCssStyleRule(rule);
  }
  // EO visitCssStyleRule

  void Output::visitCssSupportsRule(CssSupportsRule* rule)
  {
    // Prepend previous comments
    flushCssComments();
    // Never hoist nested comments
    RAII_FLAG(hoistComments, false);
    // Let inspect do its magic
    Cssize::visitCssSupportsRule(rule);
  }
  // EO visitCssSupportsRule

  void Output::visitCssAtRule(CssAtRule* rule)
  {
    // Prepend previous comments
    flushCssComments();
    // Never hoist nested comments
    RAII_FLAG(hoistComments, false);
    // Let inspect do its magic
    Cssize::visitCssAtRule(rule);
  }
  // EO visitCssAtRule

  void Output::visitCssMediaRule(CssMediaRule* rule)
  {
    // Avoid null pointer exception
    if (rule == nullptr) return;
    // Skip empty or invisible rule
    if (rule->isInvisibleCss()) return;
    // Prepend previous comments
    flushCssComments();
    // Never hoist nested comments
    RAII_FLAG(hoistComments, false);
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
      Inspect::visitString(s);
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

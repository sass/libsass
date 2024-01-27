/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_SASS_HPP
#define SASS_PARSER_SASS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser_stylesheet.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class SassParser final : public StylesheetParser
  {
  public:

    enum SassIndentType {
      AUTO, TABS, SPACES,
    };

    SassParser(
      Compiler& context,
      SourceDataObj source) :
      StylesheetParser(
        context, source),
      currentIndentation(0),
      nextIndentation(NPOS),
      nextIndentationEnd({
        source->content(),
        Offset() }),
      indentType(SassIndentType::AUTO)
    {}

  protected:

    // The current indentation level
    size_t currentIndentation = 0;

    // The indentation level of the next source line after the scanner's
    // position, or `null` if that hasn't been computed yet.
    // A source line is any line that's not entirely whitespace.
    size_t nextIndentation;

    // The beginning of the next source line after the scanner's
    // position, or `null` if that hasn't been computed yet.
    // A source line is any line that's not entirely whitespace.
    StringScannerState nextIndentationEnd;

    // Whether the document is indented using spaces or tabs.
    // If this is `true`, the document is indented using spaces. If it's `false`,
    // the document is indented using tabs. If it's `null`, we haven't yet seen
    // the indentation character used by the document.
    SassIndentType indentType = SassIndentType::AUTO;

    // Some helper function to do the most generic queries
    bool useTabIndentation() { return indentType == SassIndentType::TABS; }
    bool useSpaceIndentation() { return indentType == SassIndentType::SPACES; }

    // Whether this is a plain CSS stylesheet.
    bool plainCss() const override final { return false; }

    // Whether this is parsing the indented syntax.
    bool isIndented() const override final { return true; };

    // Parses and returns a selector used in a style rule.
    Interpolation* styleRuleSelector() override final;

    // Asserts that the scanner is positioned before a statement separator,
    // or at the end of a list of statements. If the [name] of the parent
    // rule is passed, it's used for error reporting. This consumes
    // whitespace, but nothing else, including comments.
    void expectStatementSeparator(sass::string name) override final;

    // Whether the scanner is positioned at the end of a statement.
    bool atEndOfStatement() override final;

    // Whether the scanner is positioned before a block of
    // children that can be parsed with [children].
    bool lookingAtChildren() override final;

    // Consumes an argument to an `@import` rule.
    // If anything is found it will be added to [rule].
    void scanImportArgument(ImportRule* rule) override final;

    // Tries to scan an `@else` rule after an `@if` block. Returns whether
    // that succeeded. This should just scan the rule name, not anything 
    // afterwards. [ifIndentation] is the result of [currentIndentation]
    // from before the corresponding `@if` was parsed.
    bool scanElse(size_t ifIndentation) override final;

    // Consumes a block of child statements. Unlike most production consumers,
    // this does *not* consume trailing whitespace. This is necessary to ensure
    // that the source span for the parent rule doesn't cover whitespace after the rule.
    StatementVector readChildren(Statement* (StylesheetParser::* child)()) override final;

    // Consumes top-level statements. The [statement] callback may return `nullptr`,
    // indicating that a statement was consumed that shouldn't be added to the AST.
    StatementVector readStatements(Statement* (StylesheetParser::* statement)()) override final;

    // Consumes a child of the current statement. This consumes
    // children that are allowed at all levels of the document;
    // the [child] parameter is called to consume any children
    // that are specifically allowed in the caller's context.
    Statement* parseChild(Statement* (StylesheetParser::* statement)());

    // Consumes an indented-style silent comment.
    SilentComment* readSilentComment();

    // Consumes an indented-style loud context.
    // This overrides loud comment consumption so
    // that it doesn't consume multi-line comments.
    LoudComment* readLoudComment();

    // Consumes and ignores a loud (CSS-style) comment.
    // This overrides loud comment consumption so that
    // it doesn't consume multi-line comments.
    void scanLoudComment() override final;

    void scanWhitespaceWithoutComments() override final;

    // Expect and consume a single newline character.
    void expectNewline();

    // Returns whether the scanner is immediately before *two* newlines.
    bool lookingAtDoubleNewline();

    // As long as the scanner's position is indented beneath the
    // starting line, runs [body] to consume the next statement.
    void whileIndentedLower(Statement* (StylesheetParser::* child)(), StatementVector& children);

    // Consumes indentation whitespace and returns
    // the indentation level of the next line.
    size_t readIndentation();

    // Returns the indentation level of the next line.
    size_t peekIndentation();

    // Ensures that the document uses consistent characters for indentation.
    // The [containsTab] and [containsSpace] parameters refer to
    // a single line of indentation that has just been parsed.
    void checkIndentationConsistency(bool containsTab, bool containsSpace);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_SCSS_HPP
#define SASS_PARSER_SCSS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser_stylesheet.hpp"

namespace Sass {

  class ScssParser : public StylesheetParser
  {
  public:

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    ScssParser(
      Compiler& context,
      SourceDataObj source) :
      StylesheetParser(
        context, source)
    {}

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // So far we are not parsing css
    virtual bool plainCss() const override {
      return false;
    }

    // We are sure scss is not indented syntax
    bool isIndented() const override final {
      return false;
    };

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  protected:

    // Parses and returns a selector used in a style rule.
    virtual Interpolation* styleRuleSelector() override;

    // Asserts that the scanner is positioned before a statement separator,
    // or at the end of a list of statements. If the [name] of the parent
    // rule is passed, it's used for error reporting. This consumes
    // whitespace, but nothing else, including comments.
    virtual void expectStatementSeparator(sass::string name) override;

    // Whether the scanner is positioned at the end of a statement.
    virtual bool atEndOfStatement() override;

    // Whether the scanner is positioned before a block of
    // children that can be parsed with [children].
    virtual bool lookingAtChildren() override;

    // Tries to scan an `@else` rule after an `@if` block, and returns whether 
    // that succeeded. This should just scan the rule name, not anything
    // afterwards. [ifIndentation] is the result of [currentIndentation]
    // from before the corresponding `@if` was parsed.
    virtual bool scanElse(size_t ifIndentation) override;

    // Consumes a block of child statements. Unlike most production consumers,
    // this does *not* consume trailing whitespace. This is necessary to ensure
    // that the source span for the parent rule doesn't cover whitespace after the rule.
    virtual StatementVector readChildren(
      Statement* (StylesheetParser::* child)()) override;

    // Consumes top-level statements. The [statement] callback may return `null`,
    // indicating that a statement was consumed that shouldn't be added to the AST.
    virtual StatementVector readStatements(
      Statement* (StylesheetParser::* statement)()) override;

    // Consumes a statement-level silent comment block.
    virtual SilentComment* readSilentComment();

    // Consumes a statement-level loud comment block.
    virtual LoudComment* readLoudComment();

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  };

}

#endif

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_STYLESHEET_HPP
#define SASS_PARSER_STYLESHEET_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser_base.hpp"
#include "ast_statements.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class StylesheetParser : public BaseParser
  {

    friend class ExpressionParser;

  protected:

    // Current parsing recursion depth
    // Just a counter for nesting guard
    size_t recursion = 0;

    // Whether we are inside a loop directive such as `@for`, `@while` or `@each`.
    // This will disable some incompatible static variable optimizations.
    bool inLoopDirective = false;

    // Whether we've consumed a rule other than `@charset`, `@forward`, or `@use`.
    bool isUseAllowed = true;

    // Whether the parser is currently parsing the contents of a mixin declaration.
    bool inMixin = false;

    // Whether the current mixin contains at least one `@content` rule.
    bool mixinHasContent = false;

    // Whether the parser is currently parsing a content block passed to a mixin.
    bool inContentBlock = false;

    // Whether the parser is currently parsing a control directive such as `@if` or `@each`.
    bool inControlDirective = false;

    // Whether the parser is currently parsing an unknown rule.
    bool inUnknownAtRule = false;

    // Whether the parser is currently parsing a style rule.
    bool inStyleRule = false;

    // Whether the parser is currently within a parenthesized expression.
    bool inParentheses = false;

  public:

    // Value constructor
    StylesheetParser(
      Compiler& context,
      SourceDataObj source) :
      BaseParser(
        context, source),
      recursion(0),
      isUseAllowed(true),
      inMixin(false),
      mixinHasContent(false),
      inContentBlock(false),
      inControlDirective(false),
      inUnknownAtRule(false),
      inStyleRule(false),
      inParentheses(false)
    {}

    // Main parser entry function
    Root* parseRoot();

    // Parse external callback function
    ExternalCallable* parseExternalCallable();

    // Argument declaration is tricky in terms of scoping.
    // The variable before the colon is defined on the new frame.
    // But the right side is evaluated in the parent scope.
    ArgumentDeclaration* parseArgumentDeclaration();

    // Whether this is a plain CSS stylesheet.
    virtual bool plainCss() const { return false; }

    // Whether this is parsing the indented syntax.
    virtual bool isIndented() const { return false; };

  protected:

    // Parses and returns a selector used in a style rule.
    virtual Interpolation* styleRuleSelector() = 0;

    // Asserts that the scanner is positioned before a statement separator,
    // or at the end of a list of statements. If the [name] of the parent
    // rule is passed, it's used for error reporting. This consumes
    // whitespace, but nothing else, including comments.
    virtual void expectStatementSeparator(sass::string name = "") = 0;

    // Whether the scanner is positioned at the end of a statement.
    virtual bool atEndOfStatement() = 0;

    // Whether the scanner is positioned before a block of
    // children that can be parsed with [children].
    virtual bool lookingAtChildren() = 0;

    // Tries to scan an `@else` rule after an `@if` block. Returns whether
    // that succeeded. This should just scan the rule name, not anything 
    // afterwards. [ifIndentation] is the result of [currentIndentation]
    // from before the corresponding `@if` was parsed.
    virtual bool scanElse(size_t ifIndentation) = 0;

    // Consumes a variable declaration.
    virtual Statement* readVariableDeclaration();

    // Consumes a block of child statements. Unlike most production consumers,
    // this does *not* consume trailing whitespace. This is necessary to ensure
    // that the source span for the parent rule doesn't cover whitespace after the rule.
    virtual StatementVector readChildren(Statement* (StylesheetParser::* child)()) = 0;

    // Consumes top-level statements. The [statement] callback may return `nullptr`,
    // indicating that a statement was consumed that shouldn't be added to the AST.
    virtual StatementVector readStatements(Statement* (StylesheetParser::* statement)()) = 0;

    // Consumes a statement that's allowed at the top level of the stylesheet or
    // within nested style and at rules. If [root] is `true`, this parses at-rules
    // that are allowed only at the root of the stylesheet.
    Statement* readStatement(bool root = false);

    // Helpers to either parse root or children context
    Statement* readRootStatement() { return readStatement(true); }
    Statement* readChildStatement() { return readStatement(false); }

    // Consumes a style rule.
    StyleRule* readStyleRule();

    // Consumes a [Declaration] or a [StyleRule].
    //
    // When parsing the contents of a style rule, it can be difficult to tell
    // declarations apart from nested style rules. Since we don't thoroughly
    // parse selectors until after resolving interpolation, we can share a bunch
    // of the parsing of the two, but we need to disambiguate them first. We use
    // the following criteria:
    //
    // * If the entity doesn't start with an identifier followed by a colon,
    //   it's a selector. There are some additional mostly-unimportant cases
    //   here to support various declaration hacks.
    //
    // * If the colon is followed by another colon, it's a selector.
    //
    // * Otherwise, if the colon is followed by anything other than
    //   interpolation or a character that's valid as the beginning of an
    //   identifier, it's a declaration.
    //
    // * If the colon is followed by interpolation or a valid identifier, try
    //   parsing it as a declaration value. If this fails, backtrack and parse
    //   it as a selector.
    //
    // * If the declaration value is valid but is followed by "{", backtrack and
    //   parse it as a selector anyway. This ensures that ".foo:bar {" is always
    //   parsed as a selector and never as a property with nested properties
    //   beneath it.
    Statement* readDeclarationOrStyleRule();

    // Tries to parse a declaration, and returns the value parsed so
    // far if it fails. This can return either an [InterpolationBuffer],
    // indicating that it couldn't consume a declaration and that selector
    // parsing should be attempted; or it can return a [Declaration],
    // indicating that it successfully consumed a declaration.
    Declaration* tryDeclarationOrBuffer(InterpolationBuffer& buffer);

    // Consumes a property declaration. This is only used in contexts where
    // declarations are allowed but style rules are not, such as nested
    // declarations. Otherwise, [readDeclarationOrStyleRule] is used instead.
    Declaration* readDeclaration(bool parseCustomProperties = true);

    // Consumes a statement that's allowed within a declaration.
    Statement* readDeclarationOrAtRule();

    // Consumes an at-rule. This consumes at-rules that are allowed at all levels
    // of the document; the [child] parameter is called to consume any at-rules
    // that are specifically allowed in the caller's context. If [root] is `true`,
    // this parses at-rules that are allowed only at the root of the stylesheet.
    virtual Statement* readAtRule(Statement* (StylesheetParser::* child)(), bool root = false);

    // Consumes an at-rule allowed within a property declaration.
    Statement* readDeclarationAtRule();

    // Consumes a statement allowed within a function.
    Statement* readFunctionAtRule();

    // Consumes an at-rule's name, with interpolation disallowed.
    sass::string readPlainAtRuleName();

    // Consumes an `@at-root` rule.
    // [start] should point before the `@`.
    AtRootRule* readAtRootRule(Offset start);

    // Consumes a query expression of the form `(foo: bar)`.
    Interpolation* readAtRootQuery();

    // Consumes a `@content` rule.
    // [start] should point before the `@`.
    ContentRule* readContentRule(Offset start);

    // Consumes a `@debug` rule.
    // [start] should point before the `@`.
    DebugRule* readDebugRule(Offset start);

    // Consumes an `@each` rule.
    // [start] should point before the `@`. [child] is called to consume any
    // children that are specifically allowed in the caller's context.
    EachRule* readEachRule(Offset start, Statement* (StylesheetParser::* child)());
    
    // Consumes a `@error` rule.
    // [start] should point before the `@`.
    ErrorRule* readErrorRule(Offset start);

    // Consumes a `@extend` rule.
    // [start] should point before the `@`.
    ExtendRule* readExtendRule(Offset start);

    // Consumes a `@function` rule.
    // [start] should point before the `@`.
    FunctionRule* readFunctionRule(Offset start);

    // Try to parse either `to` or `through`, if successful
    // we will return `true`. The boolean passed via [inclusive]
    // will be set to `true` if we parsed `through`. We return
    // `false` if neither of the tokens could be parsed.
    bool tryForRuleOperator(bool& inclusive);

    // Consumes a `@for` rule.
    // [start] should point before the `@`. [child] is called to consume any
    // children that are specifically allowed in the caller's context.
    ForRule* readForRule(Offset start, Statement* (StylesheetParser::* child)());

    // Consumes an `@if` rule.
    // [start] should point before the `@`. [child] is called to consume any
    // children that are specifically allowed in the caller's context.
    IfRule* readIfRule(Offset start, Statement* (StylesheetParser::* child)());

    // Consumes an `@import` rule.
    // [start] should point before the `@`.
    ImportRule* readImportRule(Offset start);

    // Consumes an argument to an `@import` rule.
    // If anything is found it will be added to [rule].
    virtual void scanImportArgument(ImportRule* rule);

    // Returns whether [url] indicates that an `@import` is a plain CSS import.
    bool isPlainImportUrl(const sass::string& url) const;

    // Consumes a supports condition and/or a media query after an `@import`.
    std::pair<SupportsConditionObj, InterpolationObj> tryImportQueries();

    // Consumes an `@include` rule.
    // [start] should point before the `@`.
    IncludeRule* readIncludeRule(Offset start);

    // Consumes a `@media` rule.
    // [start] should point before the `@`.
    MediaRule* readMediaRule(Offset start);

    // Consumes a mixin declaration.
    // [start] should point before the `@`.
    MixinRule* readMixinRule(Offset start);

    // Consumes a `@moz-document` rule. Gecko's `@-moz-document` diverges
    // from [the specification][] allows the `url-prefix` and `domain`
    // functions to omit quotation marks, contrary to the standard.
    // [the specification]: http://www.w3.org/TR/css3-conditional/
    AtRule* readMozDocumentRule(Offset start, Interpolation* name);

    // Consumes a `@return` rule.
    // [start] should point before the `@`.
    ReturnRule* readReturnRule(Offset start);

    // Consumes a `@supports` rule.
    // [start] should point before the `@`.
    SupportsRule* readSupportsRule(Offset start);

    // Consumes a `@use` rule.
    // [start] should point before the `@`.
    // UseRule* readUseRule(Offset start);

    // Consumes a `@warn` rule.
    // [start] should point before the `@`.
    WarnRule* readWarnRule(Offset start);

    // Consumes a `@while` rule. [start] should  point before the `@`. [child] is called 
    // to consume any children that are specifically allowed in the caller's context.
    WhileRule* readWhileRule(Offset start, Statement* (StylesheetParser::* child)());

    // Consumes an at-rule that's not explicitly supported by Sass.
    // [start] should point before the `@`. [name] is the name of the at-rule.
    AtRule* readAnyAtRule(Offset start, Interpolation* name);

    // Parse almost any value to report disallowed at-rule
    Statement* throwDisallowedAtRule(Offset start);

    // Consumes an argument invocation. If [mixin] is `true`, this is parsed 
    // as a mixin invocation. Mixin invocations don't allow the Microsoft-style
    // `=` operator at the top level, but function invocations do.
    ArgumentInvocation* readArgumentInvocation(bool mixin = false);

    // Consumes an expression. If [bracketList] is true, parses this expression as
    // the contents of a bracketed list. If [singleEquals] is true, allows the
    // Microsoft-style `=` operator at the top level. If [until] is passed, it's
    // called each time the expression could end and still be a valid expression.
    // When it returns `true`, this returns the expression.
    Expression* readExpression(
      bool bracketList = false, bool singleEquals = false,
      bool(StylesheetParser::* until)() = nullptr);

    // Returns `true` if scanner reached a `,`
    bool lookingAtComma();

    // Consumes an expression until it reaches a top-level comma. If [singleEquals] 
    // is true, this will allow the Microsoft-style `=` operator at the top level.
    Expression* readExpressionUntilComma(bool singleEquals = false);

    // Consumes an expression that doesn't contain any top-level whitespace.
    Expression* readSingleExpression();

    // Consumes a parenthesized expression.
    Expression* readParenthesizedExpression();

    // Not yet evaluated, therefore not Map yet
    Expression* readMapExpression(Expression* first, Offset start);

    // Consumes an expression that starts with a `#`.
    Expression* readHashExpression();

    // Consumes the contents of a hex color, after the `#`.
    ColorExpression* readColorExpression(StringScannerState start);

    // Returns whether [interpolation] is a plain
    // string that can be parsed as a hex color.
    bool isHexColor(Interpolation* interpolation) const;

    // Consumes a single hexadecimal digit.
    uint8_t readHexDigit();

    // Consumes an expression that starts with a `+`.
    Expression* readPlusExpression();

    // Consumes an expression that starts with a `-`.
    Expression* readMinusExpression();

    // Consumes an `!important` expression.
    StringExpression* readImportantExpression();

    // Consumes a unary operation expression.
    UnaryOpExpression* readUnaryOpExpression();

    // Consumes a number expression.
    NumberExpression* readNumberExpression();

    // Consumes the decimal component of a number and returns its value, or 0
    // if there is no decimal component. If [allowTrailingDot] is `false`, this
    // will throw an error if there's a dot without any numbers following it.
    // Otherwise, it will ignore the dot without consuming it.
    double tryDecimal(bool allowTrailingDot = false);

    // Consumes the exponent component of a number and returns
    // its value, or 1 if there is no exponent component.
    double tryExponent();

    // Consumes a unicode range expression.
    StringExpression* readUnicodeRange();

    // Consumes a variable expression.
    VariableExpression* readVariableExpression(bool hoist = true);

    // Consumes a parent selector expression.
    ParentExpression* readParentExpression();

    // Consumes a quoted string expression.
    StringExpression* readInterpolatedString();

    // Consumes an expression that starts like an identifier.
    virtual Expression* readIdentifierLike();

    // If [name] is the name of a function with special syntax, consumes it.
    // Otherwise, returns `null`. [start] is the location before the beginning of [name].
    StringExpression* trySpecialFunction(sass::string name, const Offset& start);

    // Consumes the contents of a plain-CSS `min()` or `max()` function into
    // [buffer] if one is available. Returns whether this succeeded. If [allowComma]
    // is `true` (the default), this allows `CalcValue` productions separated by commas.
    bool tryMinMaxContents(InterpolationBuffer& buffer, bool allowComma = true);

    // Consumes a function named [name] containing an optional `InterpolatedDeclarationValue`
    // and adds its text to [buffer]. Returns whether such a function could be consumed.
    bool tryMinMaxFunction(InterpolationBuffer& buffer, sass::string name = "");

    // Like [_urlContents], but returns `null` if the URL fails to parse.
    // [start] is the position before the beginning of the name.
    // [name] is the function's name; it defaults to `"url"`.
    Interpolation* tryUrlContents(const Offset& start, sass::string name = "");

    // Consumes a [url] token that's allowed to contain SassScript.
    // Returns either a  `StringExpression` or a `FunctionExpression`
    Expression* readFunctionOrStringExpression();

    // Consumes tokens up to "{", "}", ";", or "!".
    // This respects string and comment boundaries and supports interpolation.
    // Once this interpolation is evaluated, it's expected to be re-parsed.
    // Differences from [parseInterpolatedDeclarationValue] include:
    // * This does not balance brackets.
    // * This does not interpret backslashes, since
    //   the text is expected to be re-parsed.
    // * This supports Sass-style single-line comments.
    // * This does not compress adjacent whitespace characters.
    Interpolation* readAlmostAnyValue();

    // Consumes tokens until it reaches a top-level `";"`, `")"`, `"]"`, or `"}"` and 
    // returns their contents as a string. If [allowEmpty] is `false` (the default), this
    // requires at least one token. Unlike [declarationValue], this allows interpolation.
    Interpolation* readInterpolatedDeclarationValue(bool allowEmpty = false);

    // Consumes an identifier that may contain interpolation.
    Interpolation* readInterpolatedIdentifier();

    void consumeInterpolatedIdentifierBody(InterpolationBuffer& buffer);

    // Consumes interpolation.
    Expression* readSingleInterpolation();

    // Consumes a list of media queries.
    Interpolation* readMediaQueryList();

    // Consumes a single media query and appends it to [buffer].
    void readMediaQuery(InterpolationBuffer& buffer);

    // Consumes a media query feature.
    Interpolation* readMediaFeature();

    // Returns `false` until the scanner reaches a
    // top-level `<`, `>`, or a `=` that's not `==`.
    bool lookingAtExpressionEnd();

    // Consumes an expression until it reaches a
    // top-level `<`, `>`, or a `=` that's not `==`.
    Expression* readExpressionUntilComparison();

    // Consumes a `@supports` condition.
    SupportsCondition* readSupportsCondition();

    // Consumes a parenthesized supports condition, or an interpolation.
    SupportsCondition* readSupportsConditionInParens();

    // Tries to consume a negated supports condition.
    // Returns `nullptr` if it fails.
    SupportsNegation* trySupportsNegation();

    // Like [identifier], but rejects identifiers that begin with `_` or `-`.
    sass::string readPublicIdentifier();

    // Returns whether the scanner is immediately before an identifier that may contain
    // interpolation. This is based on the CSS algorithm, but it assumes all backslashes
    // start escapes and it considers interpolation to be valid in an identifier.
    // https://drafts.csswg.org/css-syntax-3/#would-start-an-identifier
    bool lookingAtInterpolatedIdentifier() const;

    // Returns whether the scanner is immediately before a sequence
    // of characters that could be part of an CSS identifier body.
    // The identifier body may include interpolation.
    bool lookingAtInterpolatedIdentifierBody() const;

    // Returns whether the scanner is immediately before a SassScript expression.
    bool lookingAtExpression() const;

    // Consumes a block of [child] statements and passes them, as well as
    // the span from [start] to the end of the child block, to [create].
    template <typename T, typename ...Args>
    T* withChildren(Statement* (StylesheetParser::* child)(),
      const Offset& start, Args... args)
    {
      StatementVector elements(readChildren(child));
      SharedImpl<T> result = SASS_MEMORY_NEW(T,
        scanner.relevantSpanFrom(start),
        args..., std::move(elements));
      scanWhitespaceWithoutComments();
      return result.detach();
    }

    // Resolve import of [path] and add imports to [rule]
    void resolveDynamicImport(
      ImportRule* rule, Offset start,
      const sass::string& path);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

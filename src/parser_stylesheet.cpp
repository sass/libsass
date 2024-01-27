/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_stylesheet.hpp"

#include <cstring>
#include "compiler.hpp"
#include "charcode.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "color_maps.hpp"
#include "exceptions.hpp"
#include "source_span.hpp"
#include "ast_imports.hpp"
#include "ast_supports.hpp"
#include "ast_statements.hpp"
#include "ast_expressions.hpp"
#include "parser_expression.hpp"

#include "debugger.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;
  using namespace StringUtils;

  ExternalCallable* StylesheetParser::parseExternalCallable()
  {
    // LibSass specials functions start with an `@`
    bool hasAt = scanner.scanChar(Character::$at);
    // Return new external callable object
    ExternalCallableObj callable =
      SASS_MEMORY_NEW(ExternalCallable, hasAt ?
        "@" + readIdentifier() : readIdentifier(),
        parseArgumentDeclaration(), nullptr);
    if (!scanner.isDone()) {
      error("expected selector.",
        scanner.rawSpan());
    }
    return callable.detach();
  }
  // EO parseExternalCallable

  // Parse stylesheet root block
  Root* StylesheetParser::parseRoot()
  {

    // skip over optional utf8 bom
    // ToDo: check influence on count
    scanner.scan(Strings::utf8bom);

    // Create initial states
    Offset start(scanner.offset);

    // Create new root object and setup all states
    RootObj root = SASS_MEMORY_NEW(Root, scanner.rawSpan());
    // Get pointer to variables of current context
    root->idxs = compiler.varRoot.stack.back();
    // Assign new module to the current context
    compiler.varRoot.stack.back()->module = root;

    // Set the current module context
    RAII_MODULE(modules, root);
    RAII_PTR(Root, modctx, root);

    // Get reference to (not yet) parsed children
    StatementVector& children(root->elements());

    // Check seems a bit esoteric but works
    if (compiler.included_sources.size() == 1) {
      // Apply headers only on very first include
      compiler.applyCustomHeaders(children,
        scanner.relevantSpanFrom(start));
    }

    // Parse nested root statements now
    StatementVector parsed(readStatements(
      &StylesheetParser::readRootStatement));
    
    // Move parsed children into our array
    children.insert(children.end(),
      std::make_move_iterator(parsed.begin()),
      std::make_move_iterator(parsed.end()));

    // Ensure everything is parsed
    scanner.expectDone();

    // Update parser state after we are done
    root->pstate(scanner.relevantSpanFrom(start));

    // Return root object
    return root.detach();
  }
  // EO parseRoot

  // Consumes a statement that's allowed at the top level of the stylesheet or
  // within nested style and at rules. If [root] is `true`, this parses at-rules
  // that are allowed only at the root of the stylesheet.
  Statement* StylesheetParser::readStatement(bool root)
  {
    inRoot = root; // expose
    Offset start(scanner.offset);
    switch (scanner.peekChar()) {

    case $at:
      return readAtRule(&StylesheetParser::readChildStatement, root);

    case $plus:
      if (!isIndented() || !lookingAtIdentifier(1)) {
        return readStyleRule();
      }
      isUseAllowed = false;
      start = scanner.offset;
      scanner.readChar();
      return readIncludeRule(start);

    case $equal:
      if (!isIndented()) return readStyleRule();
      isUseAllowed = false;
      start = scanner.offset;
      scanner.readChar();
      scanWhitespace();
      return readMixinRule(start);

    default:
      if (inStyleRule || inUnknownAtRule || inMixin || inContentBlock) {
        return readDeclarationOrStyleRule();
      }
      else {
        return readVariableDeclarationOrStyleRule();
      }

    }
    return nullptr;
  }
  // EO readStatement

  Expression* StylesheetParser::readNamespacedExpression(
    const sass::string& ns, Offset start)
  {
    if (scanner.peekChar() == $dollar) {
      auto name = variableName();
      // _assertPublic(name, () = > scanner.spanFrom(start));
      return new VariableExpression(
        scanner.relevantSpanFrom(start),
        name, ns);
    }
    sass::string name(readPublicIdentifier());
    CallableArgumentsObj args(readArgumentInvocation());
    return new FunctionExpression(
      scanner.relevantSpanFrom(start),
      std::move(name), args, ns);
  }

  // Consumes an `@import` rule.
  // [start] should point before the `@`.
  ImportRule* StylesheetParser::readImportRule(Offset start)
  {
    ImportRuleObj rule = SASS_MEMORY_NEW(
      ImportRule, scanner.relevantSpanFrom(start));

    do {
      scanWhitespace();
      scanImportArgument(rule);
      scanWhitespace();
    } while (scanner.scanChar($comma));
    // Check for expected finalization token
    expectStatementSeparator("@import rule");
    return rule.detach();
  }
  // EO readImportRule

  // Consumes an argument to an `@import` rule.
  // If anything is found it will be added to [rule].
  void StylesheetParser::scanImportArgument(ImportRule* rule)
  {
    const char* startpos = scanner.position;
    Offset start(scanner.offset);
    uint8_t next = scanner.peekChar();
    if (next == $u || next == $U) {
      Expression* url = readFunctionOrStringExpression();
      scanWhitespace();
      auto modifiers = tryImportModifiers();
      rule->append(SASS_MEMORY_NEW(StaticImport,
        scanner.relevantSpanFrom(start),
        SASS_MEMORY_NEW(Interpolation,
          url->pstate(), url),
        modifiers, inRoot));
      return;
    }

    sass::string url = string();
    const char* rawUrlPos = scanner.position;
    SourceSpan pstate = scanner.relevantSpanFrom(start);
    scanWhitespace();
    auto modifiers = tryImportModifiers();
    if (isPlainImportUrl(url) || modifiers != nullptr) {
      // Create static import that is never
      // resolved by libsass (output as is)
      rule->append(SASS_MEMORY_NEW(StaticImport,
        scanner.relevantSpanFrom(start),
        SASS_MEMORY_NEW(Interpolation, pstate,
          SASS_MEMORY_NEW(String, pstate,
            sass::string(startpos, rawUrlPos))),
        modifiers, inRoot));
    }
    // Otherwise return a dynamic import
    // Will resolve during the eval stage
    else {
      // Check for valid dynamic import
      if (inControlDirective || inMixin) {
        throwDisallowedAtRule(rule->pstate().position);
      }

      SourceSpan pstate(scanner.relevantSpanFrom(start));
      if (!compiler.callCustomImporters(url, pstate, rule)) {
        rule->append(SASS_MEMORY_NEW(IncludeImport,
          pstate, scanner.sourceUrl, url, nullptr));
      }

    }

  }
  // EO scanImportArgument


  // Tries to parse a name-spaced [VariableDeclaration], and returns the value
  // parsed so far if it fails.
  //
  // This can return either an [Interpolation], indicating that it couldn't
  // consume a variable declaration and that property declaration or selector
  // parsing should be attempted; or it can return a [VariableDeclaration],
  // indicating that it successfully consumed a variable declaration.
  bool StylesheetParser::tryVariableDeclarationOrInterpolation(
    AssignRule*& assignment, Interpolation*& interpolation)
  {

    if (!lookingAtIdentifier()) {
      interpolation = readInterpolatedIdentifier();
      return true;
    }

    Offset start(scanner.offset);
    sass::string identifier(readIdentifier());
    if (scanner.matches(".$")) {
      scanner.readChar();
      assignment = readVariableDeclarationWithoutNamespace(identifier, start);
      return true;
    }
    else {

      ItplStringObj prefix(SASS_MEMORY_NEW(ItplString,
        scanner.relevantSpanFrom(start), identifier));

      // Parse the rest of an interpolated identifier
      // if one exists, so callers don't have to.
      if (lookingAtInterpolatedIdentifierBody()) {
        interpolation = readInterpolatedIdentifier();
        interpolation->unshift(prefix.ptr());
      }
      else {
        interpolation = SASS_MEMORY_NEW(Interpolation,
          scanner.relevantSpanFrom(start), prefix);
      }

      return true;
    }

    return false;
  }

  AssignRule* StylesheetParser::readVariableDeclarationWithNamespace()
  {
    Offset start(scanner.offset);
    sass::string ns(readIdentifier());
    scanner.expectChar($dot);
    return readVariableDeclarationWithoutNamespace(ns, start);
  }

  // Returns whether [identifier] is module-private.
  // Assumes [identifier] is a valid Sass identifier.
  bool isPrivate(const sass::string& identifier)
  {
    return identifier[0] == $minus ||
      identifier[0] == $underscore;
  }
  // EO isPrivate

  // Throws an error if [identifier] isn't public.
  void StylesheetParser::assertPublicIdentifier(
    const sass::string& identifier, Offset start)
  {
    if (!isPrivate(identifier)) return;
    error("Private members can't be accessed from outside their modules.",
      scanner.relevantSpanFrom(start));
  }
  // EO assertPublicIdentifier

  // Consumes a style rule.
  StyleRule* StylesheetParser::readStyleRule(Interpolation* itpl)
  {
    isUseAllowed = false;
    RAII_FLAG(inStyleRule, true);

    // The indented syntax allows a single backslash to distinguish a style rule
    // from old-style property syntax. We don't support old property syntax, but
    // we do support the backslash because it's easy to do.
    if (isIndented()) scanner.scanChar($backslash);
    InterpolationObj readStyleRule(styleRuleSelector());
    if (itpl) {
      itpl->concat(readStyleRule); readStyleRule = itpl;
      readStyleRule->pstate(scanner.rawSpanFrom(itpl->pstate().position));
    }
    EnvFrame local(compiler, false);

    Offset start(scanner.offset);
    StyleRuleObj styles = withChildren<StyleRule>(
      &StylesheetParser::readChildStatement,
      start, readStyleRule.ptr(), local.idxs);

    if (isIndented() && styles->empty()) {
      compiler.addWarning(
        "This selector doesn't have any properties and won't be rendered.",
        itpl ? itpl->pstate() : SourceSpan{}, Logger::WARN_EMPTY_SELECTOR);
    }

    return styles.detach();

  }
  // EO readStyleRule

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
  Statement* StylesheetParser::readDeclarationOrStyleRule()
  {

    if (plainCss() && inStyleRule && !inUnknownAtRule) {
      return readPropertyOrVariableDeclaration();
    }

    // The indented syntax allows a single backslash to distinguish a style rule
    // from old-style property syntax. We don't support old property syntax, but
    // we do support the backslash because it's easy to do.
    if (isIndented() && scanner.scanChar($backslash)) {
      return readStyleRule();
    }

    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    Statement* declaration = tryDeclarationOrBuffer(buffer);

    if (declaration != nullptr) {
      return declaration;
    }

    // This differs from dart-sass

    buffer.addInterpolation(styleRuleSelector());
    // SourceSpan selectorPstate(scanner.relevantSpanFrom(start));
    SourceSpan selectorPstate(scanner.rawSpanFrom(start));

    RAII_FLAG(inStyleRule, true);

    if (buffer.empty()) {
      error("expected \"}\".",
        scanner.relevantSpan());
    }

    EnvFrame local(compiler, true);
    InterpolationObj itpl = buffer.getInterpolation(
      scanner.rawSpanFrom(start));
    StyleRuleObj rule = withChildren<StyleRule>(
      &StylesheetParser::readChildStatement,
      start, itpl.ptr(), local.idxs);
    if (isIndented() && rule->empty()) {
      compiler.addWarning(
        "This selector doesn't have any properties and won't be rendered.",
        selectorPstate, Logger::WARN_EMPTY_SELECTOR);
    }
    return rule.detach();
  }
  // readDeclarationOrStyleRule

  Statement* StylesheetParser::readVariableDeclarationOrStyleRule()
  {

    if (plainCss()) return readStyleRule();

    // The indented syntax allows a single backslash to distinguish a style rule
    // from old-style property syntax. We don't support old property syntax, but
    // we do support the backslash because it's easy to do.
    if (isIndented() && scanner.scanChar($backslash)) return readStyleRule();

    if (!lookingAtIdentifier()) return readStyleRule();

    // Offset start(scanner.offset);
    AssignRule* assignment = nullptr;
    Interpolation* interpolation = nullptr;
    tryVariableDeclarationOrInterpolation(assignment, interpolation);
    if (assignment) return assignment;
    return readStyleRule(interpolation); // , start
  }


  // Tries to parse a declaration, and returns the value parsed so
  // far if it fails. This can return either an [InterpolationBuffer],
  // indicating that it couldn't consume a declaration and that selector
  // parsing should be attempted; or it can return a [Declaration],
  // indicating that it successfully consumed a declaration.
  Statement* StylesheetParser::tryDeclarationOrBuffer(InterpolationBuffer& nameBuffer)
  {
    Offset start(scanner.offset);

    // Allow the "*prop: val", ":prop: val",
    // "#prop: val", and ".prop: val" hacks.
    uint8_t first = scanner.peekChar();
    bool startsWithPunctuation = false;
    if (first == $colon || first == $asterisk || first == $dot ||
        (first == $hash && scanner.peekChar(1) != $lbrace)) {
      sass::sstream strm;
      startsWithPunctuation = true;
      strm << scanner.readChar();
      strm << rawText(&StylesheetParser::scanWhitespace);
      nameBuffer.write(strm.str(), scanner.relevantSpanFrom(start));
    }

    if (!lookingAtInterpolatedIdentifier()) {
      return nullptr;
    }

    if (startsWithPunctuation == false) {
      Interpolation* itpl = nullptr;
      AssignRule* assignment = nullptr;
      tryVariableDeclarationOrInterpolation(assignment, itpl);
      if (assignment != nullptr) return assignment;
      if (itpl) nameBuffer.addInterpolation(itpl);
    }
    else {
    nameBuffer.addInterpolation(readInterpolatedIdentifier());
    }

    isUseAllowed = false;
    if (scanner.matches("/*")) nameBuffer.write(rawText(&StylesheetParser::scanLoudComment));

    StringBuffer midBuffer;
    midBuffer.write(rawText(&StylesheetParser::scanWhitespace));
    SourceSpan beforeColon(scanner.relevantSpanFrom(start));
    if (!scanner.scanChar($colon)) {
      if (!midBuffer.empty()) {
        nameBuffer.write($space);
      }
      return nullptr;
    }
    midBuffer.write($colon);

    // Parse custom properties as declarations no matter what.
    InterpolationObj name = nameBuffer.getInterpolation(beforeColon);
    if (startsWith(name->getInitialPlain(), "--")) {
      InterpolationObj value(readInterpolatedDeclarationValue());
      expectStatementSeparator("custom property");
      return SASS_MEMORY_NEW(Declaration,
        scanner.relevantSpanFrom(start), name,
        value->wrapInStringExpression(), true);
    }

    if (scanner.scanChar($colon)) {
      nameBuffer.write(midBuffer.buffer);
      nameBuffer.write($colon);
      return nullptr;
    }
    else if (isIndented() && lookingAtInterpolatedIdentifier()) {
      // In the indented syntax, `foo:bar` is always
      // considered a selector rather than a property.
      nameBuffer.write(midBuffer.buffer);
      return nullptr;
    }

    sass::string postColonWhitespace = rawText(&StylesheetParser::scanWhitespace);
    if (lookingAtChildren()) {
      return withChildren<Declaration>(
        &StylesheetParser::readDeclarationOrAtRule,
        start, name, nullptr, false);
    }

    midBuffer.write(postColonWhitespace);
    bool couldBeSelector = postColonWhitespace.empty()
      && lookingAtInterpolatedIdentifier();

    StringScannerState beforeDeclaration = scanner.state();
    ExpressionObj value;

    try {

      if (lookingAtChildren()) {
        SourceSpan pstate = scanner.relevantSpanFrom(scanner.offset);
        Interpolation* itpl = SASS_MEMORY_NEW(Interpolation, pstate);
        value = SASS_MEMORY_NEW(StringExpression, pstate, itpl, true);
      }
      else {
        value = readExpression();
      }

      if (lookingAtChildren()) {
        // Properties that are ambiguous with selectors can't have additional
        // properties nested beneath them, so we force an error. This will be
        // caught below and cause the text to be re-parsed as a selector.
        if (couldBeSelector) {
          expectStatementSeparator();
        }
      }
      else if (!atEndOfStatement()) {
        // Force an exception if there isn't a valid end-of-property character but
        // don't consume that character. This will also cause text to be re-parsed.
        expectStatementSeparator();
      }

    }
    catch (Exception::ParserException&) {
      if (!couldBeSelector) throw;

      // If the value would be followed by a semicolon, it's
      // definitely supposed to be a property, not a selector.
      scanner.backtrack(beforeDeclaration);
      InterpolationObj additional = readAlmostAnyValue();
      if (!isIndented() && scanner.peekChar() == $semicolon) throw;

      nameBuffer.write(midBuffer.buffer);
      nameBuffer.addInterpolation(additional);
      return nullptr;
    }

    if (lookingAtChildren()) {
      // Offset start(scanner.offset);
      return withChildren<Declaration>(
        &StylesheetParser::readDeclarationOrAtRule,
        start, name, value, false);
    }
    else {
      expectStatementSeparator();
      return SASS_MEMORY_NEW(Declaration,
        scanner.relevantSpanFrom(start), name, value);
    }

  }
  // EO tryDeclarationOrBuffer

  // Consumes a property declaration. This is only used in contexts where
  // declarations are allowed but style rules are not, such as nested
  // declarations. Otherwise, [readDeclarationOrStyleRule] is used instead.
  Statement* StylesheetParser::readPropertyOrVariableDeclaration(bool parseCustomProperties)
  {

    Offset start(scanner.offset);

    InterpolationObj name;
    // Allow the "*prop: val", ":prop: val",
    // "#prop: val", and ".prop: val" hacks.
    uint8_t first = scanner.peekChar();
    if (first == $colon ||
      first == $asterisk ||
      first == $dot ||
      (first == $hash && scanner.peekChar(1) != $lbrace)) {
      InterpolationBuffer nameBuffer(scanner);
      nameBuffer.write(scanner.readChar());
      nameBuffer.write(rawText(&StylesheetParser::scanWhitespace));
      nameBuffer.addInterpolation(readInterpolatedIdentifier());
      name = nameBuffer.getInterpolation(scanner.relevantSpanFrom(start));
    }
    else if (!plainCss()) {
      AssignRule* assignment = nullptr;
      Interpolation* interpolation = nullptr;
      tryVariableDeclarationOrInterpolation(assignment, interpolation);
      if (assignment) return assignment; else name = interpolation;
    }
    else {
      name = readInterpolatedIdentifier();
    }

    scanWhitespace();
    scanner.expectChar($colon);
    scanWhitespace();

    if (parseCustomProperties && startsWith(name->getInitialPlain(), "--", 2)) {
      InterpolationObj value(readInterpolatedDeclarationValue());
      expectStatementSeparator("custom property");
      return SASS_MEMORY_NEW(Declaration,
        scanner.relevantSpanFrom(start),
        name, value->wrapInStringExpression());
    }

    if (lookingAtChildren()) {
      if (plainCss()) {
        error("Nested declarations aren't allowed in plain CSS.",
          scanner.rawSpan());
      }
      return withChildren<Declaration>(
        &StylesheetParser::readDeclarationOrAtRule, start, name,
        nullptr, startsWith(name->getInitialPlain(), "--", 2));
    }

    ExpressionObj value = readExpression();
    if (lookingAtChildren()) {
      if (plainCss()) {
        error("Nested declarations aren't allowed in plain CSS.",
          scanner.rawSpan());
      }
      // only without children;
      return withChildren<Declaration>(
        &StylesheetParser::readDeclarationOrAtRule,
        start, name, value,
        startsWith(name->getInitialPlain(), "--", 2));
    }
    else {
      expectStatementSeparator();
      return SASS_MEMORY_NEW(Declaration,
        scanner.relevantSpanFrom(start), name, value,
        startsWith(name->getInitialPlain(), "--", 2));
    }

  }
  // EO readPropertyOrVariableDeclaration

  // Consumes a statement that's allowed within a declaration.
  Statement* StylesheetParser::readDeclarationOrAtRule() // _declarationChild
  {
    if (scanner.peekChar() == $at) {
      return readDeclarationAtRule();
    }
    return readPropertyOrVariableDeclaration(false);
  }
  // EO readDeclarationOrAtRule

  // Consumes an at-rule. This consumes at-rules that are allowed at all levels
  // of the document; the [child] parameter is called to consume any at-rules
  // that are specifically allowed in the caller's context. If [root] is `true`,
  // this parses at-rules that are allowed only at the root of the stylesheet.
  Statement* StylesheetParser::readAtRule(Statement* (StylesheetParser::* child)(), bool root)
  {
    // NOTE: this logic is largely duplicated in CssParser.atRule.
    // Most changes here should be mirrored there.

    Offset start(scanner.offset);
    scanner.expectChar($at, "@-rule");
    InterpolationObj name = readInterpolatedIdentifier();
    scanWhitespace();

    // We want to set [isUseAllowed] to `false` *unless* we're parsing
    // `@charset`, `@forward`, or `@use`. To avoid double-comparing the rule
    // name, we always set it to `false` and then set it back to its previous
    // value if we're parsing an allowed rule.
    bool wasUseAllowed = isUseAllowed;
    isUseAllowed = false;

    sass::string plain(name->getPlainString());
    if (plain == "at-root") {
      return readAtRootRule(start);
    }
    else if (plain == "charset") {
      isUseAllowed = wasUseAllowed;
      if (!root) throwDisallowedAtRule(start);
      sass::string throwaway(string());
      return nullptr;
    }
    else if (plain == "content") {
      return readContentRule(start);
    }
    else if (plain == "debug") {
      return readDebugRule(start);
    }
    else if (plain == "each") {
      return readEachRule(start, child);
    }
    else if (plain == "else") {
      return throwDisallowedAtRule(start);
    }
    else if (plain == "error") {
      return readErrorRule(start);
    }
    else if (plain == "extend") {
      return readExtendRule(start);
    }
    else if (plain == "for") {
      return readForRule(start, child);
    }
    else if (plain == "function") {
      return readFunctionRule(start);
    }
    else if (plain == "if") {
      return readIfRule(start, child);
    }
    else if (plain == "import") {
      return readImportRule(start);
    }
    else if (plain == "include") {
      return readIncludeRule(start);
    }
    else if (plain == "media") {
      return readMediaRule(start);
    }
    else if (plain == "mixin") {
      return readMixinRule(start);
    }
    else if (plain == "-moz-document") {
      return readMozDocumentRule(start, name);
    }
    else if (plain == "return") {
      return throwDisallowedAtRule(start);
    }
    else if (plain == "supports") {
      return readSupportsRule(start);
    }
    else if (plain == "use") {
      // return readAnyAtRule(start, name);
      isUseAllowed = wasUseAllowed;
      if (!root) throwDisallowedAtRule(start);
      return readUseRule(start);
    }
    else if (plain == "forward") {
      // return readAnyAtRule(start, name);
      isUseAllowed = wasUseAllowed;
      if (!root) throwDisallowedAtRule(start);
      return readForwardRule(start);
    }
    else if (plain == "warn") {
      return readWarnRule(start);
    }
    else if (plain == "while") {
      return readWhileRule(start, child);
    }
    else {
      return readAnyAtRule(start, name);
    }

  }
  // EO atRule

  // Consumes an at-rule allowed within a property declaration.
  Statement* StylesheetParser::readDeclarationAtRule()
  {
    Offset start(scanner.offset);
    sass::string name = readPlainAtRuleName();
    
    if (name == "content") {
      return readContentRule(start);
    }
    else if (name == "debug") {
      return readDebugRule(start);
    }
    else if (name == "each") {
      return readEachRule(start,
        &StylesheetParser::readDeclarationOrAtRule);
    }
    else if (name == "else") {
      return throwDisallowedAtRule(start);
    }
    else if (name == "error") {
      return readErrorRule(start);
    }
    else if (name == "for") {
      return readForRule(start,
        &StylesheetParser::readDeclarationOrAtRule);
    }
    else if (name == "if") {
      return readIfRule(start,
        &StylesheetParser::readDeclarationOrAtRule);
    }
    else if (name == "include") {
      return readIncludeRule(start);
    }
    else if (name == "warn") {
      return readWarnRule(start);
    }
    else if (name == "while") {
      return readWhileRule(start,
        &StylesheetParser::readDeclarationOrAtRule);
    }
    else {
      return throwDisallowedAtRule(start);
    }
  }

  // Consumes a statement allowed within a function.
  Statement* StylesheetParser::readFunctionRuleChild()
  {
    if (scanner.peekChar() != $at) {

      StringScannerState state(scanner.state());
      try { return readVariableDeclarationWithNamespace(); }
      // dart-sass does some error cosmetic here
      catch (...) {

        scanner.backtrack(state);

      }

      // If a variable declaration failed to parse, it's possible the user
      // thought they could write a style rule or property declaration in a
      // function. If so, throw a more helpful error message.
      StatementObj statement(readDeclarationOrStyleRule());
      // ToDo: dart-sass has a try/catch clause here!?
      bool isStyleRule = statement->isaStyleRule() != nullptr;
      error(
        sass::string("@function rules may not contain ")
        + (isStyleRule ? "style rules." : "declarations."),
        statement->pstate());
    }

    Offset start(scanner.offset);
    sass::string name(readPlainAtRuleName());
    if (name == "debug") {
      return readDebugRule(start);
    }
    else if (name == "each") {
      return readEachRule(start,
        &StylesheetParser::readFunctionRuleChild);
    }
    else if (name == "else") {
      return throwDisallowedAtRule(start);
    }
    else if (name == "error") {
      return readErrorRule(start);
    }
    else if (name == "for") {
      return readForRule(start,
        &StylesheetParser::readFunctionRuleChild);
    }
    else if (name == "if") {
      return readIfRule(start,
        &StylesheetParser::readFunctionRuleChild);
    }
    else if (name == "return") {
      return readReturnRule(start);
    }
    else if (name == "warn") {
      return readWarnRule(start);
    }
    else if (name == "while") {
      return readWhileRule(start,
        &StylesheetParser::readFunctionRuleChild);
    }
    else {
      return throwDisallowedAtRule(start);
    }
  }

  // Consumes an at-rule's name, with interpolation disallowed.
  sass::string StylesheetParser::readPlainAtRuleName()
  {
    scanner.expectChar($at, "@-rule");
    sass::string name = readIdentifier();
    scanWhitespace();
    return name;
  }

  // Consumes an `@at-root` rule.
  // [start] should point before the `@`.
  AtRootRule* StylesheetParser::readAtRootRule(Offset start)
  {

    EnvFrame local(compiler, false);

    if (scanner.peekChar() == $lparen) {
      InterpolationObj query = readAtRootQuery();
      scanWhitespace();
      return withChildren<AtRootRule>(
        &StylesheetParser::readChildStatement,
        start, query, local.idxs);
    }
    else if (lookingAtChildren()) {
      return withChildren<AtRootRule>(
        &StylesheetParser::readChildStatement,
        start, nullptr, local.idxs);
    }
    StyleRule* child = readStyleRule();
    return SASS_MEMORY_NEW(AtRootRule,
      scanner.relevantSpanFrom(start),
      nullptr, local.idxs, { child });
  }
  // EO readAtRootRule

  // Consumes a query expression of the form `(foo: bar)`.
  Interpolation* StylesheetParser::readAtRootQuery()
  {
    if (scanner.peekChar() == $hash) {
      Expression* interpolation(readSingleInterpolation());
      return SASS_MEMORY_NEW(Interpolation,
        interpolation->pstate(), interpolation);
    }

    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    scanner.expectChar($lparen);
    buffer.writeCharCode($lparen);
    scanWhitespace();

    buffer.add(readExpression());
    if (scanner.scanChar($colon)) {
      scanWhitespace();
      buffer.writeCharCode($colon);
      buffer.writeCharCode($space);
      buffer.add(readExpression());
    }

    scanner.expectChar($rparen);
    scanWhitespace();
    buffer.writeCharCode($rparen);

    return buffer.getInterpolation(
      scanner.relevantSpanFrom(start));

  }

  // Consumes a `@content` rule.
  // [start] should point before the `@`.
  ContentRule* StylesheetParser::readContentRule(Offset start)
  {
    if (!inMixin) {
      error("@content is only allowed within mixin declarations.",
        scanner.relevantSpanFrom(start));
    }

    scanWhitespace();

    CallableArgumentsObj args;
    if (scanner.peekChar() == $lparen) {
      args = readArgumentInvocation(true);
    }
    else {
      args = SASS_MEMORY_NEW(CallableArguments,
        scanner.relevantSpan(), ExpressionVector(), {});
    }

    RAII_FLAG(mixinHasContent, true);
    expectStatementSeparator("@content rule");
    // ToDo: ContentRule
    return SASS_MEMORY_NEW(ContentRule,
      scanner.relevantSpanFrom(start), args);

  }
  // EO readContentRule

  // Try to parse either `to` or `through`, if successful
  // we will return `true`. The boolean passed via [inclusive]
  // will be set to `true` if we parsed `through`. We return
  // `false` if neither of the tokens could be parsed.
  bool StylesheetParser::tryForRuleOperator(bool& inclusive)
  {
    if (!lookingAtIdentifier()) return false;
    if (scanIdentifier("to")) {
      inclusive = false;
      return true;
    }
    else if (scanIdentifier("through")) {
      inclusive = true;
      return true;
    }
    else {
      return false;
    }
  }
    

  // Consumes an `@each` rule.
  // [start] should point before the `@`. [child] is called to consume any
  // children that are specifically allowed in the caller's context.
  EachRule* StylesheetParser::readEachRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    // This must be enabled to pass tests
    RAII_FLAG(inControlDirective, true);
    sass::vector<EnvKey> variables;
    EnvFrame local(compiler, true);
    variables.emplace_back(variableName());
    local.idxs->createVariable(variables.back());
    scanWhitespace();
    while (scanner.scanChar($comma)) {
      scanWhitespace();
      variables.emplace_back(variableName());
      local.idxs->createVariable(variables.back());
      scanWhitespace();
    }
    expectIdentifier("in", "\"in\"");
    scanWhitespace();
    ExpressionObj list = readExpression();
    return withChildren<EachRule>(
      child, start, variables, list, local.idxs);
  }

  ErrorRule* StylesheetParser::readErrorRule(Offset start)
  {
    ExpressionObj value = readExpression();
    expectStatementSeparator("@error rule");
    return SASS_MEMORY_NEW(ErrorRule,
      scanner.relevantSpanFrom(start), value);
  }
  // EO readErrorRule

  // Consumes an `@extend` rule.
  // [start] should point before the `@`.
  ExtendRule* StylesheetParser::readExtendRule(Offset start)
  {
    if (!inStyleRule && !inMixin && !inContentBlock) {
      error("@extend may only be used within style rules.",
        scanner.relevantSpanFrom(start));
    }

    InterpolationObj value = readAlmostAnyValue();
    bool optional = scanner.scanChar($exclamation);
    if (optional) expectIdentifier("optional", "\"optional\"");
    expectStatementSeparator("@extend rule");
    return SASS_MEMORY_NEW(ExtendRule,
      scanner.relevantSpanFrom(start), value, optional);
  }
  // EO readExtendRule

  // Returns `true` if scanner reached a `,`
  bool StylesheetParser::lookingAtForRuleContinuation()
  {
    if (!lookingAtIdentifier()) return false;
    if (scanIdentifier("to")) {
      _foundForRuleExpression = true;
      _exclusiveAtForRule = true;
      return true;
    }
    else if (scanIdentifier("through")) {
      _foundForRuleExpression = true;
      _exclusiveAtForRule = false;
      return true;
    }
    else {
      return false;
    }
  }


  ForRule* StylesheetParser::readForRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    RAII_FLAG(inControlDirective, true);
    EnvFrame local(compiler, true);
    sass::string variable = variableName();
    local.idxs->createVariable(variable);
    scanWhitespace();
    expectIdentifier("from", "\"from\"");
    scanWhitespace();
    // ExpressionObj from = readSingleExpression();
    _exclusiveAtForRule = false;
    _foundForRuleExpression = false;
    ExpressionObj from = readExpression(false, false,
      &StylesheetParser::lookingAtForRuleContinuation);
    if (!_foundForRuleExpression) {
      error("Expected \"to\" or \"through\".",
        scanner.relevantSpan());
    }

    scanWhitespace();
    ExpressionObj to = readExpression();
    auto qwe = withChildren<ForRule>(child, start,
      variable, from, to, !_exclusiveAtForRule, local.idxs);
    return qwe;
  }

  // ToDo: dart-sass stores all else ifs in the same object, smart ...
  IfRule* StylesheetParser::readIfRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    // var ifIndentation = currentIndentation;
    size_t ifIndentation = 0;
    RAII_FLAG(inControlDirective, true);
    ExpressionObj predicate = readExpression();

    IfRuleObj root;
    IfRuleObj cur;

    /* create anonymous lexical scope */ {
      EnvFrame local(compiler, true);
      StatementVector children(
        readChildren(child));
      cur = root = SASS_MEMORY_NEW(IfRule,
        scanner.relevantSpanFrom(start), local.idxs,
        std::move(children), std::move(predicate));
    }
    // EO lexical scope

    scanWhitespaceWithoutComments();

    sass::vector<IfRule*> ifs;
    ifs.push_back(root);

    while (scanElse(ifIndentation)) {
      scanWhitespace();
      // scanned a else if
      if (scanIdentifier("if")) {
        scanWhitespace();

        ExpressionObj predicate = readExpression();
        start = scanner.offset;

        EnvFrame local(compiler, true);
        StatementVector children(
          readChildren(child));
        IfRule* alternative = SASS_MEMORY_NEW(IfRule,
          scanner.relevantSpanFrom(start), local.idxs,
          std::move(children), std::move(predicate));
        cur->alternative(alternative);
        cur = alternative;
      }
      // scanned a pure else
      else {

        EnvFrame local(compiler, true);

        start = scanner.offset;
        StatementVector children(
          readChildren(child));
        IfRule* alternative = SASS_MEMORY_NEW(IfRule,
          scanner.relevantSpanFrom(start), local.idxs,
          std::move(children)); // else has no predicate
        cur->alternative(alternative);
        break;
      }
    }

    scanWhitespaceWithoutComments();

    return root.detach();

  }

  /// Parses the namespace of a `@use` rule from an `as` clause, or returns the
  /// default namespace from its URL.
  sass::string StylesheetParser::readUseNamespace(const sass::string& url, const Offset& start)
  {
    if (scanIdentifier("as")) {
      scanWhitespace();
      return scanner.scanChar($asterisk)
        ? "*" : readIdentifier();
    }

    // Check if name is valid identifier
    if (url.empty() || isDigit(url[0])) {
      SourceSpan pstate(scanner.relevantSpanFrom(start));
      callStackFrame csf(compiler, pstate);
      throw Exception::InvalidDefaultNamespace(compiler, url);
    }

    return "";
  }

  bool StylesheetParser::readWithConfiguration(
    sass::vector<WithConfigVar>& vars,
    bool allowGuarded)
  {

    if (!scanIdentifier("with")) return false;

    scanWhitespace();
    scanner.expectChar($lparen);

    std::set<EnvKey> seen;

    while (true) {
      scanWhitespace();

      Offset variableStart(scanner.offset);
      sass::string name(variableName());
      scanWhitespace();
      scanner.expectChar($colon);
      scanWhitespace();
      ExpressionObj expression = readExpressionUntilComma();

      bool guarded = false;
      Offset flagStart(scanner.offset);
      if (allowGuarded && scanner.scanChar($exclamation)) {
        sass::string flag(readIdentifier());
        if (flag == "default") {
          guarded = true;
        }
        else {
          error("Invalid flag name.",
            scanner.relevantSpanFrom(flagStart));
        }
      }

      if (seen.count(name) == 1) {
        error("The same variable may only be configured once.",
          scanner.relevantSpanFrom(variableStart));
      }

      seen.insert(name);

      WithConfigVar kvar;
      kvar.expression44 = expression;
      kvar.isGuarded41 = guarded;
      kvar.pstate = scanner.relevantSpanFrom(variableStart);
      kvar.name = name;
      vars.push_back(kvar);

      if (!scanner.scanChar($comma)) break;
      scanWhitespace();
      if (!lookingAtExpression()) break;
    }

    scanWhitespace();
    scanner.expectChar($rparen);
    return true;
  }

  void StylesheetParser::readForwardMembers(std::set<EnvKey>& variables, std::set<EnvKey>& callables)
  {
    try {
      do {
        scanWhitespace();
        if (scanner.peekChar() == $dollar) {
          variables.insert(variableName());
        }
        else {
          callables.insert(readIdentifier());
        }
        scanWhitespace();
      } while (scanner.scanChar($comma));
    }
    // ToDo: Profile how expensive this is
    catch (Exception::ParserException& err) {
      err.msg = "Expected variable, mixin, or function name";
      throw err;
    }
  }

  // Returns whether [url] indicates that an `@import` is a plain CSS import.
  bool StylesheetParser::isPlainImportUrl(const sass::string& url) const
  {
    if (url.length() < 5) return false;

    if (endsWithIgnoreCase(url, ".css", 4)) return true;

    uint8_t first = url[0];
    if (first == $slash) return url[1] == $slash;
    if (first != $h) return false;
    return startsWithIgnoreCase(url, "http://", 7)
      || startsWithIgnoreCase(url, "https://", 6);
  }

  // Consumes a sequence of modifiers (such as media or supports queries)
  // after an import argument. Returns `null` if there are no modifiers.
  Interpolation* StylesheetParser::tryImportModifiers()
  {
    if (!lookingAtInterpolatedIdentifier() && scanner.peekChar() != $lparen) {
      return nullptr;
    }

    Offset start = scanner.offset;
    InterpolationBuffer buffer(scanner);
    while (true) {
      // std::cerr << "====\n";
      if (lookingAtInterpolatedIdentifier()) {
        // std::cerr << "Looki looki\n";
        // if (!buffer.empty()) buffer.writeCharCode($space);

        InterpolationObj identifier = readInterpolatedIdentifier();
        if (identifier == nullptr) std::cerr << "IS NULL";
        // std::cerr << "got ident\n";
        if (!buffer.empty()) buffer.writeCharCode($space);
        buffer.addInterpolation(identifier);
        // std::cerr << " asddde " << identifier << "\n";

        auto name = identifier->getPlainString();

        // std::cerr << "is it " << name.c_str() << "\n";

        // convert to lower case
        if (!equalsIgnoreCase(name, "and", 3) && scanner.scanChar($lparen)) {
          // std::cerr << "is and\n";
          if (equalsIgnoreCase(name, "supports", 8)) {
            auto query = readImportSupportsQuery();
            if (!query->isaSupportsDeclaration())
              buffer.writeCharCode($lparen);
            auto foo = SASS_MEMORY_NEW(SupportsExpression,
              scanner.rawSpanFrom(start), query);
            // std::cerr << "0 ++ " << foo->toString() << "\n";
            buffer.add(foo);
            if (!query->isaSupportsDeclaration())
              buffer.writeCharCode($rparen);
          }
          else {
            buffer.writeCharCode($lparen);
            auto itpl = readInterpolatedDeclarationValue(true, true);
            // std::cerr << "1 ++ " << itpl->toString() << "\n";
            buffer.addInterpolation(itpl);
            buffer.writeCharCode($rparen);
          }

          scanner.expectChar($rparen);
          scanWhitespace();
        }
        else {
          // std::cerr << "is other\n";
          scanWhitespace();
          if (scanner.scanChar($comma)) {
            buffer.write(", ");
            buffer.addInterpolation(readMediaQueryList());
            // std::cerr << "return 2\n";
            return buffer.getInterpolation(
              scanner.relevantSpanFrom(start));
          }
        }
      }
      else {
        // std::cerr << "checki checki " << scanner.peekChar() << "\n";
        if (scanner.peekChar() == $lparen) {
          // std::cerr << "HEPPA\n";
          if (!buffer.empty()) buffer.writeCharCode($space);
          auto foo = readMediaQueryList();
          // std::cerr << "2 ++ " << foo->toString() << "\n";
          buffer.addInterpolation(foo);
          return buffer.getInterpolation(
            scanner.relevantSpanFrom(start));
        }
        else {
          // std::cerr << "return 3\n";
          return buffer.getInterpolation(
            scanner.relevantSpanFrom(start));
        }
      }
    }
    // std::cerr << "ended\n";f
  }
  // EO tryImportModifiers

  // Consumes the contents of a `supports()` function after
  // an `@import` rule (but not the function name or parentheses).
  SupportsCondition* StylesheetParser::readImportSupportsQuery() {
    if (scanIdentifier("not")) {
      scanWhitespace();
      Offset start(scanner.offset);
      StringScannerState state(scanner.state());
      return new SupportsNegation(
        scanner.rawSpanFrom(start),
        readSupportsConditionInParens());
    }
    else if (scanner.peekChar() == $lparen) {
      return readSupportsCondition();
    }
    else {
      auto function = tryImportSupportsFunction();
      if (function != nullptr) return function;

      Offset start(scanner.offset);
      StringScannerState state(scanner.state());
      auto name = readExpression();
      scanner.expectChar($colon);
      return readSupportsDeclarationValue(name, start);
    }
  }
  // EO readImportSupportsQuery

  // Consumes a function call within a `supports()`
  // function after an `@import` if available.
  SupportsFunction* StylesheetParser::tryImportSupportsFunction() {
    if (!lookingAtInterpolatedIdentifier()) return nullptr;

    Offset start(scanner.offset);
    StringScannerState state(scanner.state());
    auto name = readInterpolatedIdentifier();
    assert(name.asPlain != "not");

    if (!scanner.scanChar($lparen)) {
      scanner.backtrack(state);
      return nullptr;
    }

    auto value = readInterpolatedDeclarationValue(true, true);
    scanner.expectChar($rparen);

    return new SupportsFunction(
      scanner.relevantSpanFrom(start),
      name, value);
  }
  // EO tryImportSupportsFunction

  // Consumes a supports condition and/or a media query after an `@import`.
  std::pair<SupportsConditionObj, InterpolationObj> StylesheetParser::tryImportQueries()
  {
    SupportsConditionObj supports;
    if (scanIdentifier("supports")) {
      scanner.expectChar($lparen);
      Offset start(scanner.offset);
      if (scanIdentifier("not")) {
        scanWhitespace();
        SupportsCondition* condition = readSupportsConditionInParens();
        supports = SASS_MEMORY_NEW(SupportsNegation,
          scanner.relevantSpanFrom(start), condition);
      }
      else if (scanner.peekChar() == $lparen) {
        supports = readSupportsCondition();
      }
      else {
        Expression* name = readExpression();
        scanner.expectChar($colon);
        scanWhitespace();
        Expression* value = readExpression();
        supports = SASS_MEMORY_NEW(SupportsDeclaration,
          scanner.relevantSpanFrom(start), name, value);
      }
      scanner.expectChar($rparen);
      scanWhitespace();
    }

    InterpolationObj media;
    if (scanner.peekChar() == $lparen) {
      media = readMediaQueryList();
    }
    else if (lookingAtInterpolatedIdentifier()) {
      media = readMediaQueryList();
    }
    return std::make_pair(supports, media);
  }
  // EO tryImportQueries


  // Consumes a `@use` rule.
  // [start] should point before the `@`.
  UseRule* StylesheetParser::readUseRule(Offset start)
  {

    scanWhitespace();
    sass::string url(string());
    scanWhitespace();
    sass::string ns(readUseNamespace(url, start));
    scanWhitespace();

    SourceSpan state(scanner.relevantSpanFrom(start));

    // Check if name is valid identifier
    //if (url.empty() || isDigit(url[0])) {
    //  // don't throw if it has an "as"
    //  callStackFrame csf(compiler, state);
    //  throw Exception::InvalidSassIdentifier(compiler, url);
    //}

    sass::vector<WithConfigVar> config;
    bool hasWith(readWithConfiguration(config, false));
    expectStatementSeparator("@use rule");

    if (isUseAllowed == false) {
      callStackFrame csf(compiler, state);
      throw Exception::TardyAtRule(
        compiler, Strings::useRule);
    }

    UseRuleObj rule = SASS_MEMORY_NEW(UseRule,
      scanner.relevantSpanFrom(start),
      scanner.sourceUrl, url, {},
      wconfig, std::move(config), hasWith);

    RAII_PTR(WithConfig, wconfig, rule);

    // Support internal modules first
    if (startsWithIgnoreCase(url, "sass:", 5)) {

      if (hasWith) {
        callStackFrame csf(compiler, rule->pstate());
        throw Exception::RuntimeException(compiler,
          "Built-in modules can't be configured.");
      }

      sass::string name(url.substr(5));
      if (ns.empty()) ns = name;
      rule->ns(ns == "*" ? "" : ns);

      BuiltInMod* module(compiler.getModule(name));

      if (module == nullptr) {
        callStackFrame csf(compiler, rule->pstate());
        throw Exception::RuntimeException(compiler,
          "Invalid internal module requested.");
      }

      rule->module32(module);

      return rule.detach();
    }
    // BuiltIn

    // Deduct the namespace from url
    // After last slash before first dot
    if (ns.empty() && !url.empty()) {
      auto start = url.find_last_of("/\\");
      start = (start == NPOS ? 0 : start + 1);
      auto end = url.find_first_of(".", start);
      if (url[start] == '_') start += 1;
      ns = url.substr(start, end);
    }

    rule->ns(ns == "*" ? "" : ns);
    return rule.detach();
  }

  // Consumes a `@forward` rule.
  // [start] should point before the `@`.
  ForwardRule* StylesheetParser::readForwardRule(Offset start)
  {
    scanWhitespace();
    sass::string url = string();

    scanWhitespace();
    sass::string prefix;
    if (scanIdentifier("as")) {
      scanWhitespace();
      prefix = readIdentifier();
      scanner.expectChar($asterisk);
      scanWhitespace();
    }

    bool isShown = false;
    bool isHidden = false;
    std::set<EnvKey> varFilters;
    std::set<EnvKey> callFilters;
    // Offset beforeShow(scanner.offset);
    if (scanIdentifier("show")) {
      readForwardMembers(varFilters, callFilters);
      isShown = true;
    }
    else if (scanIdentifier("hide")) {
      readForwardMembers(varFilters, callFilters);
      isHidden = true;
    }

    sass::vector<WithConfigVar> config;
    bool hasWith(readWithConfiguration(config, true));
    // RAII_FLAG(hasWithConfig, hasWithConfig || hasWith);
    expectStatementSeparator("@forward rule");

    if (isUseAllowed == false) {
      SourceSpan state(scanner.relevantSpanFrom(start));
      callStackFrame csf(compiler, state);
      throw Exception::ParserException(compiler,
        "@forward rules must be written before any other rules.");
    }

    ForwardRuleObj rule = SASS_MEMORY_NEW(ForwardRule,
      scanner.relevantSpanFrom(start),
      scanner.sourceUrl, url, {},
      prefix, wconfig,
      std::move(varFilters),
      std::move(callFilters),
      std::move(config),
      isShown, isHidden, hasWith);

    RAII_PTR(WithConfig, wconfig, rule);

    if (startsWithIgnoreCase(url, "sass:", 5)) {

      if (hasWith) {
        callStackFrame csf(compiler, rule->pstate());
        throw Exception::RuntimeException(compiler,
          "Built-in modules can't be configured.");
      }

      sass::string name(url.substr(5));
      if (BuiltInMod* module = compiler.getModule(name)) {
        rule->module32(module);
        rule->root47(nullptr);
      }
      else {
        callStackFrame csf(compiler, rule->pstate());
        throw Exception::RuntimeException(compiler,
          "Invalid internal module requested.");
      }

    }

    return rule.detach();
  }

  // Consumes an `@include` rule.
  // [start] should point before the `@`.
  IncludeRule* StylesheetParser::readIncludeRule(Offset start)
  {

    sass::string ns;
    sass::string name = readIdentifier();
    if (scanner.scanChar($dot)) {
      ns = name;
      name = readPublicIdentifier();
    }

    scanWhitespace();
    CallableArgumentsObj arguments;
    if (scanner.peekChar() == $lparen) {
      arguments = readArgumentInvocation(true);
    }
    scanWhitespace();

    EnvFrame local(compiler, true);

    CallableSignatureObj contentArguments;
    if (scanIdentifier("using")) {
      scanWhitespace();
      contentArguments = parseArgumentDeclaration();
      scanWhitespace();
    }

    // ToDo: Add checks to allow to omit arguments fully
    if (!arguments) {
      SourceSpan pstate(scanner.relevantSpanFrom(start));
      arguments = SASS_MEMORY_NEW(CallableArguments,
        std::move(pstate), {}, {});
    }

    sass::vector<EnvRef> midxs;

    IncludeRuleObj rule = SASS_MEMORY_NEW(IncludeRule,
    scanner.relevantSpanFrom(start), name, ns, arguments);

    ContentBlockObj content;
    if (contentArguments || lookingAtChildren()) {
      RAII_FLAG(inContentBlock, true);
      // EnvFrame inner(compiler.varRoot.stack);
      if (contentArguments.isNull()) {
        // Dart-sass creates this one too
        contentArguments = SASS_MEMORY_NEW(
          CallableSignature,
          scanner.relevantSpan());
      }
      Offset start(scanner.offset);
      rule->content(withChildren<ContentBlock>(
        &StylesheetParser::readChildStatement,
        start, contentArguments, local.idxs));
    }
    else {
      expectStatementSeparator();
    }

    /*
    var span =
      scanner.rawSpanFrom(start, start).expand((content ? ? arguments).span);
    return IncludeRule(name, arguments, span,
      namespace : ns, content : content);
      */

    return rule.detach(); // mixin.detach();
  }
  // EO readIncludeRule

  // Consumes a `@media` rule.
  // [start] should point before the `@`.
  MediaRule* StylesheetParser::readMediaRule(Offset start)
  {
    // std::cerr << "read media rule\n";
    EnvFrame local(compiler, false);
    InterpolationObj query = readMediaQueryList();
    return withChildren<MediaRule>(
      &StylesheetParser::readChildStatement,
      start, query, local.idxs);
  }


  // Consumes a `@moz-document` rule. Gecko's `@-moz-document` diverges
  // from [the specification][] allows the `url-prefix` and `domain`
  // functions to omit quotation marks, contrary to the standard.
  // [the specification]: http://www.w3.org/TR/css3-conditional/
  AtRule* StylesheetParser::readMozDocumentRule(Offset start, Interpolation* name)
  {

    Offset valueStart(scanner.offset);
    InterpolationBuffer buffer(scanner);
    bool needsDeprecationWarning = false;
    EnvFrame local(compiler, true);

    while (true) {

      if (scanner.peekChar() == $hash) {
        buffer.add(readSingleInterpolation());
        needsDeprecationWarning = true;
      }
      else {


        Offset identifierStart(scanner.offset);
        sass::string identifier = this->readIdentifier();
        if (identifier == "url" || identifier == "url-prefix" || identifier == "domain") {
          Interpolation* contents = tryUrlContents(identifierStart, /* name: */ identifier);
          if (contents != nullptr) {
            buffer.addInterpolation(contents);
          }
          else {
            scanner.expectChar($lparen);
            scanWhitespace();
            StringExpressionObj argument = readInterpolatedString();
            scanner.expectChar($rparen);

            buffer.write(identifier);
            buffer.write($lparen);
            buffer.addInterpolation(argument->getAsInterpolation());
            buffer.write($rparen);
          }

          // A url-prefix with no argument, or with an empty string as an
          // argument, is not (yet) deprecated.
          sass::string trailing = buffer.trailingString();
          if (!endsWithIgnoreCase(trailing, "url-prefix()", 12) &&
            !endsWithIgnoreCase(trailing, "url-prefix('')", 14) &&
            !endsWithIgnoreCase(trailing, "url-prefix(\"\")", 14)) {
            needsDeprecationWarning = true;
          }
        }
        else if (identifier == "regexp") {
          buffer.write("regexp(");
          scanner.expectChar($lparen);
          StringExpressionObj str = readInterpolatedString();
          buffer.addInterpolation(str->getAsInterpolation());
          scanner.expectChar($rparen);
          buffer.write($rparen);
          needsDeprecationWarning = true;
        }
        else {
          error("Invalid function name.",
            scanner.relevantSpanFrom(identifierStart));
        }
      }

      scanWhitespace();
      if (!scanner.scanChar($comma)) break;

      buffer.write($comma);
      buffer.write(rawText(&StylesheetParser::scanWhitespace));

    }

    InterpolationObj value = buffer.getInterpolation(scanner.rawSpanFrom(valueStart));


    AtRule* atRule = withChildren<AtRule>(
      &StylesheetParser::readChildStatement,
      start, name, value, local.idxs, false);

    if (needsDeprecationWarning) {

      compiler.addDeprecation(
        "@-moz-document is deprecated and support will be removed from Sass in a future\n"
        "release. For details, see http://bit.ly/moz-document.",
        atRule->pstate(), Logger::WARN_MOZ_DOC);
    }

    return atRule;

  }

  // Consumes a `@return` rule.
  // [start] should point before the `@`.
  ReturnRule* StylesheetParser::readReturnRule(Offset start)
  {
    ExpressionObj value = readExpression();
    expectStatementSeparator("@return rule");
    return SASS_MEMORY_NEW(ReturnRule,
      scanner.relevantSpanFrom(start), value);
  }
  // EO readReturnRule

  // Consumes a `@supports` rule.
  // [start] should point before the `@`.
  SupportsRule* StylesheetParser::readSupportsRule(Offset start)
  {
    auto condition = readSupportsCondition();
    scanWhitespace();
    EnvFrame local(compiler, true);
    return withChildren<SupportsRule>(
      &StylesheetParser::readChildStatement,
      start, condition, local.idxs);
  }
  // EO readSupportsRule


  // Consumes a `@debug` rule.
  // [start] should point before the `@`.
  DebugRule* StylesheetParser::readDebugRule(Offset start)
  {
    ExpressionObj value(readExpression());
    expectStatementSeparator("@debug rule");
    return SASS_MEMORY_NEW(DebugRule,
      scanner.relevantSpanFrom(start), value);
  }
  // EO readDebugRule

  // Consumes a `@warn` rule.
  // [start] should point before the `@`.
  WarnRule* StylesheetParser::readWarnRule(Offset start)
  {
    ExpressionObj value(readExpression());
    expectStatementSeparator("@warn rule");
    return SASS_MEMORY_NEW(WarnRule,
      scanner.relevantSpanFrom(start), value);
  }
  // EO readWarnRule

  // Consumes a `@while` rule. [start] should  point before the `@`. [child] is called 
  // to consume any children that are specifically allowed in the caller's context.
  WhileRule* StylesheetParser::readWhileRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    RAII_FLAG(inControlDirective, true);
    EnvFrame local(compiler, true);
    ExpressionObj condition(readExpression());
    return withChildren<WhileRule>(child,
      start, condition.ptr(), local.idxs);
  }
  // EO readWhileRule

  // Consumes an at-rule that's not explicitly supported by Sass.
  // [start] should point before the `@`. [name] is the name of the at-rule.
  AtRule* StylesheetParser::readAnyAtRule(Offset start, Interpolation* name)
  {
    RAII_FLAG(inUnknownAtRule, true);
    EnvFrame local(compiler, false);

    InterpolationObj value;
    uint8_t next = scanner.peekChar();
    if (next != $exclamation && !atEndOfStatement()) {
      value = readAlmostAnyValue();
    }

    if (lookingAtChildren()) {
      return withChildren<AtRule>(
        &StylesheetParser::readChildStatement,
        start, name, value, local.idxs, false);
    }
    expectStatementSeparator();
    return SASS_MEMORY_NEW(AtRule,
      scanner.relevantSpanFrom(start),
      name, value, local.idxs, true);
  }
  // EO readAnyAtRule

    // Parse almost any value to report disallowed at-rule
  Statement* StylesheetParser::throwDisallowedAtRule(Offset start)
  {
    InterpolationObj value(readAlmostAnyValue());
    error("This at-rule is not allowed here.",
      scanner.relevantSpanFrom(start));
    return nullptr;
  }
  // EO throwDisallowedAtRule

  // Argument declaration is tricky in terms of scoping.
  // The variable before the colon is defined on the new frame.
  // But the right side is evaluated in the parent scope.
  CallableSignature* StylesheetParser::parseArgumentDeclaration()
  {

    Offset start(scanner.offset);
    scanner.expectChar($lparen);
    scanWhitespace();
    sass::vector<ArgumentObj> arguments;
    EnvKeySet named;
    sass::string restArgument;
    while (scanner.peekChar() == $dollar) {
      Offset variableStart(scanner.offset);
      sass::string name(variableName());
      EnvKey norm(name);
      scanWhitespace();

      ExpressionObj defaultValue;
      if (scanner.scanChar($colon)) {
        scanWhitespace();
        defaultValue = readExpressionUntilComma();
      }
      else if (scanner.scanChar($dot)) {
        scanner.expectChar($dot);
        scanner.expectChar($dot);
        scanWhitespace();
        restArgument = name;
        // Defer adding variable until we parsed expression
        // Just in case the same variable is mentioned again
        compiler.varRoot.stack.back()->createVariable(norm);
        break;
      }

      // Defer adding variable until we parsed expression
      // Just in case the same variable is mentioned again
      compiler.varRoot.stack.back()->createVariable(norm);

      arguments.emplace_back(SASS_MEMORY_NEW(Argument,
        scanner.relevantSpanFrom(variableStart), name, defaultValue));

      if (named.count(norm) == 1) {
        error("Duplicate argument.",
          arguments.back()->pstate());
      }
      named.insert(std::move(norm));

      if (!scanner.scanChar($comma)) break;
      scanWhitespace();
    }
    scanner.expectChar($rparen);

    return SASS_MEMORY_NEW(
      CallableSignature,
      scanner.relevantSpanFrom(start),
      std::move(arguments),
      std::move(restArgument));

  }
  // EO parseArgumentDeclaration

  // Consumes an argument invocation. If [mixin] is `true`, this is parsed 
  // as a mixin invocation. Mixin invocations don't allow the Microsoft-style
  // `=` operator at the top level, but function invocations do.
  CallableArguments* StylesheetParser::readArgumentInvocation(
    bool mixin, bool allowEmptySecondArg)
  {

    Offset start(scanner.offset);
    scanner.expectChar($lparen);
    scanWhitespace();

    ExpressionVector positional;
    // Maybe make also optional?
    ExpressionFlatMap named;
    ExpressionObj restArg;
    ExpressionObj kwdRest;
    while (lookingAtExpression()) {
      Offset start(scanner.offset);
      ExpressionObj expression = readExpressionUntilComma(!mixin);
      if (expression == nullptr) {
        error("Expected expression.",
          scanner.rawSpan());
      }
      scanWhitespace();
      VariableExpression* var = expression->isaVariableExpression();
      if (var && scanner.scanChar($colon)) {
        scanWhitespace();
        if (named.count(var->name()) == 1) {
          error("Duplicate argument.",
            expression->pstate());
        }
        auto ex = readExpressionUntilComma(!mixin);
        named[var->name()] = ex;
      }
      else if (scanner.scanChar($dot)) {
        scanner.expectChar($dot);
        scanner.expectChar($dot);
        if (restArg == nullptr) {
          restArg = expression;
        }
        else {
          kwdRest = expression;
          scanWhitespace();
          break;
        }
      }
      else if (!named.empty()) {
        // Positional before
        if (!scanner.scan("...")) {
          error("Positional arguments must"
            " come before keyword arguments.",
            scanner.spanAt(start));
        }
      }
      else {
        positional.emplace_back(expression);
      }

      scanWhitespace();
      if (!scanner.scanChar($comma)) break;
      scanWhitespace();

      if (allowEmptySecondArg &&
        positional.size() == 1 &&
        named.empty() &&
        restArg == nullptr &&
        scanner.peekChar() == $rparen)
      {
        positional.push_back(
          SASS_MEMORY_NEW(StringExpression, scanner.rawSpan(), ""));
        break;
      }

    }
    scanner.expectChar($rparen);

    return SASS_MEMORY_NEW(
      CallableArguments,
      scanner.relevantSpanFrom(start),
      std::move(positional),
      std::move(named),
      restArg, kwdRest);

  }
  // EO readArgumentInvocation

  // Consumes an expression. If [bracketList] is true, parses this expression as
  // the contents of a bracketed list. If [singleEquals] is true, allows the
  // Microsoft-style `=` operator at the top level. If [until] is passed, it's
  // called each time the expression could end and still be a valid expression.
  // When it returns `true`, this returns the expression.
  Expression* StylesheetParser::readExpression(
    bool bracketList, bool singleEquals,
    bool(StylesheetParser::* until)())
  {

    NESTING_GUARD(recursion);

    // std::cerr << "---------- PARSE EXPRESSION\n";

    if (until != nullptr && (this->*until)()) {
      SourceSpan span(scanner.rawSpan());
      error("Expected expression.", span);
    }

    // StringScannerState beforeBracket;
    Offset start(scanner.offset);
    if (bracketList) {
      // beforeBracket = scanner.position;
      scanner.expectChar($lbracket);
      scanWhitespace();

      if (scanner.scanChar($rbracket)) {
        ListExpression* list = SASS_MEMORY_NEW(ListExpression,
          scanner.relevantSpanFrom(start), SASS_UNDEF);
        list->hasBrackets(true);
        return list;
      }
    }

    ExpressionParser ep(*this);

    bool wasInParentheses = inParentheses;

    while (true) {
      scanWhitespace();
      if (until != nullptr && (this->*until)()) break;

      uint8_t next, first = scanner.peekChar();
      Offset beforeToken(scanner.offset);

      switch (first) {
      case $lparen:
        // Parenthesized numbers can't be slash-separated.
        ep.addSingleExpression(readParenthesizedExpression());
        break;

      case $lbracket:
        ep.addSingleExpression(readExpression(true));
        break;

      case $dollar:
        ep.addSingleExpression(readVariableExpression());
        break;

      case $ampersand:
        ep.addSingleExpression(readParentExpression());
        break;

      case $apos:
      case $quote:
        ep.addSingleExpression(readInterpolatedString());
        break;

      case $hash:
        ep.addSingleExpression(readHashExpression());
        break;

      case $equal:
        scanner.readChar();
        if (singleEquals && scanner.peekChar() != $equal) {
          ep.resolveSpaceExpressions();
          ep.singleEqualsOperand = ep.singleExpression;
          ep.singleExpression = {};
        }
        else {
          scanner.expectChar($equal);
          ep.addOperator(SassOperator::EQ, beforeToken);
        }
        break;

      case $exclamation:
        next = scanner.peekChar(1);
        if (next == $equal) {
          scanner.readChar();
          scanner.readChar();
          ep.addOperator(SassOperator::NEQ, beforeToken);
        }
        else if (next == $nul ||
            equalsLetterIgnoreCase($i, next) ||
          isWhitespace(next))
        {
          ep.addSingleExpression(readImportantExpression());
        }
        else {
          goto endOfLoop;
        }
        break;

      case $lt:
        scanner.readChar();
        ep.addOperator(scanner.scanChar($equal)
          ? SassOperator::LTE : SassOperator::LT, beforeToken);
        break;

      case $gt:
        scanner.readChar();
        ep.addOperator(scanner.scanChar($equal)
          ? SassOperator::GTE : SassOperator::GT, beforeToken);
        break;

      case $asterisk:
        scanner.readChar();
        ep.addOperator(SassOperator::MUL, beforeToken);
        break;

      case $plus:
        if (ep.singleExpression == nullptr) {
          ep.addSingleExpression(readUnaryOpExpression());
        }
        else {
          scanner.readChar();
          ep.addOperator(SassOperator::ADD, beforeToken);
        }
        break;

      case $minus:
        next = scanner.peekChar(1);
        if ((isDigit(next) || next == $dot) &&
          // Make sure `1-2` parses as `1 - 2`, not `1 (-2)`.
          (ep.singleExpression == nullptr ||
            isWhitespace(scanner.peekChar(-1)))) {
          ep.addSingleExpression(readNumberExpression(), true);
        }
        else if (lookingAtInterpolatedIdentifier()) {
          ep.addSingleExpression(readIdentifierLike());
        }
        else if (ep.singleExpression == nullptr) {
          ep.addSingleExpression(readUnaryOpExpression());
        }
        else {
          scanner.readChar();
          ep.addOperator(SassOperator::SUB, beforeToken);
        }
        break;

      case $slash:
        if (ep.singleExpression == nullptr) {
          ep.addSingleExpression(readUnaryOpExpression());
        }
        else {
          scanner.readChar();
          ep.addOperator(SassOperator::DIV, beforeToken);
        }
        break;

      case $percent:
        scanner.readChar();
        ep.addOperator(SassOperator::MOD, beforeToken);
        break;

      case $0:
      case $1:
      case $2:
      case $3:
      case $4:
      case $5:
      case $6:
      case $7:
      case $8:
      case $9:
        ep.addSingleExpression(readNumberExpression(), true);
        break;

      case $dot:
        if (scanner.peekChar(1) == $dot) goto endOfLoop;
        ep.addSingleExpression(readNumberExpression(), true);
        break;

      case $a:
        if (!plainCss() && scanIdentifier("and")) {
          ep.addOperator(SassOperator::AND, beforeToken);
        }
        else {
          ep.addSingleExpression(readIdentifierLike());
        }
        break;

      case $o:
        if (!plainCss() && scanIdentifier("or")) {
          ep.addOperator(SassOperator::OR, beforeToken);
        }
        else {
          ep.addSingleExpression(readIdentifierLike());
        }
        break;

      case $u:
      case $U:
        if (scanner.peekChar(1) == $plus) {
          ep.addSingleExpression(readUnicodeRange());
        }
        else {
          ep.addSingleExpression(readIdentifierLike());
        }
        break;

      case $b:
      case $c:
      case $d:
      case $e:
      case $f:
      case $g:
      case $h:
      case $i:
      case $j:
      case $k:
      case $l:
      case $m:
      case $n:
      case $p:
      case $q:
      case $r:
      case $s:
      case $t:
      case $v:
      case $w:
      case $x:
      case $y:
      case $z:
      case $A:
      case $B:
      case $C:
      case $D:
      case $E:
      case $F:
      case $G:
      case $H:
      case $I:
      case $J:
      case $K:
      case $L:
      case $M:
      case $N:
      case $O:
      case $P:
      case $Q:
      case $R:
      case $S:
      case $T:
      case $V:
      case $W:
      case $X:
      case $Y:
      case $Z:
      case $_:
      case $backslash:
        ep.addSingleExpression(readIdentifierLike());
        break;

      case $comma:
        // If we discover we're parsing a list whose first element is a
        // division operation, and we're in parentheses, re-parse outside of a
        // parent context. This ensures that `(1/2, 1)` doesn't perform division
        // on its first element.
        if (inParentheses) {
          inParentheses = false;
          if (ep.allowSlash) {
            ep.resetState();
            break;
          }
        }

        if (ep.singleExpression == nullptr) {
          SourceSpan span(scanner.rawSpan());
          error("Expected expression.", span);
        }

        ep.resolveSpaceExpressions();
        ep.commaExpressions.emplace_back(ep.singleExpression);
        scanner.readChar();
        ep.allowSlash = true;
        ep.singleExpression = {};
        break;

      default:
        if (first != $nul && first >= 0x80) {
          ep.addSingleExpression(readIdentifierLike());
          break;
        }
        else {
          goto endOfLoop;
        }
      }
    }

  endOfLoop:

    if (bracketList) scanner.expectChar($rbracket);
    if (!ep.commaExpressions.empty()) {
      ep.resolveSpaceExpressions();
      inParentheses = wasInParentheses;
      if (ep.singleExpression != nullptr) {
        ep.commaExpressions.emplace_back(ep.singleExpression);
      }
      ListExpression* list = SASS_MEMORY_NEW(ListExpression,
        scanner.relevantSpanFrom(start), SASS_COMMA);
      list->concat(std::move(ep.commaExpressions));
      list->hasBrackets(bracketList);
      //debug_ast(list);
      return list;
    }
    else if (bracketList &&
        !ep.spaceExpressions.empty() &&
        ep.singleEqualsOperand == nullptr) {
      ep.resolveOperations();
      ListExpression* list = SASS_MEMORY_NEW(ListExpression,
        scanner.relevantSpanFrom(start), SASS_SPACE);
      ep.spaceExpressions.emplace_back(ep.singleExpression);
      list->concat(std::move(ep.spaceExpressions));
      list->hasBrackets(true);
      //debug_ast(list);
      return list;
    }
    else {
      ep.resolveSpaceExpressions();
      if (bracketList) {
        ListExpression* list = SASS_MEMORY_NEW(ListExpression,
          scanner.relevantSpanFrom(start), SASS_UNDEF);
        list->append(ep.singleExpression);
        list->hasBrackets(true);
        //debug_ast(list);
        return list;
      }
      //debug_ast(ep.singleExpression);
      return ep.singleExpression.detach();
    }

  }
  // EO expression

  // Returns `true` if scanner reached a `,`
  bool StylesheetParser::lookingAtComma()
  {
    return scanner.peekChar() == $comma;
  }


  // Consumes an expression until it reaches a top-level comma.
  // If [singleEquals] is true, this will allow the
  // Microsoft-style `=` operator at the top level.
  Expression* StylesheetParser::readExpressionUntilComma(bool singleEquals)
  {
    return readExpression(false, singleEquals,
      &StylesheetParser::lookingAtComma);
  }

    
  // Consumes an expression until it reaches a top-level comma.
  // If [singleEquals] is true, this will allow the
  // Microsoft-style `=` operator at the top level.
  Expression* StylesheetParser::readSingleExpression()
  {
    NESTING_GUARD(recursion);
    uint8_t first = scanner.peekChar();
    switch (first) {
      // Note: when adding a new case, make sure it's reflected in
      // [lookingAtExpression] and [_expression].
    case $lparen:
      return readParenthesizedExpression();
    case $slash:
      return readUnaryOpExpression();
    case $dot:
      return readNumberExpression();
    case $lbracket:
      return readExpression(true);
    case $dollar:
      return readVariableExpression();
    case $ampersand:
      return readParentExpression();

    case $apos:
    case $quote:
      return readInterpolatedString();

    case $hash:
      return readHashExpression();

    case $plus:
      return readPlusExpression();

    case $minus:
      return readMinusExpression();

    case $exclamation:
      return readImportantExpression();

    case $u:
    case $U:
      if (scanner.peekChar(1) == $plus) {
        return readUnicodeRange();
      }
      else {
        return readIdentifierLike();
      }
      break;

    case $0:
    case $1:
    case $2:
    case $3:
    case $4:
    case $5:
    case $6:
    case $7:
    case $8:
    case $9:
      return readNumberExpression();
      break;

    case $a:
    case $b:
    case $c:
    case $d:
    case $e:
    case $f:
    case $g:
    case $h:
    case $i:
    case $j:
    case $k:
    case $l:
    case $m:
    case $n:
    case $o:
    case $p:
    case $q:
    case $r:
    case $s:
    case $t:
    case $v:
    case $w:
    case $x:
    case $y:
    case $z:
    case $A:
    case $B:
    case $C:
    case $D:
    case $E:
    case $F:
    case $G:
    case $H:
    case $I:
    case $J:
    case $K:
    case $L:
    case $M:
    case $N:
    case $O:
    case $P:
    case $Q:
    case $R:
    case $S:
    case $T:
    case $V:
    case $W:
    case $X:
    case $Y:
    case $Z:
    case $_:
    case $backslash:
      return readIdentifierLike();
      break;

    default:
      if (first != $nul && first >= 0x80) {
        return readIdentifierLike();
      }
      error("Expected expression.",
        scanner.rawSpan());
      return nullptr;
    }
  }
  // EO readSingleExpression

  // Consumes a parenthesized expression.
  Expression* StylesheetParser::readParenthesizedExpression()
  {
    // Expressions are only allowed within calculations, but we verify this at
    // evaluation time.
    if (plainCss()) {
      // This one is needed ...
      // error("Parentheses aren't allowed in plain CSS outside calculation.",
      //error("Parentheses aren't allowed in plain CSS.",
      //  scanner.rawSpan());
    }

    RAII_FLAG(inParentheses, true);

    Offset start(scanner.offset);
    scanner.expectChar($lparen);
    scanWhitespace();
    if (!lookingAtExpression()) {
      scanner.expectChar($rparen);
      return SASS_MEMORY_NEW(ListExpression,
        scanner.relevantSpanFrom(start),
        SASS_UNDEF);
    }

    ExpressionObj first = readExpressionUntilComma();
    if (scanner.scanChar($colon)) {
      scanWhitespace();
      return readMapExpression(first, start);
    }

    if (!scanner.scanChar($comma)) {
      scanner.expectChar($rparen);
      return SASS_MEMORY_NEW(ParenthesizedExpression,
        scanner.relevantSpanFrom(start), first);
    }
    scanWhitespace();

    ExpressionVector
      expressions = { first };

    ListExpressionObj list = SASS_MEMORY_NEW(ListExpression,
      scanner.relevantSpanFrom(start), SASS_COMMA);

    while (true) {
      if (!lookingAtExpression()) {
        break;
      }
      expressions.emplace_back(readExpressionUntilComma());
      if (!scanner.scanChar($comma)) {
        break;
      }
      list->separator(SASS_COMMA);
      scanWhitespace();
    }

    scanner.expectChar($rparen);
    list->concat(std::move(expressions));
    list->pstate(scanner.relevantSpanFrom(start));
    return list.detach();
  }
  // EO readParenthesizedExpression

  // Consumes a map expression. This expects to be called after the
  // first colon in the map, with [first] as the expression before
  // the colon and [start] the point before the opening parenthesis.
  Expression* StylesheetParser::readMapExpression(Expression* first, Offset start)
  {
    MapExpressionObj map = SASS_MEMORY_NEW(
      MapExpression, scanner.relevantSpanFrom(start));

    map->append(first);
    map->append(readExpressionUntilComma());

    while (scanner.scanChar($comma)) {
      scanWhitespace();
      if (!lookingAtExpression()) break;

      map->append(readExpressionUntilComma());
      scanner.expectChar($colon);
      scanWhitespace();
      map->append(readExpressionUntilComma());
    }

    scanner.expectChar($rparen);
    map->pstate(scanner.relevantSpanFrom(start));
    return map.detach();
  }
  // EO _map

  // Consumes an expression that starts with a `#`.
  Expression* StylesheetParser::readHashExpression()
  {
    // assert(scanner.peekChar() == $hash);
    if (scanner.peekChar(1) == $lbrace) {
      return readIdentifierLike();
    }

    Offset start(scanner.offset);
    StringScannerState state(scanner.state());
    scanner.expectChar($hash);

    uint8_t first = scanner.peekChar();
    if (first != $nul && isDigit(first)) {
      // ColorExpression
      return readColorExpression(state);
    }

    StringScannerState afterHash = scanner.state();
    InterpolationObj identifier = readInterpolatedIdentifier();
    if (isHexColor(identifier)) {
      scanner.backtrack(afterHash);
      return readColorExpression(state);
    }

    InterpolationBuffer buffer(scanner);
    buffer.write($hash);
    buffer.addInterpolation(identifier);
    SourceSpan pstate(scanner.relevantSpanFrom(start));
    return SASS_MEMORY_NEW(StringExpression,
      pstate, buffer.getInterpolation(pstate));
  }

  ColorExpression* StylesheetParser::readColorExpression(StringScannerState state)
  {

    uint8_t digit1 = readHexDigit();
    uint8_t digit2 = readHexDigit();
    uint8_t digit3 = readHexDigit();

    uint8_t red;
    uint8_t green;
    uint8_t blue;
    double alpha = 1.0;
    bool keep = true;

    if (!isHex(scanner.peekChar())) {
      red = (digit1 << 4) + digit1;
      green = (digit2 << 4) + digit2;
      blue = (digit3 << 4) + digit3;
    }
    else {
      uint8_t digit4 = readHexDigit();
      if (!isHex(scanner.peekChar())) {
        red = (digit1 << 4) + digit1;
        green = (digit2 << 4) + digit2;
        blue = (digit3 << 4) + digit3;
        uint8_t a = (digit4 << 4) + digit4;
        alpha = a / 255.0;
        keep = false;
      }
      else {
        red = (digit1 << 4) + digit2;
        green = (digit3 << 4) + digit4;
        uint8_t digit5 = readHexDigit();
        uint8_t digit6 = readHexDigit();
        blue = (digit5 << 4) + digit6;
        if (isHex(scanner.peekChar())) {
          uint8_t digit7 = readHexDigit();
          uint8_t digit8 = readHexDigit();
          uint8_t a = (digit7 << 4) + digit8;
          alpha = a / 255.0;
          keep = false;
        }
      }
    }

    SourceSpan pstate(scanner.relevantSpanFrom(state.offset));
    sass::string original(state.position, scanner.position);
    if (keep == false) original = str_empty; // reset!?
    Color* color = SASS_MEMORY_NEW(ColorRgba, pstate,
      red, green, blue, alpha, original, false);
    return SASS_MEMORY_NEW(ColorExpression, pstate, color);
  }

  // Returns whether [interpolation] is a plain
  // string that can be parsed as a hex color.
  bool StylesheetParser::isHexColor(Interpolation* interpolation) const
  {
    const sass::string& plain(interpolation->getPlainString());
    if (plain.empty()) return false;
    if (plain.length() != 3 &&
      plain.length() != 4 &&
      plain.length() != 6 &&
      plain.length() != 8)
    {
      return false;
    }
    // return plain.codeUnits.every(isHex);
    for (size_t i = 0; i < plain.length(); i++) {
      if (!isHex(plain[i])) return false;
    }
    return true;
  }
  // EO isHexColor

  // Consumes a single hexadecimal digit.
  uint8_t StylesheetParser::readHexDigit()
  {
    uint8_t chr = scanner.peekChar();
    if (chr == $nul || !isHex(chr)) {
      error("Expected hex digit.",
        scanner.relevantSpan());
    }
    return asHex(scanner.readChar());
  }
  // EO readHexDigit

  // Consumes an expression that starts with a `+`.
  Expression* StylesheetParser::readPlusExpression()
  {
    SASS_ASSERT(scanner.peekChar() == $plus,
      "plusExpression expects a plus sign");
    uint8_t next = scanner.peekChar(1);
    if (isDigit(next) || next == $dot) {
      return readNumberExpression();
    }
    else {
      return readUnaryOpExpression();
    }
  }
  // EO readPlusExpression

  // Consumes an expression that starts with a `-`.
  Expression* StylesheetParser::readMinusExpression()
  {
    SASS_ASSERT(scanner.peekChar() == $minus,
      "minusExpression expects a minus sign");
    uint8_t next = scanner.peekChar(1);
    if (isDigit(next) || next == $dot) return readNumberExpression();
    if (lookingAtInterpolatedIdentifier()) return readIdentifierLike();
    return readUnaryOpExpression();
  }
  // EO readMinusExpression

  // Consumes an `!important` expression.
  StringExpression* StylesheetParser::readImportantExpression()
  {
    SASS_ASSERT(scanner.peekChar() == $exclamation,
      "importantExpression expects an exclamation");
    Offset start(scanner.offset);
    scanner.readChar();
    scanWhitespace();
    expectIdentifier("important", "\"important\"");
    return SASS_MEMORY_NEW(StringExpression,
      scanner.relevantSpanFrom(start),
      sass::string("!important"));
  }
  // EO readImportantExpression

  // Consumes a unary operation expression.
  UnaryOpExpression* StylesheetParser::readUnaryOpExpression()
  {
    Offset start(scanner.offset);
    UnaryOpType op =
      UnaryOpType::PLUS;
    switch (scanner.readChar()) {
    case $plus:
      op = UnaryOpType::PLUS;
      break;
    case $minus:
      op = UnaryOpType::MINUS;
      break;
    case $slash:
      op = UnaryOpType::SLASH;
      break;
    default:
      error("Expected unary operator.",
        scanner.relevantSpan());
    }

    if (plainCss() && op != UnaryOpType::SLASH) {
      error("Operators aren't allowed in plain CSS.",
        scanner.relevantSpan());
    }

    scanWhitespace();
    Expression* operand = readSingleExpression();
    return SASS_MEMORY_NEW(UnaryOpExpression,
      scanner.relevantSpanFrom(start),
      op, operand);
  }
  // EO readUnaryOpExpression

  // Consumes a number expression.
  NumberExpression* StylesheetParser::readNumberExpression()
  {
    StringScannerState start = scanner.state();
    uint8_t first = scanner.peekChar();

    double sign = first == $minus ? -1 : 1;
    if (first == $plus || first == $minus) scanner.readChar();

    double number = scanner.peekChar() == $dot ? 0.0 : naturalNumber();

    // Don't complain about a dot after a number unless the number
    // starts with a dot. We don't allow a plain ".", but we need
    // to allow "1." so that "1..." will work as a rest argument.
    number += tryDecimal(scanner.position != start.position);
    number *= tryExponent();

    sass::string unit;
    if (scanner.scanChar($percent)) {
      unit = "%";
    }
    else if (lookingAtIdentifier() &&
      // Disallow units beginning with `--`.
      (scanner.peekChar() != $minus || scanner.peekChar(1) != $minus)) {
      unit = readIdentifier(true);
    }

    auto pstate(scanner.relevantSpanFrom(start.offset));
    return SASS_MEMORY_NEW(NumberExpression, pstate,
      SASS_MEMORY_NEW(Number, pstate, sign * number, unit));
  }

  /* Locale unspecific atof function. */
  double sass_strtod(const char* str)
  {
    char separator = *(localeconv()->decimal_point);
    if (separator != '.') {
      // The current locale specifies another
      // separator. convert the separator to the
      // one understood by the locale if needed
      const char* found = strchr(str, '.');
      if (found != NULL) {
        // substitution is required. perform the substitution on a exposing
        // of the string. This is slower but it is thread safe.
        char* copy = sass_copy_c_string(str);
        *(copy + (found - str)) = separator;
        double res = strtod(copy, NULL);
        free(copy);
        return res;
      }
    }

    return strtod(str, NULL);
  }

  // Consumes the decimal component of a number and returns its value, or 0
  // if there is no decimal component. If [allowTrailingDot] is `false`, this
  // will throw an error if there's a dot without any numbers following it.
  // Otherwise, it will ignore the dot without consuming it.
  double StylesheetParser::tryDecimal(bool allowTrailingDot)
  {
    // Offset start(scanner.offset);
    StringScannerState state(scanner.state());
    if (scanner.peekChar() != $dot) return 0.0;

    if (!isDigit(scanner.peekChar(1))) {
      if (allowTrailingDot) return 0.0;
      scanner.consumedChar($dot);
      error("Expected digit.",
        scanner.rawSpan());
    }

    scanner.readChar();
    while (isDigit(scanner.peekChar())) {
      scanner.readChar();
    }

    // Use built-in double parsing so that we don't accumulate
    // floating-point errors for numbers with lots of digits.
    sass::string nr(scanner.substring(state.position));
    return sass_strtod(nr.c_str());
  }
  // EO tryDecimal

  // Consumes the exponent component of a number and returns
  // its value, or 1 if there is no exponent component.
  double StylesheetParser::tryExponent()
  {
    uint8_t first = scanner.peekChar();
    if (first != $e && first != $E) return 1.0;

    uint8_t next = scanner.peekChar(1);
    if (!isDigit(next) && next != $minus && next != $plus) return 1.0;

    scanner.readChar();
    double exponentSign = next == $minus ? -1.0 : 1.0;
    if (next == $plus || next == $minus) scanner.readChar();
    if (!isDigit(scanner.peekChar())) {
      SourceSpan span(scanner.relevantSpan());
      callStackFrame frame(compiler,
        BackTrace(span));
      error(
        "Expected digit.",
        scanner.relevantSpan());
    }

    double exponent = 0.0;
    while (isDigit(scanner.peekChar())) {
      exponent *= 10.0;
      exponent += scanner.readChar() - $0;
    }

    return pow(10.0, exponentSign * exponent);
  }
  // EO tryExponent

  // Consumes a unicode range expression.
  StringExpression* StylesheetParser::readUnicodeRange()
  {


    StringScannerState state = scanner.state();
    expectIdentChar($u);
    scanner.expectChar($plus);

    size_t firstRangeLength = 0;
    while (scanCharIf(isHex)) {
      firstRangeLength++;
    }

    bool hasQuestionMark = false;
    while (scanner.scanChar($question)) {
      hasQuestionMark = true;
      firstRangeLength++;
    }

    if (firstRangeLength == 0) {
      error("Expected hex digit or \"?\".", scanner.rawSpan());
    }
    else if (firstRangeLength > 6) {
      error("Expected at most 6 digits.", scanner.rawSpanFrom(state.offset));
    }
    else if (hasQuestionMark) {
      return SASS_MEMORY_NEW(StringExpression,
        scanner.rawSpanFrom(state.offset),
        scanner.substring(state.position));
    }

    if (scanner.scanChar($minus)) {
      auto secondRangeStart = scanner.state();
      size_t secondRangeLength = 0;
      while (scanCharIf(isHex)) {
        secondRangeLength++;
      }

      if (secondRangeLength == 0) {
        error("Expected hex digit.", scanner.rawSpan());
      }
      else if (secondRangeLength > 6) {
        error("Expected at most 6 digits.", scanner.rawSpanFrom(secondRangeStart.offset));
      }
    }

    if (lookingAtInterpolatedIdentifierBody()) {
      error("Expected end of identifier.",
        scanner.relevantSpan());
    }

    return SASS_MEMORY_NEW(StringExpression,
      scanner.relevantSpanFrom(state.offset),
      scanner.substring(state.position));
  }
  // EO readUnicodeRange

  // Consumes a variable expression (only called without namespace).
  VariableExpression* StylesheetParser::readVariableExpression(bool hoist)
  {
    Offset start(scanner.offset);

    sass::string ns, name = variableName();
    if (scanner.peekChar() == $dot && scanner.peekChar(1) != $dot) {
      // Skip the dot
      scanner.readChar();
      ns = name;
      name = readPublicIdentifier();
    }

    if (plainCss()) {
      error("Sass variables aren't allowed in plain CSS.",
        scanner.relevantSpanFrom(start));
    }

    if (!ns.empty()) {
      auto pstate(scanner.relevantSpanFrom(start));
      callStackFrame csf(compiler, pstate);
      throw Exception::ParserException(compiler,
        "Variable namespaces not supported!");
    }

    VariableExpression* expression =
      SASS_MEMORY_NEW(VariableExpression,
        scanner.relevantSpanFrom(start),
        name, ns);
    return expression;

  }
  // readVariableExpression

  // Consumes a selector expression.
  SelectorExpression* StylesheetParser::readParentExpression()
  {
    if (plainCss()) {
      error("The parent selector isn't allowed in plain CSS.",
        scanner.rawSpan());
      /* ,length: 1 */
    }

    Offset start(scanner.offset);
    scanner.expectChar($ampersand);

    if (scanner.scanChar($ampersand)) {
      compiler.addWarning(
        "In Sass, \"&&\" means two copies of the parent selector. You "
        "probably want to use \"and\" instead.",
        scanner.relevantSpanFrom(start),
        Logger::WARN_DOUBLE_PARENT);
      scanner.offset.column -= 1;
      scanner.position -= 1;
    }

    return SASS_MEMORY_NEW(SelectorExpression,
      scanner.relevantSpanFrom(start));
  }
  // readParentExpression

  // Consumes a quoted string expression.
  StringExpression* StylesheetParser::readInterpolatedString()
  {
    // NOTE: this logic is largely duplicated in ScssParser.readInterpolatedString.
    // Most changes here should be mirrored there.

    Offset start(scanner.offset);
    uint8_t quote = scanner.readChar();
    uint8_t next = 0, second = 0;

    if (quote != $apos && quote != $quote) {
      error("Expected string.",
        scanner.relevantSpanFrom(start));
    }

    InterpolationBuffer buffer(scanner);
    while (true) {
      if (!scanner.peekChar(next)) {
        break;
      }
      if (next == quote) {
        scanner.readChar();
        break;
      }
      else if (next == $nul || isNewline(next)) {
        sass::sstream strm;
        strm << "Expected " << quote << ".";
        error(strm.str(),
          scanner.relevantSpan());
      }
      else if (next == $backslash) {
        if (!scanner.peekChar(second, 1)) {
          break;
        }
        if (isNewline(second)) {
          scanner.readChar();
          scanner.readChar();
          if (second == $cr) scanner.scanChar($lf);
        }
        else {
          buffer.writeCharCode(escapeCharacter());
        }
      }
      else if (next == $hash) {
        if (scanner.peekChar(1) == $lbrace) {
          buffer.add(readSingleInterpolation());
        }
        else {
          buffer.write(scanner.readChar());
        }
      }
      else {
        buffer.write(scanner.readChar());
      }
    }

    SourceSpan pstate(scanner.relevantSpanFrom(start));
    Interpolation* itpl(buffer.getInterpolation(pstate));
    return SASS_MEMORY_NEW(StringExpression, pstate, itpl, true);
  }
  // EO readInterpolatedString

  // Consumes an expression that starts like an identifier.
  Expression* StylesheetParser::readIdentifierLike()
  {
    Offset start(scanner.offset);
    InterpolationObj identifier = readInterpolatedIdentifier();
    sass::string plain(identifier->getPlainString());
    // std::cerr << "readIdentifierLike " << plain << "\n";

    if (!plain.empty()) {
      if (plain == "if" && scanner.peekChar() == $lparen) {
        CallableArguments* invocation = readArgumentInvocation();
        return SASS_MEMORY_NEW(IfExpression,
          invocation->pstate(), invocation);
      }
      else if (plain == "not") {
        scanWhitespace();
        Expression* expression = readSingleExpression();
        return SASS_MEMORY_NEW(UnaryOpExpression,
          scanner.relevantSpanFrom(start),
          UnaryOpType::NOT,
          expression);
      }

      if (scanner.peekChar() != $lparen) {
        if (plain == "false") {
          auto pstate(scanner.relevantSpanFrom(start));
          return SASS_MEMORY_NEW(BooleanExpression, pstate,
            SASS_MEMORY_NEW(Boolean, pstate, false));
        }
        else if (plain == "true") {
          auto pstate(scanner.relevantSpanFrom(start));
          return SASS_MEMORY_NEW(BooleanExpression, pstate,
            SASS_MEMORY_NEW(Boolean, pstate, true));
        }
        else if (plain == "null") {
          auto pstate(scanner.relevantSpanFrom(start));
          return SASS_MEMORY_NEW(NullExpression, pstate,
            SASS_MEMORY_NEW(Null, pstate));
        }

        if (const ColorRgba* color = name_to_color(plain)) {
          // ToDo: can we avoid this copy here?
          ColorRgba* copy = SASS_MEMORY_COPY(color);
          copy->pstate(identifier->pstate());
          copy->disp(plain);
          return SASS_MEMORY_NEW(ColorExpression,
            copy->pstate(), copy);
        }
      }

      auto specialFunction = trySpecialFunction(plain, start);
      if (specialFunction != nullptr) {
        // std::cerr << "is special function\n";
        return specialFunction;
      }
    }

    sass::string ns;
    Offset beforeName(scanner.offset);
    uint8_t next = scanner.peekChar();
    // std::cerr << "next char is " << next << "\n";
    if (next == $dot) {

      if (scanner.peekChar(1) == $dot) {
        return SASS_MEMORY_NEW(StringExpression,
          scanner.relevantSpanFrom(beforeName), identifier);
      }
      scanner.readChar();

      if (scanner.peekChar() == $dollar) {
        sass::string name(variableName());

        sass::vector<EnvRef> vidxs;

        VariableExpressionObj expression = SASS_MEMORY_NEW(VariableExpression,
          scanner.relevantSpanFrom(start), name, plain);

        if (isPrivate(name)) {
          callStackFrame csf(compiler, expression->pstate());
          throw Exception::ParserException(compiler,
            "Private members can't be accessed "
            "from outside their modules.");
        }

        return expression.detach();
      }

      ns = identifier->getPlainString();
      beforeName = scanner.offset;

      Offset before(scanner.offset);
      StringObj ident(SASS_MEMORY_NEW(String,
        scanner.relevantSpanFrom(before),
        readPublicIdentifier()));

      InterpolationObj itpl = SASS_MEMORY_NEW(Interpolation,
        ident->pstate(), ident);

      if (ns.empty()) {
        error("Interpolation isn't allowed in namespaces.",
          scanner.relevantSpanFrom(start));
      }

      CallableArguments* args = readArgumentInvocation();
      sass::string name(identifier->getPlainString());

      // Plain Css as it's interpolated
      if (identifier->getPlainString().empty()) {
        return SASS_MEMORY_NEW(ItplFnExpression,
          scanner.relevantSpanFrom(start), itpl, args, ns);
      }

      return SASS_MEMORY_NEW(FunctionExpression,
        scanner.relevantSpanFrom(start),
        itpl->getPlainString(),
        args, name);
    }
    else if (next == $lparen) {

      // Plain Css as it's interpolated
      if (identifier->getPlainString().empty()) {
        // std::cerr << "Create a itpl fn expression\n";
        CallableArguments* args = readArgumentInvocation();
        return SASS_MEMORY_NEW(ItplFnExpression,
          scanner.relevantSpanFrom(start), identifier, args, ns);
      }

      CallableArguments* args = readArgumentInvocation(false,
        StringUtils::equalsIgnoreCase(plain, "var", 3));
      FunctionExpressionObj fn = SASS_MEMORY_NEW(FunctionExpression,
        scanner.relevantSpanFrom(start), plain, args, ns);
      // sass::string name(identifier->getPlainString());
      return fn.detach();
    }
    else {
      return SASS_MEMORY_NEW(StringExpression,
        identifier->pstate(), identifier);
    }

  }
  // readIdentifierLike

  // If [name] is the name of a function with special syntax, consumes it.
  // Otherwise, returns `null`. [start] is the location before the beginning of [name].
  StringExpression* StylesheetParser::trySpecialFunction(sass::string name, const Offset& start)
  {
    uint8_t next = 0;
    makeLowerCase(name);
    InterpolationBuffer buffer(scanner);
    sass::string normalized(StringUtils::unvendor(name));

    if (normalized == "element" || normalized == "expression"
        || (normalized == "calc" && normalized != name)) {
      if (!scanner.scanChar($lparen)) return nullptr;
      buffer.write(name);
      buffer.write($lparen);
    }
    /*
    else if (normalized == "min" || normalized == "max") {
      // min() and max() are parsed as the plain CSS mathematical functions if
      // possible, and otherwise are parsed as normal Sass functions.
      StringScannerState beginningOfContents = scanner.state();
      if (!scanner.scanChar($lparen)) return nullptr;
      scanWhitespace();

      buffer.write(name);
      buffer.write($lparen);

      if (!tryMinMaxContents(buffer)) {
        scanner.backtrack(beginningOfContents);
        return nullptr;
      }

      SourceSpan pstate(scanner.relevantSpanFrom(start));
      return SASS_MEMORY_NEW(StringExpression,
        pstate, buffer.getInterpolation(pstate));
    }
    */
    else if (normalized == "progid") {
      if (!scanner.scanChar($colon)) return nullptr;
      buffer.write(name);
      buffer.write($colon);
      while (scanner.peekChar(next) &&
          (isAlphabetic(next) || next == $dot)) {
        buffer.write(scanner.readChar());
      }
      scanner.expectChar($lparen);
      buffer.write($lparen);
    }
    else if (normalized == "url") {
      InterpolationObj contents = tryUrlContents(start);
      if (contents == nullptr) return nullptr;
      return SASS_MEMORY_NEW(StringExpression,
        scanner.relevantSpanFrom(start), contents);
    }
    /*
    else if (normalized == "clamp") {
      // Vendor-prefixed clamp() functions aren't parsed specially, because
      // no browser has ever supported clamp() with a prefix.
      if (name != "clamp") return nullptr;
      if (!scanner.scanChar($lparen)) return nullptr;
      buffer.write(name);
      buffer.write($lparen);
    }
    */
    else {
      return nullptr;
    }

    buffer.addInterpolation(readInterpolatedDeclarationValue(true));
    scanner.expectChar($rparen);
    buffer.write($rparen);

    SourceSpan pstate(scanner.relevantSpanFrom(start));
    return SASS_MEMORY_NEW(StringExpression,
      pstate, buffer.getInterpolation(pstate));
  }
  // trySpecialFunction

  /*
  // Consumes the contents of a plain-CSS `min()` or `max()` function into
  // [buffer] if one is available. Returns whether this succeeded. If [allowComma]
  // is `true` (the default), this allows `CalcValue` productions separated by commas.
  bool StylesheetParser::tryMinMaxContents(InterpolationBuffer& buffer, bool allowComma)
  {
    uint8_t next = 0;
    // The number of open parentheses that need to be closed.
    while (true) {
      if (!scanner.peekChar(next)) {
        return false;
      }
      switch (next) {
      case $minus:
      case $plus:
      case $0:
      case $1:
      case $2:
      case $3:
      case $4:
      case $5:
      case $6:
      case $7:
      case $8:
      case $9:
        buffer.add(readNumberExpression());
        break;

      case $hash:
        if (scanner.peekChar(1) != $lbrace) return false;
        buffer.add(readSingleInterpolation());
        break;

      case $c:
      case $C:
        switch (scanner.peekChar(1)) {
        case $a:
        case $A:
          if (!tryMinMaxFunction(buffer, "calc")) return false;
          break;

        case $l:
        case $L:
          if (!tryMinMaxFunction(buffer, "clamp")) return false;
          break;
        }
        break;

      case $e:
      case $E:
        if (!tryMinMaxFunction(buffer, "env")) return false;
        break;

      case $v:
      case $V:
        if (!tryMinMaxFunction(buffer, "var")) return false;
        break;

      case $lparen:
        buffer.write(scanner.readChar());
        if (!tryMinMaxContents(buffer, false)) return false;
        break;

      case $m:
      case $M:
        scanner.readChar();
        if (scanIdentChar($i)) {
          if (!scanIdentChar($n)) return false;
          buffer.write("min(");
        }
        else if (scanIdentChar($a)) {
          if (!scanIdentChar($x)) return false;
          buffer.write("max(");
        }
        else {
          return false;
        }
        if (!scanner.scanChar($lparen)) return false;

        if (!tryMinMaxContents(buffer)) return false;
        break;

      default:
        return false;
      }

      scanWhitespace();

      next = scanner.peekChar();
      switch (next) {
      case $rparen:
        buffer.write(scanner.readChar());
        return true;

      case $plus:
      case $minus:
      case $asterisk:
      case $slash:
        buffer.write($space);
        buffer.write(scanner.readChar());
        buffer.write($space);
        break;

      case $comma:
        if (!allowComma) return false;
        buffer.write(scanner.readChar());
        buffer.write($space);
        break;

      default:
        return false;
      }

      scanWhitespace();
    }
  }
  */
  // EO tryMinMaxContents

  // Consumes a function named [name] containing an
  // `InterpolatedDeclarationValue` if possible, and
  // adds its text to [buffer]. Returns whether such a
  // function could be consumed.
  /*
  bool StylesheetParser::tryMinMaxFunction(InterpolationBuffer& buffer, sass::string name)
  {
    if (!scanIdentifier(name)) return false;
    if (!scanner.scanChar($lparen)) return false;
    buffer.write(name);
    buffer.write($lparen);
    buffer.addInterpolation(readInterpolatedDeclarationValue(true));
    buffer.write($rparen);
    if (!scanner.scanChar($rparen)) return false;
    return true;
  }
  */
  // tryMinMaxFunction

  // Like [_urlContents], but returns `null` if the URL fails to parse.
  // [start] is the position before the beginning of the name.
  // [name] is the function's name; it defaults to `"url"`.
  Interpolation* StylesheetParser::tryUrlContents(const Offset& start, sass::string name)
  {
    // NOTE: this logic is largely duplicated in Parser.tryUrl.
    // Most changes here should be mirrored there.
    StringScannerState beginningOfContents = scanner.state();
    if (!scanner.scanChar($lparen)) return nullptr;
    scanWhitespaceWithoutComments();

    // Match Ruby Sass's behavior: parse a raw URL() if possible, and if not
    // backtrack and re-parse as a function expression.
    InterpolationBuffer buffer(scanner);
    buffer.write(name.empty() ? "url" : name);
    buffer.write($lparen);
    while (true) {
      uint8_t next = scanner.peekChar();
      if (next == $nul) {
        break;
      }
      else if (next == $backslash) {
        escape(buffer.text);
      }
      else if (next == $hash && scanner.peekChar(1) == $lbrace) {
        buffer.add(readSingleInterpolation());
      }
      else if (next == $exclamation ||
        next == $percent ||
        next == $ampersand ||
        next == $hash ||
        (next >= $asterisk && next <= $tilde) ||
          next >= 0x0080) {
        buffer.write(scanner.readChar());
      }
      else if (isWhitespace(next)) {
        scanWhitespaceWithoutComments();
        if (scanner.peekChar() != $rparen) break;
      }
      else if (next == $rparen) {
        buffer.write(scanner.readChar());
        return buffer.getInterpolation(scanner.relevantSpanFrom(start));
      }
      else {
        break;
      }
    }

    scanner.backtrack(beginningOfContents);
    return nullptr;

  }
  // tryUrlContents

  // Consumes a [url] token that's allowed to contain SassScript.
  // Returns either a  `StringExpression` or a `FunctionExpression`
  Expression* StylesheetParser::readFunctionOrStringExpression()
  {
    Offset start(scanner.offset);
    expectIdentifier("url", "\"url\"");
    String* fnName = SASS_MEMORY_NEW(String,
      scanner.relevantSpanFrom(start), "url");
    InterpolationObj itpl = SASS_MEMORY_NEW(Interpolation,
      scanner.relevantSpanFrom(start), fnName);
    InterpolationObj contents = tryUrlContents(start);
    if (contents != nullptr) {
      return SASS_MEMORY_NEW(StringExpression,
        scanner.relevantSpanFrom(start), contents);
    }

    SourceSpan pstate(scanner.relevantSpanFrom(start));
    CallableArguments* args = readArgumentInvocation();

    // Plain Css as it's interpolated
    if (itpl->getPlainString().empty()) {
      return SASS_MEMORY_NEW(ItplFnExpression,
        scanner.relevantSpanFrom(start), itpl, args, "");
    }

    return SASS_MEMORY_NEW(FunctionExpression,
      pstate, itpl->getPlainString(), args, "");
  }
  // readFunctionOrStringExpression

  // Consumes tokens up to "{", "}", ";", or "!".
  // This respects string and comment boundaries and supports interpolation.
  // Once this interpolation is evaluated, it's expected to be re-parsed.
  // Differences from [parseInterpolatedDeclarationValue] include:
  // * This does not balance brackets.
  // * This does not interpret backslashes, since
  //   the text is expected to be re-parsed.
  // * This supports Sass-style single-line comments.
  // * This does not compress adjacent whitespace characters.
  Interpolation* StylesheetParser::readAlmostAnyValue(bool omitComments)
  {
    // const char* start = scanner.position;
    InterpolationBuffer buffer(scanner);
    const char* commentStart;
    StringExpressionObj strex;
    StringScannerState start = scanner.state();
    Interpolation* contents;
    uint8_t next = 0;

    while (true) {
      if (!scanner.peekChar(next)) {
        goto endOfLoop;
      }
      switch (next) {
      case $backslash:
        // Write a literal backslash because this text will be re-parsed.
        buffer.write(scanner.readChar());
        // ToDo: Check unexpected file end here?
        buffer.write(scanner.readChar());
        break;

      case $apos:
      case $quote:
        strex = readInterpolatedString();
        buffer.addInterpolation(strex->getAsInterpolation());
        break;

      case $slash:
        commentStart = scanner.position;
        if (scanComment()) {
          if (!omitComments) buffer.write(scanner.substring(commentStart));
        }
        else {
          buffer.write(scanner.readChar());
        }
        break;

      case $hash:
        if (scanner.peekChar(1) == $lbrace) {
          // Add a full interpolated identifier to handle cases like
          // "#{...}--1", since "--1" isn't a valid identifier on its own.
          auto foo = readInterpolatedIdentifier();
          buffer.addInterpolation(foo);
        }
        else {
          buffer.write(scanner.readChar());
        }
        break;

      case $cr:
      case $lf:
      case $ff:
        if (isIndented()) goto endOfLoop;
        buffer.write(scanner.readChar());
        break;

      case $exclamation:
      case $semicolon:
      case $lbrace:
      case $rbrace:
        goto endOfLoop;

      case $u:
      case $U:
        start = scanner.state();
        if (!scanIdentifier("url")) {
          buffer.write(scanner.readChar());
          break;
        }
        contents = tryUrlContents(start.offset);
        if (contents == nullptr) {
          scanner.backtrack(start);
          buffer.write(scanner.readChar());
        }
        else {
          buffer.addInterpolation(contents);
        }
        break;

      default:
        if (lookingAtIdentifier()) {
          buffer.write(readIdentifier());
        }
        else {
          buffer.write(scanner.readChar());
        }
        break;
      }
    }

  endOfLoop:
    // scanner.relevant
    // scanner.backtrack(scanner.relevant);
    // scanWhitespaceWithoutComments(); // consume trailing white-space
    return buffer.getInterpolation(scanner.rawSpanFrom(start.offset), false);

  }
  // readAlmostAnyValue

  // Consumes tokens until it reaches a top-level `";"`, `")"`, `"]"`, or `"}"` and returns
  // their contents as a string. If [allowEmpty] is `false` (the default), this requires
  // at least one token. If [allowSemicolon] is `true`, this doesn't stop at semicolons
  // and instead includes them in the interpolated output. If [allowColon] is `false`,
  // this stops at top-level colons.Unlike [declarationValue], this allows interpolation.
  Interpolation* StylesheetParser::readInterpolatedDeclarationValue(bool allowEmpty, bool allowSemicolon, bool allowColon)
  {
    // NOTE: this logic is largely duplicated in Parser.declarationValue and
    // isIdentifier in utils.dart. Most changes here should be mirrored there.
    StringScannerState beforeUrl = scanner.state();
    Interpolation* contents;

    InterpolationBuffer buffer(scanner);
    Offset start(scanner.offset);
    sass::vector<uint8_t> brackets;
    bool wroteNewline = false;
    uint8_t next = 0;

    InterpolationObj itpl;
    StringExpressionObj strex;

    while (true) {
      if (!scanner.peekChar(next)) {
        goto endOfLoop;
      }
      switch (next) {
      case $backslash:
        escape(buffer.text, true);
        wroteNewline = false;
        break;

      case $apos:
      case $quote:
        strex = readInterpolatedString();
        itpl = strex->getAsInterpolation();
        buffer.addInterpolation(itpl);
        wroteNewline = false;
        break;

      case $slash:
        if (scanner.peekChar(1) == $asterisk) {
          buffer.write(rawText(&StylesheetParser::scanLoudComment));
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;

      case $hash:
        if (scanner.peekChar(1) == $lbrace) {
          // Add a full interpolated identifier to handle cases like
          // "#{...}--1", since "--1" isn't a valid identifier on its own.
          itpl = readInterpolatedIdentifier();
          buffer.addInterpolation(itpl);
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;

      case $space:
      case $tab:
        if (wroteNewline || !isWhitespace(scanner.peekChar(1))) {
          buffer.write(scanner.readChar());
        }
        else {
          scanner.readChar();
        }
        break;

      case $lf:
      case $cr:
      case $ff:
        if (isIndented()) goto endOfLoop;
        if (!isNewline(scanner.peekChar(-1))) {
          buffer.write("\n");
        }
        scanner.readChar();
        wroteNewline = true;
        break;

      case $lparen:
      case $lbrace:
      case $lbracket:
        buffer.write(next);
        brackets.emplace_back(opposite(scanner.readChar()));
        wroteNewline = false;
        break;

      case $rparen:
      case $rbrace:
      case $rbracket:
        if (brackets.empty()) goto endOfLoop;
        buffer.write(next);
        scanner.expectChar(brackets.back());
        brackets.pop_back();
        wroteNewline = false;
        break;

      case $semicolon:
        if (!allowSemicolon && brackets.empty()) goto endOfLoop;
        buffer.write(scanner.readChar());
        wroteNewline = false;
        break;

      case $colon:
        if (!allowColon && brackets.empty()) goto endOfLoop;
        buffer.writeCharCode(scanner.readChar());
        wroteNewline = false;
      break;

      case $u:
      case $U:
        beforeUrl = scanner.state();
        if (!scanIdentifier("url")) {
          buffer.write(scanner.readChar());
          wroteNewline = false;
          break;
        }

        contents = tryUrlContents(beforeUrl.offset);
        if (contents == nullptr) {
          scanner.backtrack(beforeUrl);
          buffer.write(scanner.readChar());
        }
        else {
          buffer.addInterpolation(contents);
        }
        wroteNewline = false;
        break;

      default:
        if (next == $nul) goto endOfLoop;

        if (lookingAtIdentifier()) {
          buffer.write(readIdentifier());
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;
      }
    }

  endOfLoop:

    if (!brackets.empty()) scanner.expectChar(brackets.back());
    if (!allowEmpty && buffer.empty()) {
      error("Expected token.",
        scanner.relevantSpan());
    }
    SourceSpan pstate(scanner.rawSpanFrom(start));
    return buffer.getInterpolation(pstate);

  }
  // parseInterpolatedDeclarationValue

  // Consumes an identifier that may contain interpolation.
  Interpolation* StylesheetParser::readInterpolatedIdentifier()
  {
    InterpolationBuffer buffer(scanner);
    Offset start(scanner.offset);

    if (scanner.scanChar($minus)) {
      buffer.writeCharCode($minus);
      if (scanner.scanChar($minus)) {
        buffer.writeCharCode($minus);
        consumeInterpolatedIdentifierBody(buffer);
        return buffer.getInterpolation(scanner.relevantSpanFrom(start));
      }
    }

    uint8_t first = 0; // , next = 0;
    if (!scanner.peekChar(first)) {
      error("Expected identifier.",
        scanner.relevantSpanFrom(start));
    }
    else if (isNameStart(first)) {
      buffer.write(scanner.readChar());
    }
    else if (first == $backslash) {
      escape(buffer.text, true);
    }
    else if (first == $hash && scanner.peekChar(1) == $lbrace) {
      ExpressionObj ex = readSingleInterpolation();
      buffer.add(ex.ptr());
    }
    else {
      error("Expected identifier.",
        scanner.relevantSpan());
    }

    consumeInterpolatedIdentifierBody(buffer);
    return buffer.getInterpolation(scanner.relevantSpanFrom(start));

  }

  void StylesheetParser::consumeInterpolatedIdentifierBody(InterpolationBuffer& buffer)
  {

    uint8_t /* first = 0, */ next = 0;
    while (true) {
      if (!scanner.peekChar(next)) {
        break;
      }
      else if (next == $underscore ||
          next == $minus ||
          isAlphanumeric(next) ||
          next >= 0x0080) {
        buffer.write(scanner.readChar());
      }
      else if (next == $backslash) {
        escape(buffer.text);
      }
      else if (next == $hash && scanner.peekChar(1) == $lbrace) {
        buffer.add(readSingleInterpolation());
      }
      else {
        break;
      }
    }

  }
  // readInterpolatedIdentifier

  // Consumes interpolation.
  Expression* StylesheetParser::readSingleInterpolation()
  {
    Offset start(scanner.offset);
    scanner.expect("#{");
    scanWhitespace();
    ExpressionObj contents(readExpression());
    scanner.expectChar($rbrace);

    if (plainCss()) {
      error(
        "Interpolation isn't allowed in plain CSS.",
        scanner.rawSpanFrom(start));
    }

    return contents.detach();
  }
  // readSingleInterpolation

  // Consumes a list of media queries.
  Interpolation* StylesheetParser::readMediaQueryList()
  {
    //std::cerr << "read media query list\n";
    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    while (true) {
      scanWhitespace();
      readMediaQuery(buffer);
      scanWhitespace();
      if (!scanner.scanChar($comma)) break;
      buffer.write($comma);
      buffer.write($space);
    }
    return buffer.getInterpolation(scanner.relevantSpanFrom(start));
  }
  // readMediaQueryList


  // Consumes one or more `MediaOrInterp` expressions
  // separated by [operator] and writes them to [buffer].
  void StylesheetParser::readMediaLogicSequence(
    InterpolationBuffer& buffer, sass::string op)
  {
    //std::cerr << "read media logic sequence\n";
    while (true) {
      readMediaOrInterp(buffer);
      scanWhitespace();

      if (!scanIdentifier(op)) return;
      expectWhitespace();

      buffer.writeCharCode($space);
      buffer.write(op);
      buffer.writeCharCode($space);
    }
  }


  // Consumes a `MediaOrInterp` expression and writes it to [buffer].
  void StylesheetParser::readMediaOrInterp(InterpolationBuffer& buffer)
  {
    if (scanner.peekChar() == $hash) {
      Expression* interpolation = readSingleInterpolation();
      Interpolation* itpl = SASS_MEMORY_NEW(Interpolation, interpolation->pstate());
      itpl->append(interpolation);
      buffer.addInterpolation(itpl);
    }
    else {
      readMediaInParens(buffer);
    }
  }

  // Consumes a `MediaInParens` expression and writes it to [buffer].
  void StylesheetParser::readMediaInParens(InterpolationBuffer& buffer)
  {
    // std::cerr << "read media in parens\n";
    scanner.expectChar($lparen, "media condition in parentheses");
    buffer.writeCharCode($lparen);
    scanWhitespace();

    if (scanner.peekChar() == $lparen) {
      readMediaInParens(buffer);
      scanWhitespace();
      if (scanIdentifier("and")) {
        buffer.write(" and ");
        expectWhitespace();
        readMediaLogicSequence(buffer, "and");
      }
      else if (scanIdentifier("or")) {
        buffer.write(" or ");
        expectWhitespace();
        readMediaLogicSequence(buffer, "or");
      }
    }
    else if (scanIdentifier("not")) {
      buffer.write("not ");
      expectWhitespace();
      readMediaOrInterp(buffer);
    }
    else {
      buffer.add(readExpressionUntilComparison());
      if (scanner.scanChar($colon)) {
        scanWhitespace();
        buffer.writeCharCode($colon);
        buffer.writeCharCode($space);
        buffer.add(readExpression());
      }
      else {
        auto next = scanner.peekChar();
        if (next == $lt || next == $gt || next == $equal) {
          buffer.writeCharCode($space);
          buffer.writeCharCode(scanner.readChar());
          if (next == $lt || next == $gt) {
            if (scanner.scanChar($equal)) {
              buffer.writeCharCode($equal);
            }
          }
          buffer.writeCharCode($space);

          scanWhitespace();
          buffer.add(readExpressionUntilComparison());

          // dart-lang/sdk#45356
          if (next == $lt || next == $gt) {
            if (scanner.scanChar(next)) {
              buffer.writeCharCode($space);
              buffer.writeCharCode(next);
              if (scanner.scanChar($equal))
                buffer.writeCharCode($equal);
              buffer.writeCharCode($space);

              scanWhitespace();
              buffer.add(readExpressionUntilComparison());
            }
          }
        }
      }
    }
  //  std::cerr << "buffer " << buffer.text << "\n";
    scanner.expectChar($rparen);
    scanWhitespace();
    buffer.writeCharCode($rparen);
  }


  // Consumes a single media query and appends it to [buffer].
  void StylesheetParser::readMediaQuery(InterpolationBuffer& buffer)
  {
    //std::cerr << "read media query\n";

    if (scanner.peekChar() == $lparen) {
      readMediaInParens(buffer);
      scanWhitespace();
      if (scanIdentifier("and")) {
        buffer.write(" and ");
        expectWhitespace();
        readMediaLogicSequence(buffer, "and");
      }
      else if (scanIdentifier("or")) {
        buffer.write(" or ");
        expectWhitespace();
        readMediaLogicSequence(buffer, "or");
      }
      return;
    }

    auto identifier1 = readInterpolatedIdentifier();
    if (equalsIgnoreCase(identifier1->getPlainString(), "not")) {
      // For example, "@media not (...) {"
      expectWhitespace();

      if (!lookingAtInterpolatedIdentifier()) {
        buffer.write("not ");
        readMediaOrInterp(buffer);
        return;
      }
    }

    scanWhitespace();
    buffer.addInterpolation(identifier1);
    if (!lookingAtInterpolatedIdentifier()) {
      // For example, "@media screen {".
      return;
    }

    buffer.writeCharCode($space);
    auto identifier2 = readInterpolatedIdentifier();

    if (equalsIgnoreCase(identifier2->getPlainString(), "and")) {
      expectWhitespace();
      // For example, "@media screen and ..."
      buffer.write(" and ");
    }
    else {
      scanWhitespace();
      buffer.addInterpolation(identifier2);
      if (scanIdentifier("and")) {
        // For example, "@media only screen and ..."
        expectWhitespace();
        buffer.write(" and ");
      }
      else {
        // For example, "@media only screen {"
        return;
      }
    }

    // We've consumed either `IDENTIFIER "and"` or
    // `IDENTIFIER IDENTIFIER "and"`.

    if (scanIdentifier("not")) {
      // For example, "@media screen and not (...) {"
      expectWhitespace();
      buffer.write("not ");
      readMediaOrInterp(buffer);
      return;
    }

    readMediaLogicSequence(buffer, "and");
  }
  // readMediaQuery

  // Consumes a media query feature.
  Interpolation* StylesheetParser::readMediaFeature()
  {
    if (scanner.peekChar() == $hash) {
      Expression* interpolation = readSingleInterpolation();
      Interpolation* itpl = SASS_MEMORY_NEW(Interpolation,
        interpolation->pstate());
      itpl->append(interpolation);
      return itpl;
    }

    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    scanner.expectChar($lparen);
    buffer.write($lparen);
    scanWhitespace();

    buffer.add(readExpressionUntilComparison());
    if (scanner.scanChar($colon)) {
      scanWhitespace();
      buffer.write($colon);
      buffer.write($space);
      buffer.add(readExpression());
    }
    else {
      uint8_t next = scanner.peekChar();
      bool isAngle = next == $lt || next == $gt;
      if (isAngle || next == $equal) {
        buffer.write($space);
        buffer.write(scanner.readChar());
        if (isAngle && scanner.scanChar($equal)) {
          buffer.write($equal);
        }
        buffer.write($space);

        scanWhitespace();
        buffer.add(readExpressionUntilComparison());

        if (isAngle && scanner.scanChar(next)) {
          buffer.write($space);
          buffer.write(next);
          if (scanner.scanChar($equal)) {
            buffer.write($equal);
          }
          buffer.write($space);

          scanWhitespace();
          buffer.add(readExpressionUntilComparison());
        }
      }
    }

    scanner.expectChar($rparen);
    scanWhitespace();
    buffer.write($rparen);

    return buffer.getInterpolation(scanner.relevantSpanFrom(start));

  }
  // readMediaFeature

  // Helper function for until condition
  bool StylesheetParser::lookingAtExpressionEnd()
  {
    uint8_t next = scanner.peekChar();
    if (next == $equal) return scanner.peekChar(1) != $equal;
    return next == $lt || next == $gt;
  }
  // lookingAtExpressionEnd

  // Consumes an expression until it reaches a
  // top-level `<`, `>`, or a `=` that's not `==`.
  Expression* StylesheetParser::readExpressionUntilComparison()
  {
    return readExpression(false, false,
      &StylesheetParser::lookingAtExpressionEnd);
  }

  // Consumes a `@supports` condition.
  SupportsCondition* StylesheetParser::readSupportsCondition()
  {
    Offset start(scanner.offset);

    if (scanIdentifier("not")) {
      scanWhitespace();
      return SASS_MEMORY_NEW(SupportsNegation,
        scanner.relevantSpanFrom(start), readSupportsConditionInParens());
    }

    SupportsConditionObj condition =
      readSupportsConditionInParens();
    scanWhitespace();
    bool hasOp = false;
    SupportsOperation::Operand op;
    while (lookingAtIdentifier()) {
      if (hasOp) {
        if (op == SupportsOperation::AND)
          expectIdentifier("and", "\"and\"");
        else
          expectIdentifier("or", "\"or\"");
      }
      else if (scanIdentifier("or")) {
        op = SupportsOperation::OR;
        hasOp = true;
      }
      else {
        expectIdentifier("and", "\"and\"");
        op = SupportsOperation::AND;
        hasOp = true;
      }
      scanWhitespace();
      SupportsConditionObj right =
        readSupportsConditionInParens();
      condition = SASS_MEMORY_NEW(SupportsOperation,
        scanner.relevantSpanFrom(start), condition, right, op);
      scanWhitespace();
    }
    return condition.detach();
  }
  // EO readSupportsCondition

  // Consumes a parenthesized supports condition, or an interpolation.
  SupportsCondition* StylesheetParser::readSupportsConditionInParens()
  {
    Offset start(scanner.offset);

    if (lookingAtInterpolatedIdentifier()) {
      InterpolationObj identifier = readInterpolatedIdentifier();
      sass::string initialPlain(identifier->getInitialPlain());
      if (StringUtils::equalsIgnoreCase(initialPlain, "not", 3)) {
        error("\"not\" is not a valid identifier here.", identifier->pstate());
      }

      if (scanner.scanChar($lparen)) {
        InterpolationObj arguments =
          readInterpolatedDeclarationValue(true, true);
        scanner.expectChar($rparen);
        return SASS_MEMORY_NEW(SupportsFunction,
          scanner.relevantSpanFrom(start),
          identifier, arguments);
      }
      else if (identifier->size() != 1 || !identifier->first()->isaExpression()) {
        error("Expected @supports condition.", identifier->pstate());
      }
      return SASS_MEMORY_NEW(SupportsInterpolation,
        scanner.relevantSpanFrom(start),
        identifier->first()->isaExpression());
    }

    scanner.expectChar($lparen);
    scanWhitespace();
    if (scanIdentifier("not")) {
      scanWhitespace();
      SupportsConditionObj condition
        = readSupportsConditionInParens();
      scanner.expectChar($rparen);
      return SASS_MEMORY_NEW(SupportsNegation,
        scanner.relevantSpanFrom(start),
        condition);
    }
    else if (scanner.peekChar() == $lparen) {
      SupportsConditionObj condition
        = readSupportsCondition();
      scanner.expectChar($rparen);
      return condition.detach();
    }

    ExpressionObj name;
    StringScannerState state(scanner.state());
    try {
      name = readExpression();
      scanner.expectChar($colon);
    }
    catch (Exception::ParserException& err) {

      scanner.backtrack(state);
      InterpolationObj identifier = readInterpolatedIdentifier();
      SupportsOperationObj operation = trySupportsOperation(identifier, start);
      if (operation != nullptr) {
        scanner.expectChar($rparen);
        return operation.detach();
      }

      // If parsing an expression fails, try to parse an
      // `InterpolatedAnyValue` instead. But if that value runs into a
      // top-level colon, then this is probably intended to be a declaration
      // after all, so we rethrow the declaration-parsing error.
      InterpolationBuffer buffer(scanner);
      buffer.addInterpolation(identifier);
      buffer.addInterpolation(readInterpolatedDeclarationValue(true, true, false));
      if (scanner.peekChar() == $colon) throw err;
      scanner.expectChar($rparen);

      return SASS_MEMORY_NEW(SupportsAnything, scanner.relevantSpanFrom(start),
        buffer.getInterpolation(scanner.relevantSpanFrom(start), false));
    }

    SupportsDeclarationObj declaration = readSupportsDeclarationValue(name, start);
    scanner.expectChar($rparen);
    return declaration.detach();

    //scanWhitespace();
    //ExpressionObj value(readExpression());
    //scanner.expectChar($rparen);
    //
    //return SASS_MEMORY_NEW(SupportsDeclaration,
    //  scanner.relevantSpanFrom(start), name, value);
  }
  // EO readSupportsConditionInParens

  // Tries to consume a negated supports condition. Returns `null` if it fails.
  SupportsNegation* StylesheetParser::trySupportsNegation()
  {
    StringScannerState start = scanner.state();
    if (!scanIdentifier("not") || scanner.isDone()) {
      scanner.backtrack(start);
      return nullptr;
    }

    uint8_t next = scanner.peekChar();
    if (!isWhitespace(next) && next != $lparen) {
      scanner.backtrack(start);
      return nullptr;
    }

    scanWhitespace();

    return SASS_MEMORY_NEW(SupportsNegation,
      scanner.relevantSpanFrom(start.offset),
      readSupportsConditionInParens());

  }
  // trySupportsNegation

  // Parses and returns the right-hand side of
  // a declaration in a supports query.
  SupportsDeclaration* StylesheetParser::readSupportsDeclarationValue(
    Expression* name, Offset& start)
  {
    ExpressionObj value;
    if (StringExpression* ex = name->isaStringExpression()) {
      if (ex->text() != nullptr && ex->hasQuotes() == false) {
        auto plain = ex->text()->getInitialPlain();
        if (strncmp(plain.c_str(), "--", 2) == 0) {
          value = SASS_MEMORY_NEW(StringExpression,
            scanner.rawSpanFrom(start),
            readInterpolatedDeclarationValue());
          return SASS_MEMORY_NEW(SupportsDeclaration,
            scanner.relevantSpanFrom(start),
            name, value);
        }
      }
    }
    scanWhitespace();
    value = readExpression();
    return SASS_MEMORY_NEW(SupportsDeclaration,
      scanner.relevantSpanFrom(start),
      name, value);
  }
  // EO readSupportsDeclarationValue

  // If [interpolation] is followed by `"and"` or `"or"`, parse it as a supports
  // operation. Otherwise, return `null` without moving the scanner position.
  SupportsOperation* StylesheetParser::trySupportsOperation(
    Interpolation* interpolation, Offset& start)
  {
    if (interpolation->size() != 1) return nullptr;
    Interpolant* expression = interpolation->first();
    if (!expression->isaExpression()) return nullptr;
    StringScannerState state(scanner.state());

    scanWhitespace();

    bool hasOp = false;
    SupportsOperation::Operand op;
    SupportsOperationObj operation{};
    while (lookingAtIdentifier()) {
      if (hasOp) {
        if (op == SupportsOperation::AND)
          expectIdentifier("and", "\"and\"");
        else
          expectIdentifier("or", "\"or\"");
      }
      else if (scanIdentifier("or")) {
        op = SupportsOperation::OR;
        hasOp = true;
      }
      else if (scanIdentifier("and")) {
        op = SupportsOperation::AND;
        hasOp = true;
      }
      else {
        scanner.backtrack(state);
        return nullptr;
      }

      scanWhitespace();

      SupportsConditionObj rhs = readSupportsConditionInParens();

      if (operation != nullptr) {
        operation = SASS_MEMORY_NEW(SupportsOperation,
          scanner.rawSpanFrom(start),
          operation.ptr(), rhs, op);
      }
      else {
        SupportsInterpolationObj wrapped = SASS_MEMORY_NEW(
          SupportsInterpolation, interpolation->pstate(),
          expression->isaExpression());
        operation = SASS_MEMORY_NEW(SupportsOperation,
          scanner.rawSpanFrom(start),
          wrapped.ptr(), rhs, op);
      }

      scanWhitespace();
    }

    return operation.detach();
  }

  // Returns whether the scanner is immediately before an identifier that may contain
  // interpolation. This is based on the CSS algorithm, but it assumes all backslashes
  // start escapes and it considers interpolation to be valid in an identifier.
  // https://drafts.csswg.org/css-syntax-3/#would-start-an-identifier
  bool StylesheetParser::lookingAtInterpolatedIdentifier() const
  {
    // See also [ScssParser._lookingAtIdentifier].

    uint8_t first = scanner.peekChar();
    if (first == $nul) return false;
    if (isNameStart(first) || first == $backslash) return true;
    if (first == $hash) return scanner.peekChar(1) == $lbrace;

    if (first != $minus) return false;
    uint8_t second = scanner.peekChar(1);
    if (second == $nul) return false;
    // if (isNameStart(second)) return true;
    // if (second == $backslash) return true;

    if (second == $hash) return scanner.peekChar(2) == $lbrace;
    // if (second != $minus) return false;

    // uint8_t third = scanner.peekChar(2);
    // if (third == $nul) return false;
    // if (third != $hash) return isNameStart(third);
    //  else return scanner.peekChar(3) == $lbrace;

    return isNameStart(second)
      || second == $backslash
      || second == $minus;
  }
  // EO lookingAtInterpolatedIdentifier

  // Returns whether the scanner is immediately before a sequence
  // of characters that could be part of an CSS identifier body.
  // The identifier body may include interpolation.
  bool StylesheetParser::lookingAtInterpolatedIdentifierBody() const
  {
    uint8_t first = scanner.peekChar();
    if (first == $nul) return false;
    if (isName(first) || first == $backslash) return true;
    return first == $hash && scanner.peekChar(1) == $lbrace;
  }
  // EO lookingAtInterpolatedIdentifierBody

  // Returns whether the scanner is immediately before a SassScript expression.
  bool StylesheetParser::lookingAtExpression() const
  {
    uint8_t character, next;
    if (!scanner.peekChar(character)) {
      return false;
    }
    if (character == $dot) {
      return scanner.peekChar(1) != $dot;
    }
    if (character == $exclamation) {
      if (!scanner.peekChar(next, 1)) {
      }
      return isWhitespace(next)
        || equalsLetterIgnoreCase($i, next);
    }

    return character == $lparen
      || character == $slash
      || character == $lbracket
      || character == $apos
      || character == $quote
      || character == $hash
      || character == $plus
      || character == $minus
      || character == $backslash
      || character == $dollar
      || character == $ampersand
      || isNameStart(character)
      || isDigit(character);
  }
  // EO lookingAtExpression

  // Like [identifier], but rejects identifiers that begin with `_` or `-`.
  sass::string StylesheetParser::readPublicIdentifier()
  {
    Offset start(scanner.offset);
    auto result = readIdentifier();

    uint8_t first = result[0];
    if (first == $minus || first == $underscore) {
      error("Private members can't be accessed from outside their modules.",
        scanner.rawSpanFrom(start));
    }

    return result;
  }
  // EO readPublicIdentifier

}

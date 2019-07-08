#include "parser_stylesheet.hpp"

#include <cstring>
#include "compiler.hpp"
#include "charcode.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "color_maps.hpp"
#include "exceptions.hpp"
#include "source_span.hpp"
#include "ast_supports.hpp"
#include "ast_statements.hpp"
#include "ast_expressions.hpp"
#include "parser_expression.hpp"

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


  // Parse stylesheet root block
  Root* StylesheetParser::parseRoot()
  {

    // skip over optional utf8 bom
    // ToDo: check influence on count
    scanner.scan(Strings::utf8bom);

    // Create initial states
    Offset start(scanner.offset);

    // The parsed children
    StatementVector children;

    // Check seems a bit esoteric but works
    if (context.included_sources.size() == 1) {
      // Apply headers only on very first include
      context.applyCustomHeaders(children,
        scanner.relevantSpanFrom(start));
    }

    // Parse nested root statements
    StatementVector parsed(readStatements(
      &StylesheetParser::readRootStatement));

    // Move parsed children into our array
    children.insert(children.end(),
      std::make_move_iterator(parsed.begin()),
      std::make_move_iterator(parsed.end()));

    // make sure everything is parsed
    scanner.expectDone();

    // Finalize variable scopes
    context.varRoot.finalizeScopes();

    // Return the new root object
    return SASS_MEMORY_NEW(Root,
      scanner.relevantSpanFrom(start),
      std::move(children));
  }
  // EO parseRoot

  // Consumes a variable declaration.
  Statement* StylesheetParser::readVariableDeclaration()
  {
    Offset start(scanner.offset);

    sass::string ns;
    sass::string id = variableName();
    if (scanner.scanChar($dot)) {
      ns = id;
      id = readPublicIdentifier();
    }

    // Create EnvKey from id
    EnvKey name(std::move(id));

    if (plainCss()) {
      error("Sass variables aren't allowed in plain CSS.",
        scanner.relevantSpanFrom(start));
    }

    scanWhitespace();
    scanner.expectChar($colon);
    scanWhitespace();

    ExpressionObj value = readExpression();

    bool guarded = false;
    bool global = false;

    Offset flagStart(scanner.offset);
    while (scanner.scanChar($exclamation)) {
       sass::string flag = readIdentifier();
       if (flag == "default") {
         guarded = true;
       }
       else if (flag == "global") {
         if (!ns.empty()) {
           error("!global isn't allowed for variables in other modules.",
             scanner.relevantSpanFrom(flagStart));
         }
         global = true;
       }
       else {
         error("Invalid flag name.",
           scanner.relevantSpanFrom(flagStart));
       }

       scanWhitespace();
       flagStart = scanner.offset;
    }

    expectStatementSeparator("variable declaration");

    bool has_local = false;
    // Skip to optional global scope
    EnvFrame* frame = global ?
      context.varStack.front() :
      context.varStack.back();
    // As long as we are not in a loop construct, we
    // can utilize full static variable optimizations.
    VarRef vidx(inLoopDirective ?
      frame->getLocalVariableIdx(name) :
      frame->getVariableIdx(name));
    // Check if we found a local variable to use
    if (vidx.isValid()) has_local = true;
    // Otherwise we must create a new local variable
    else vidx = frame->createVariable(name);

    // JIT self assignments
    #ifdef SASS_OPTIMIZE_SELF_ASSIGN
    // Optimization for cases where functions manipulate same variable
    // We detect these cases here in order for the function to optimize
    // self-assignment case (e.g. map-merge). It can then manipulate
    // the value passed as the first argument directly in-place.
    if (has_local && !global) {
      // Certainly looks a bit like some poor man's JIT 
      if (auto fn = value->isaFunctionExpression()) {
        auto& pos(fn->arguments()->positional());
        if (pos.size() > 0) {
          if (auto var = pos[0]->isaVariableExpression()) {
            if (var->name() == name) { // same name
              fn->selfAssign(true); // Up to 15% faster
            }
          }
        }
      }
    }
    #endif

    AssignRule* declaration = SASS_MEMORY_NEW(AssignRule,
      scanner.relevantSpanFrom(start), name, vidx, value, guarded, global);
    if (inLoopDirective) frame->assignments.push_back(declaration);
    return declaration;
  }
  // EO variableDeclaration

  // Consumes a statement that's allowed at the top level of the stylesheet or
  // within nested style and at rules. If [root] is `true`, this parses at-rules
  // that are allowed only at the root of the stylesheet.
  Statement* StylesheetParser::readStatement(bool root)
  {
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
      isUseAllowed = false;
      if (inStyleRule || inUnknownAtRule || inMixin || inContentBlock) {
        return readDeclarationOrStyleRule();
      }
      else {
        return readStyleRule();
      }

    }
    return nullptr;
  }
  // EO readStatement

  // Consumes a style rule.
  StyleRule* StylesheetParser::readStyleRule()
  {
    LOCAL_FLAG(inStyleRule, true);

    // The indented syntax allows a single backslash to distinguish a style rule
    // from old-style property syntax. We don't support old property syntax, but
    // we do support the backslash because it's easy to do.
    if (isIndented()) scanner.scanChar($backslash);
    InterpolationObj readStyleRule(styleRuleSelector());
    EnvFrame local(context.varStack, false);

    Offset start(scanner.offset);
    return withChildren<StyleRule>(
      &StylesheetParser::readChildStatement,
      start, readStyleRule.ptr(), local.idxs);

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
      // _propertyOrVariableDeclaration
      return readDeclaration();
    }

    // The indented syntax allows a single backslash to distinguish a style rule
    // from old-style property syntax. We don't support old property syntax, but
    // we do support the backslash because it's easy to do.
    if (isIndented() && scanner.scanChar($backslash)) {
      return readStyleRule();
    }

    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    Declaration* declaration = tryDeclarationOrBuffer(buffer);

    if (declaration != nullptr) {
      return declaration;
    }

    buffer.addInterpolation(styleRuleSelector());
    SourceSpan selectorPstate(scanner.relevantSpanFrom(start));

    LOCAL_FLAG(inStyleRule, true);

    if (buffer.empty()) {
      error("expected \"}\".",
        scanner.relevantSpan());
    }

    EnvFrame local(context.varStack, true);
    InterpolationObj itpl = buffer.getInterpolation(
      scanner.relevantSpanFrom(start));
    StyleRuleObj rule = withChildren<StyleRule>(
      &StylesheetParser::readChildStatement,
      start, itpl.ptr(), local.idxs);
    if (isIndented() && rule->empty()) {
      context.addWarning(
        "This selector doesn't have any properties and won't be rendered.",
        selectorPstate);
    }
    return rule.detach();
  }
  // readDeclarationOrStyleRule


  // Tries to parse a declaration, and returns the value parsed so
  // far if it fails. This can return either an [InterpolationBuffer],
  // indicating that it couldn't consume a declaration and that selector
  // parsing should be attempted; or it can return a [Declaration],
  // indicating that it successfully consumed a declaration.
  Declaration* StylesheetParser::tryDeclarationOrBuffer(InterpolationBuffer& nameBuffer)
  {
    Offset start(scanner.offset);

    // Allow the "*prop: val", ":prop: val",
    // "#prop: val", and ".prop: val" hacks.
    uint8_t first = scanner.peekChar();
    if (first == $colon || first == $asterisk || first == $dot ||
        (first == $hash && scanner.peekChar(1) != $lbrace)) {
      sass::sstream strm;
      strm << scanner.readChar();
      strm << rawText(&StylesheetParser::scanWhitespace);
      nameBuffer.write(strm.str(), scanner.relevantSpanFrom(start));
    }

    if (!lookingAtInterpolatedIdentifier()) {
      return nullptr;
    }

    nameBuffer.addInterpolation(readInterpolatedIdentifier());
    if (scanner.matches("/*")) nameBuffer.write(rawText(&StylesheetParser::loudComment));

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
  Declaration* StylesheetParser::readDeclaration(bool parseCustomProperties)
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
    else {
      name = readInterpolatedIdentifier();
    }

    scanWhitespace();
    scanner.expectChar($colon);
    scanWhitespace();

    if (parseCustomProperties && startsWith(name->getInitialPlain(), "--", 2)) {
      InterpolationObj value = readInterpolatedDeclarationValue();
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
        &StylesheetParser::readDeclarationOrAtRule,
        start, name, nullptr, false);
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
        start, name, value, false);
    }
    else {
      expectStatementSeparator();
      return SASS_MEMORY_NEW(Declaration,
        scanner.relevantSpanFrom(start), name, value);
    }

  }
  // EO readDeclaration

  // Consumes a statement that's allowed within a declaration.
  Statement* StylesheetParser::readDeclarationOrAtRule()
  {
    if (scanner.peekChar() == $at) {
      return readDeclarationAtRule();
    }
    return readDeclaration(false);
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
    LOCAL_FLAG(isUseAllowed, false);

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
      return readAnyAtRule(start, name);
      // isUseAllowed = wasUseAllowed;
      // if (!root) throwDisallowedAtRule(start);
      // return parseUseRule(start);
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
        &StylesheetParser::readDeclarationAtRule);
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
  Statement* StylesheetParser::readFunctionAtRule()
  {
    if (scanner.peekChar() != $at) {
      // If a variable declaration failed to parse, it's possible the user
      // thought they could write a style rule or property declaration in a
      // function. If so, throw a more helpful error message.
      StatementObj statement(readDeclarationOrStyleRule());
      // ToDo: dart-sass has a try/catch clause here!?
      bool isStyleRule = statement->isaStyleRule();
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
        &StylesheetParser::readFunctionAtRule);
    }
    else if (name == "else") {
      return throwDisallowedAtRule(start);
    }
    else if (name == "error") {
      return readErrorRule(start);
    }
    else if (name == "for") {
      return readForRule(start,
        &StylesheetParser::readFunctionAtRule);
    }
    else if (name == "if") {
      return readIfRule(start,
        &StylesheetParser::readFunctionAtRule);
    }
    else if (name == "return") {
      return readReturnRule(start);
    }
    else if (name == "warn") {
      return readWarnRule(start);
    }
    else if (name == "while") {
      return readWhileRule(start,
        &StylesheetParser::readFunctionAtRule);
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

    EnvFrame local(context.varStack, false);

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

    ArgumentInvocationObj args;
    if (scanner.peekChar() == $lparen) {
      args = readArgumentInvocation(true);
    }
    else {
      args = SASS_MEMORY_NEW(ArgumentInvocation,
        scanner.relevantSpan(), ExpressionVector(), {});
    }

    LOCAL_FLAG(mixinHasContent, true);
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
    LOCAL_FLAG(inControlDirective, true);
    LOCAL_FLAG(inLoopDirective, true);
    sass::vector<EnvKey> variables;
    EnvFrame local(context.varStack, true);
    variables.emplace_back(variableName());
    local.createVariable(variables.back());
    scanWhitespace();
    while (scanner.scanChar($comma)) {
      scanWhitespace();
      variables.emplace_back(variableName());
      local.createVariable(variables.back());
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

  // Consumes a function declaration.
  // [start] should point before the `@`.
  FunctionRule* StylesheetParser::readFunctionRule(Offset start)
  {
    // Variables should not be hoisted through
    EnvFrame* parent = context.varStack.back();
    EnvFrame local(context.varStack, false);

    // var precedingComment = lastSilentComment;
    // lastSilentComment = null;
    sass::string name = readIdentifier();
    sass::string normalized(name);
    scanWhitespace();

    ArgumentDeclarationObj arguments = parseArgumentDeclaration();

    if (inMixin || inContentBlock) {
      error("Mixins may not contain function declarations.",
        scanner.relevantSpanFrom(start));
    }
    else if (inControlDirective) {
      error("Functions may not be declared in control directives.",
        scanner.relevantSpanFrom(start));
    }

    sass::string fname(Util::unvendor(name));
    if (fname == "calc" || fname == "element" || fname == "expression" ||
      fname == "url" || fname == "and" || fname == "or" || fname == "not") {
      error("Invalid function name.",
        scanner.relevantSpanFrom(start));
    }

    scanWhitespace();
    FunctionRule* rule = withChildren<FunctionRule>(
      &StylesheetParser::readFunctionAtRule,
      start, name, arguments, local.idxs);
    rule->fidx(parent->createFunction(name));
    return rule;
  }
  // EO readFunctionRule

  ForRule* StylesheetParser::readForRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    LOCAL_FLAG(inControlDirective, true);
    LOCAL_FLAG(inLoopDirective, true);
    EnvFrame local(context.varStack, true);
    sass::string variable = variableName();
    local.createVariable(variable);
    scanWhitespace();
    expectIdentifier("from", "\"from\"");
    scanWhitespace();
    ExpressionObj from = readSingleExpression();
    scanWhitespace();
    bool inclusive = false;
    if (!tryForRuleOperator(inclusive)) {
      error("Expected \"to\" or \"through\".",
        scanner.relevantSpan());
    }
    scanWhitespace();
    ExpressionObj to = readExpression();
    return withChildren<ForRule>(child, start,
      variable, from, to, inclusive, local.idxs);
  }

  // ToDo: dart-sass stores all else ifs in the same object, smart ...
  IfRule* StylesheetParser::readIfRule(Offset start, Statement* (StylesheetParser::* child)())
  {
    // var ifIndentation = currentIndentation;
    size_t ifIndentation = 0;
    LOCAL_FLAG(inControlDirective, true);
    ExpressionObj predicate = readExpression();

    IfRuleObj root;
    IfRuleObj cur;

    /* create anonymous lexical scope */ {
      EnvFrame local(context.varStack, true);
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

        EnvFrame local(context.varStack, true);
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

        EnvFrame local(context.varStack, true);

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

  void StylesheetParser::scanImportArgument(ImportRule* rule)
  {
    const char* startpos = scanner.position;
    Offset start(scanner.offset);
    uint8_t next = scanner.peekChar();
    if (next == $u || next == $U) {
      Expression* url = readFunctionOrStringExpression();
      scanWhitespace();
      auto queries = tryImportQueries();
      rule->append(SASS_MEMORY_NEW(StaticImport,
        scanner.relevantSpanFrom(start),
        SASS_MEMORY_NEW(Interpolation,
           url->pstate(), url),
        queries.first, queries.second));
      return;
    }

    sass::string url = string();
    const char* rawUrlPos = scanner.position;
    SourceSpan pstate = scanner.relevantSpanFrom(start);
    scanWhitespace();
    auto queries = tryImportQueries();
    if (isPlainImportUrl(url) || queries.first != nullptr || queries.second != nullptr) {
      // Create static import that is never
      // resolved by libsass (output as is)
      rule->append(SASS_MEMORY_NEW(StaticImport,
        scanner.relevantSpanFrom(start),
        SASS_MEMORY_NEW(Interpolation, pstate,
          SASS_MEMORY_NEW(String, pstate,
            sass::string(startpos, rawUrlPos))),
         queries.first, queries.second));
    }
    // Otherwise return a dynamic import
    // Will resolve during the eval stage
    else {
      // Check for valid dynamic import
      if (inControlDirective || inMixin) {
        throwDisallowedAtRule(rule->pstate().position);
      }
      // Call custom importers and check if any of them handled the import
      if (!context.callCustomImporters(url, pstate, rule)) {
        // Try to load url into context.sheets
        resolveDynamicImport(rule, start, url);
      }
    }
  
  }

  // Resolve import of [path] and add imports to [rule]
  void StylesheetParser::resolveDynamicImport(
    ImportRule* rule, Offset start, const sass::string& path)
  {
    SourceSpan pstate = scanner.relevantSpanFrom(start);
    const ImportRequest import(path, scanner.sourceUrl);
    callStackFrame frame(context, { pstate, Strings::importRule });

    // Search for valid imports (e.g. partials) on the file-system
    // Returns multiple valid results for ambiguous import path
    const sass::vector<ResolvedImport> resolved(context.findIncludes(import));

    // Error if no file to import was found
    if (resolved.empty()) {
      context.addFinalStackTrace(pstate);
      throw Exception::ParserException(context,
        "Can't find stylesheet to import.");
    }
    // Error if multiple files to import were found
    else if (resolved.size() > 1) {
      sass::sstream msg_stream;
      msg_stream << "It's not clear which file to import. Found:\n";
      for (size_t i = 0, L = resolved.size(); i < L; ++i)
      { msg_stream << "  " << resolved[i].imp_path << "\n"; }
      throw Exception::ParserException(context, msg_stream.str());
    }

    // We made sure exactly one entry was found, load its content
    if (ImportObj loaded = context.loadImport(resolved[0])) {
      StyleSheet* sheet = context.registerImport(loaded);
      rule->append(SASS_MEMORY_NEW(IncludeImport, pstate, sheet));
    }
    else {
      context.addFinalStackTrace(pstate);
      throw Exception::ParserException(context,
        "Couldn't read stylesheet for import.");
    }

  }
  // EO resolveDynamicImport


  /*
  sass::string StylesheetParser::parseImportUrl(sass::string url)
  {

    // Backwards-compatibility for implementations that
    // allow absolute Windows paths in imports.
    if (File::is_absolute_path(url)) {
    //   return p.windows.toUri(url).toString();
    }
    

    using LUrlParser::clParseURL;
    clParseURL clURL = clParseURL::ParseURL(url);

    if (clURL.IsValid()) {

    }


    // Throw a [FormatException] if [url] is invalid.
    // Uri.parse(url);
    return url;
  }
  */

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
    ArgumentInvocationObj arguments;
    if (scanner.peekChar() == $lparen) {
      arguments = readArgumentInvocation(true);
    }
    scanWhitespace();

    EnvFrame local(context.varStack, true);

    ArgumentDeclarationObj contentArguments;
    if (scanIdentifier("using")) {
      scanWhitespace();
      contentArguments = parseArgumentDeclaration();
      scanWhitespace();
    }

    // ToDo: Add checks to allow to omit arguments fully
    if (!arguments) {
      SourceSpan pstate(scanner.relevantSpanFrom(start));
      arguments = SASS_MEMORY_NEW(ArgumentInvocation,
        std::move(pstate), {}, {});
    }

    IncludeRuleObj rule = SASS_MEMORY_NEW(IncludeRule,
      scanner.relevantSpanFrom(start), name, arguments);

    if (!name.empty()) {
      // Get the function through the whole stack
      auto midx = context.varStack.back()->getMixinIdx(name);
      rule->midx(midx);
    }


    ContentBlockObj content;
    if (contentArguments || lookingAtChildren()) {
      LOCAL_FLAG(inContentBlock, true);
      // EnvFrame inner(context.varStack);
      if (contentArguments.isNull()) {
        // Dart-sass creates this one too
        contentArguments = SASS_MEMORY_NEW(
          ArgumentDeclaration,
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
    InterpolationObj query = readMediaQueryList();
    return withChildren<MediaRule>(
      &StylesheetParser::readChildStatement,
      start, query);
  }

  // Consumes a mixin declaration.
  // [start] should point before the `@`.
  MixinRule* StylesheetParser::readMixinRule(Offset start)
  {

    EnvFrame* parent = context.varStack.back();
    EnvFrame local(context.varStack, false);
    // Create space for optional content callable
    // ToDo: check if this can be conditionally done?
    auto cidx = local.createMixin(Keys::contentRule);
    // var precedingComment = lastSilentComment;
    // lastSilentComment = null;
    sass::string name = readIdentifier();
    scanWhitespace();

    ArgumentDeclarationObj arguments;
    if (scanner.peekChar() == $lparen) {
      arguments = parseArgumentDeclaration();
    }
    else {
      // Dart-sass creates this one too
      arguments = SASS_MEMORY_NEW(ArgumentDeclaration,
        scanner.relevantSpan(), sass::vector<ArgumentObj>()); // empty declaration
    }

    if (inMixin || inContentBlock) {
      error("Mixins may not contain mixin declarations.",
        scanner.relevantSpanFrom(start));
    }
    else if (inControlDirective) {
      error("Mixins may not be declared in control directives.",
        scanner.relevantSpanFrom(start));
    }

    scanWhitespace();
    LOCAL_FLAG(inMixin, true);
    LOCAL_FLAG(mixinHasContent, false);

    VarRef fidx = parent->createMixin(name);
    MixinRule* rule = withChildren<MixinRule>(
      &StylesheetParser::readChildStatement,
      start, name, arguments, local.idxs);
    rule->midx(fidx); // to parent
    rule->cidx(cidx);
    return rule;
  }
  // EO _mixinRule

  // Consumes a `@moz-document` rule. Gecko's `@-moz-document` diverges
  // from [the specification][] allows the `url-prefix` and `domain`
  // functions to omit quotation marks, contrary to the standard.
  // [the specification]: http://www.w3.org/TR/css3-conditional/
  AtRule* StylesheetParser::readMozDocumentRule(Offset start, Interpolation* name)
  {

    Offset valueStart(scanner.offset);
    InterpolationBuffer buffer(scanner);
    bool needsDeprecationWarning = false;

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
      start, name, value, false);

    if (needsDeprecationWarning) {

      context.addDeprecation(
        "@-moz-document is deprecated and support will be removed from Sass "
        "in a future release. For details, see http://bit.ly/moz-document.",
        atRule->pstate());
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
    EnvFrame local(context.varStack, true);
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
    LOCAL_FLAG(inControlDirective, true);
    LOCAL_FLAG(inLoopDirective, true);
    EnvFrame local(context.varStack, true);
    ExpressionObj condition(readExpression());
    return withChildren<WhileRule>(child,
      start, condition.ptr(), local.idxs);
  }
  // EO readWhileRule

  // Consumes an at-rule that's not explicitly supported by Sass.
  // [start] should point before the `@`. [name] is the name of the at-rule.
  AtRule* StylesheetParser::readAnyAtRule(Offset start, Interpolation* name)
  {
    LOCAL_FLAG(inUnknownAtRule, true);

    InterpolationObj value;
    uint8_t next = scanner.peekChar();
    if (next != $exclamation && !atEndOfStatement()) {
      value = readAlmostAnyValue();
    }

    if (lookingAtChildren()) {
      return withChildren<AtRule>(
        &StylesheetParser::readChildStatement,
        start, name, value, false);
    }
    expectStatementSeparator();
    return SASS_MEMORY_NEW(AtRule,
      scanner.relevantSpanFrom(start),
      name, value, true);
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
  ArgumentDeclaration* StylesheetParser::parseArgumentDeclaration()
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
        context.varStack.back()->createVariable(norm);
        break;
      }

      // Defer adding variable until we parsed expression
      // Just in case the same variable is mentioned again
      context.varStack.back()->createVariable(norm);

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
      ArgumentDeclaration,
      scanner.relevantSpanFrom(start),
      std::move(arguments),
      std::move(restArgument));

  }
  // EO parseArgumentDeclaration

  // Consumes an argument invocation. If [mixin] is `true`, this is parsed 
  // as a mixin invocation. Mixin invocations don't allow the Microsoft-style
  // `=` operator at the top level, but function invocations do.
  ArgumentInvocation* StylesheetParser::readArgumentInvocation(bool mixin)
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
    }
    scanner.expectChar($rparen);

    return SASS_MEMORY_NEW(
      ArgumentInvocation,
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

      case $single_quote:
      case $double_quote:
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

      case $langle:
        scanner.readChar();
        ep.addOperator(scanner.scanChar($equal)
          ? SassOperator::LTE : SassOperator::LT, beforeToken);
        break;

      case $rangle:
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
      return list;
    }
    else {
      ep.resolveSpaceExpressions();
      if (bracketList) {
        ListExpression* list = SASS_MEMORY_NEW(ListExpression,
          scanner.relevantSpanFrom(start), SASS_UNDEF);
        list->append(ep.singleExpression);
        list->hasBrackets(true);
        return list;
      }
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

    case $single_quote:
    case $double_quote:
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
        scanner.relevantSpan());
      return nullptr;
    }
  }
  // EO readSingleExpression

  // Consumes a parenthesized expression.
  Expression* StylesheetParser::readParenthesizedExpression()
  {
    if (plainCss()) {
      // This one is needed ...
      error("Parentheses aren't allowed in plain CSS.",
        scanner.rawSpan());
    }

    LOCAL_FLAG(inParentheses, true);

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
        }
      }
    }

    SourceSpan pstate(scanner.relevantSpanFrom(state.offset));
    sass::string original(state.position, scanner.position);
    return SASS_MEMORY_NEW(ColorExpression, pstate,
      SASS_MEMORY_NEW(ColorRgba, pstate,
        red, green, blue, alpha, original));
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
      (scanner.peekChar() != $dash || scanner.peekChar(1) != $dash)) {
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
        // substitution is required. perform the substitution on a copy
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
    Offset start(scanner.offset);
    StringScannerState state(scanner.state());
    if (scanner.peekChar() != $dot) return 0.0;

    if (!isDigit(scanner.peekChar(1))) {
      if (allowTrailingDot) return 0.0;
      error("Expected digit.",
        scanner.relevantSpanFrom(start));
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
      callStackFrame frame(context,
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
    expectCharIgnoreCase($u);
    scanner.expectChar($plus);

    size_t i = 0;
    for (; i < 6; i++) {
      if (!scanCharIf(isHex)) break;
    }

    if (scanner.scanChar($question)) {
      i++;
      for (; i < 6; i++) {
        if (!scanner.scanChar($question)) break;
      }
      return SASS_MEMORY_NEW(StringExpression,
        scanner.rawSpanFrom(state.offset),
        scanner.substring(state.position));
    }
    if (i == 0) {
      error("Expected hex digit or \"?\".",
        scanner.relevantSpan());
    }

    if (scanner.scanChar($minus)) {
      size_t j = 0;
      for (; j < 6; j++) {
        if (!scanCharIf(isHex)) break;
      }
      if (j == 0) {
        error("Expected hex digit.",
          scanner.relevantSpan());
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

  // Consumes a variable expression.
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

    VariableExpression* expression =
      SASS_MEMORY_NEW(VariableExpression,
        scanner.relevantSpanFrom(start),
        name);

    if (inLoopDirective) {
      // Static variable resolution will be done in finalize stage
      // Must be postponed since in loops we may reference post vars
      context.varStack.back()->variables.push_back(expression);
    }
    else {
      // Otherwise utilize full static optimizations
      EnvFrame* frame(context.varStack.back());
      VarRef vidx(frame->getVariableIdx(name, true));
      if (vidx.isValid()) expression->vidxs().push_back(vidx);
      else context.varStack.back()->variables.push_back(expression);
    }

    return expression;
  }
  // readVariableExpression

  // Consumes a selector expression.
  ParentExpression* StylesheetParser::readParentExpression()
  {
    if (plainCss()) {
      error("The parent selector isn't allowed in plain CSS.",
        scanner.rawSpan());
      /* ,length: 1 */
    }

    Offset start(scanner.offset);
    scanner.expectChar($ampersand);

    if (scanner.scanChar($ampersand)) {
      context.addWarning(
        "In Sass, \"&&\" means two copies of the parent selector. You "
        "probably want to use \"and\" instead.",
        scanner.relevantSpanFrom(start));
      scanner.offset.column -= 1;
      scanner.position -= 1;
    }

    return SASS_MEMORY_NEW(ParentExpression,
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

    if (quote != $single_quote && quote != $double_quote) {
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

    if (!plain.empty()) {
      if (plain == "if") {
        ArgumentInvocation* invocation = readArgumentInvocation();
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

        // ToDo: dart-sass has ColorExpression(color);
        if (const ColorRgba* color = name_to_color(plain)) {
          ColorRgba* copy = SASS_MEMORY_COPY(color);
          copy->pstate(identifier->pstate());
          copy->disp(plain);
          return SASS_MEMORY_NEW(ColorExpression,
            copy->pstate(), copy);
        }
      }

      auto specialFunction = trySpecialFunction(plain, start);
      if (specialFunction != nullptr) {
        return specialFunction;
      }
    }

    sass::string ns;
    Offset beforeName(scanner.offset);
    uint8_t next = scanner.peekChar();
    if (next == $dot) {

      if (scanner.peekChar(1) == $dot) {
        return SASS_MEMORY_NEW(StringExpression,
          scanner.relevantSpanFrom(beforeName), identifier);
      }

      ns = identifier->getPlainString();
      scanner.readChar();
      beforeName = scanner.offset;

      Offset start(scanner.offset);
      StringObj ident(SASS_MEMORY_NEW(String,
        scanner.relevantSpanFrom(start),
        readPublicIdentifier()));

      InterpolationObj itpl = SASS_MEMORY_NEW(Interpolation,
        ident->pstate());

      if (ns.empty()) {
        error("Interpolation isn't allowed in namespaces.",
          scanner.relevantSpanFrom(start));
      }

      ArgumentInvocation* args = readArgumentInvocation();
      return SASS_MEMORY_NEW(FunctionExpression,
        scanner.relevantSpanFrom(start), itpl, args, ns);

    }
    else if (next == $lparen) {
      ArgumentInvocation* args = readArgumentInvocation();
      FunctionExpression* fn = SASS_MEMORY_NEW(FunctionExpression,
        scanner.relevantSpanFrom(start), identifier, args, ns);
      sass::string name(identifier->getPlainString());
      if (!name.empty()) {
        // Get the function through the whole stack
        auto fidx = context.varStack.back()->getFunctionIdx(name);
        fn->fidx(fidx);
      }
      return fn;
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
    sass::string normalized(Util::unvendor(name));

    if (normalized == "calc" || normalized == "element" || normalized == "expression") {
      if (!scanner.scanChar($lparen)) return nullptr;
      buffer.write(name);
      buffer.write($lparen);
    }
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
        if (!tryMinMaxFunction(buffer, "calc")) return false;
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
        if (scanCharIgnoreCase($i)) {
          if (!scanCharIgnoreCase($n)) return false;
          buffer.write("min(");
        }
        else if (scanCharIgnoreCase($a)) {
          if (!scanCharIgnoreCase($x)) return false;
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
  // EO tryMinMaxContents

  // Consumes a function named [name] containing an
  // `InterpolatedDeclarationValue` if possible, and
  // adds its text to [buffer]. Returns whether such a
  // function could be consumed.
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
      else if (next == $exclamation ||
          next == $percent ||
          next == $ampersand ||
          (next >= $asterisk && next <= $tilde) ||
          next >= 0x0080) {
        buffer.write(scanner.readChar());
      }
      else if (next == $backslash) {
        escape(buffer.text);
      }
      else if (next == $hash) {
        if (scanner.peekChar(1) == $lbrace) {
          buffer.add(readSingleInterpolation());
        }
        else {
          buffer.write(scanner.readChar());
        }
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
    ArgumentInvocation* args = readArgumentInvocation();
    return SASS_MEMORY_NEW(FunctionExpression,
      pstate, itpl, args, "");
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
  Interpolation* StylesheetParser::readAlmostAnyValue()
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
        buffer.write(scanner.readChar());
        break;

      case $double_quote:
      case $single_quote:
        strex = readInterpolatedString();
        buffer.addInterpolation(strex->getAsInterpolation());
        break;

      case $slash:
        commentStart = scanner.position;
        if (scanComment()) {
          buffer.write(scanner.substring(commentStart));
        }
        else {
          buffer.write(scanner.readChar());
        }
        break;

      case $hash:
        if (scanner.peekChar(1) == $lbrace) {
          // Add a full interpolated identifier to handle cases like
          // "#{...}--1", since "--1" isn't a valid identifier on its own.
          buffer.addInterpolation(readInterpolatedIdentifier());
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
    return buffer.getInterpolation(scanner.relevantSpanFrom(start.offset), true);

  }
  // readAlmostAnyValue

    // Consumes tokens until it reaches a top-level `";"`, `")"`, `"]"`, or `"}"` and 
  // returns their contents as a string. If [allowEmpty] is `false` (the default), this
  // requires at least one token. Unlike [declarationValue], this allows interpolation.
  Interpolation* StylesheetParser::readInterpolatedDeclarationValue(bool allowEmpty)
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

      case $double_quote:
      case $single_quote:
        strex = readInterpolatedString();
        itpl = strex->getAsInterpolation();
        buffer.addInterpolation(itpl);
        wroteNewline = false;
        break;

      case $slash:
        if (scanner.peekChar(1) == $asterisk) {
          buffer.write(rawText(&StylesheetParser::loudComment));
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
        if (brackets.empty()) goto endOfLoop;
        buffer.write(scanner.readChar());
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

    if (scanner.scanChar($dash)) {
      buffer.writeCharCode($dash);
      if (scanner.scanChar($dash)) {
        buffer.writeCharCode($dash);
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
      buffer.add(ex);
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
          next == $dash ||
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
    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    while (true) {
      scanWhitespace();
      readMediaQuery(buffer);
      if (!scanner.scanChar($comma)) break;
      buffer.write($comma);
      buffer.write($space);
    }
    return buffer.getInterpolation(scanner.relevantSpanFrom(start));
  }
  // readMediaQueryList

  // Consumes a single media query and appends it to [buffer].
  void StylesheetParser::readMediaQuery(InterpolationBuffer& buffer)
  {
    // This is somewhat duplicated in MediaQueryParser.readMediaQuery.
    if (scanner.peekChar() != $lparen) {
      buffer.addInterpolation(readInterpolatedIdentifier());
      scanWhitespace();

      if (!lookingAtInterpolatedIdentifier()) {
        // For example, "@media screen {".
        return;
      }

      buffer.write($space);
      InterpolationObj identifier =
        readInterpolatedIdentifier();
      scanWhitespace();

      if (equalsIgnoreCase(identifier->getPlainString(), "and", 3)) {
        // For example, "@media screen and ..."
        buffer.write(" and ");
      }
      else {
        buffer.addInterpolation(identifier);
        if (scanIdentifier("and")) {
          // For example, "@media only screen and ..."
          scanWhitespace();
          buffer.write(" and ");
        }
        else {
          // For example, "@media only screen {"
          return;
        }
      }
    }

    // We've consumed either `IDENTIFIER "and"` or
    // `IDENTIFIER IDENTIFIER "and"`.

    while (true) {
      scanWhitespace();
      buffer.addInterpolation(readMediaFeature());
      scanWhitespace();
      if (!scanIdentifier("and")) break;
      buffer.write(" and ");
    }
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
      bool isAngle = next == $langle || next == $rangle;
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
    return next == $langle || next == $rangle;
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
    uint8_t first = scanner.peekChar();
    if (first != $lparen && first != $hash) {
      Offset start(scanner.offset);
      expectIdentifier("not", "\"not\"");
      scanWhitespace();
      return SASS_MEMORY_NEW(SupportsNegation,
        scanner.relevantSpanFrom(start), readSupportsConditionInParens());
    }

    SupportsConditionObj condition =
      readSupportsConditionInParens();
    scanWhitespace();
    while (lookingAtIdentifier()) {
      SupportsOperation::Operand op;
      if (scanIdentifier("or")) {
        op = SupportsOperation::OR;
      }
      else {
        expectIdentifier("and", "\"and\"");
        op = SupportsOperation::AND;
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
    if (scanner.peekChar() == $hash) {
      return SASS_MEMORY_NEW(SupportsInterpolation,
        scanner.relevantSpanFrom(start), readSingleInterpolation());
    }

    scanner.expectChar($lparen);
    scanWhitespace();
    uint8_t next = scanner.peekChar();
    if (next == $lparen || next == $hash) {
      SupportsConditionObj condition
        = readSupportsCondition();
      scanWhitespace();
      scanner.expectChar($rparen);
      return condition.detach();
    }

    if (next == $n || next == $N) {
      SupportsNegationObj negation
        = trySupportsNegation();
      if (negation != nullptr) {
        scanner.expectChar($rparen);
        return negation.detach();
      }
    }

    ExpressionObj name(readExpression());
    scanner.expectChar($colon);
    scanWhitespace();
    ExpressionObj value(readExpression());
    scanner.expectChar($rparen);

    return SASS_MEMORY_NEW(SupportsDeclaration,
      scanner.relevantSpanFrom(start), name, value);
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

    if (first != $dash) return false;
    uint8_t second = scanner.peekChar(1);
    if (second == $nul) return false;
    // if (isNameStart(second)) return true;
    // if (second == $backslash) return true;

    if (second == $hash) return scanner.peekChar(2) == $lbrace;
    // if (second != $dash) return false;

    // uint8_t third = scanner.peekChar(2);
    // if (third == $nul) return false;
    // if (third != $hash) return isNameStart(third);
    //  else return scanner.peekChar(3) == $lbrace;

    return isNameStart(second)
      || second == $backslash
      || second == $dash;
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
      || character == $single_quote
      || character == $double_quote
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
    if (first == $dash || first == $underscore) {
      error("Private members can't be accessed from outside their modules.",
        scanner.rawSpanFrom(start));
    }

    return result;
  }
  // EO readPublicIdentifier

}

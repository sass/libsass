/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_expression.hpp"

#include "character.hpp"
#include "utf8/checked.h"
#include "ast_values.hpp"
#include "ast_expressions.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  void ExpressionParser::addOperator(SassOperator op, Offset& start) {
	  if (parser.plainCss() && op != SassOperator::ASSIGN &&
      // These are allowed in calculations, so
      // we have to check them at evaluation time.
      op != SassOperator::ADD &&
      op != SassOperator::SUB &&
      op != SassOperator::MUL &&
      op != SassOperator::DIV) {
      parser.error("Operators aren't allowed in plain CSS.",
        parser.scanner.relevantSpanFrom(start));
        /* ,
																		position: scanner.position - operator.operator.length,
																		length : operator.operator.length */
	  }

	  allowSlash = allowSlash && op == SassOperator::DIV; // done
	  while ((!operators.empty()) &&
		  (sass_op_to_precedence(operators.back())
		  >= sass_op_to_precedence(op))) {
      resolveOneOperation();
    }

    //std::cerr << "Add operator, prev [" << parser.scanner.peekChar(-2) << "], next [" << parser.scanner.peekChar(0) << "]\n";

    // ToDo: merge into one structure?
	  operators.emplace_back(op);
    opstates.emplace_back(parser.scanner.relevantSpanFrom(start));
    bool isSafe = Character::isWhitespace(parser.scanner.peekChar(-2));
    isSafe &= Character::isWhitespace(parser.scanner.peekChar(0));
    calcSafe.push_back(isSafe);

    // We started parsing with an operator
    if (singleExpression == nullptr) {
      SourceSpan pstate(parser.scanner.relevantSpanFrom(start));
      singleExpression = SASS_MEMORY_NEW(NullExpression,
        pstate, SASS_MEMORY_NEW(Null, pstate));
    }

	  // assert(singleExpression != null);

    operands.emplace_back(singleExpression);
	  parser.scanWhitespace();
	  // allowSlash = allowSlash && parser.lookingAtNumber();
	  singleExpression = parser.readSingleExpression();
    // allowSlash = allowSlash && singleExpression->isaNumberExpression();
  }

  ExpressionParser::ExpressionParser(StylesheetParser& parser) :
    start(parser.scanner.state()),
    commaExpressions(),
    singleEqualsOperand(),
    spaceExpressions(),
    operators(),
    opstates(),
    calcSafe(),
    operands(),
    allowSlash(true),
    singleExpression(),
    parser(parser)
  {
    // allowSlash = parser.lookingAtNumber();
    singleExpression = parser.readSingleExpression();
  }

  void ExpressionParser::resetState()
  {
    commaExpressions.clear();
    spaceExpressions.clear();
    operators.clear();
    opstates.clear();
    calcSafe.clear();
    operands.clear();
    parser.scanner.backtrack(start);
    allowSlash = true; // parser.lookingAtNumber();
    singleExpression = parser.readSingleExpression();
  }

  bool _isSlashOperand(Expression* expression) {
    if (expression->isaNumberExpression()) return true;
    if (expression->isaFunctionExpression()) return true;
    if (auto* op = expression->isaBinaryOpExpression()) {
      return op->allowsSlash();
    }
    return false;
  }

  void ExpressionParser::resolveOneOperation()
  {
    enum SassOperator op = operators.back();
    SourceSpan opstate = opstates.back();
    bool isCalcSafe = calcSafe.back();
    // auto start(parser.scanner.offset);
    operators.pop_back();
    opstates.pop_back();
    calcSafe.pop_back();

    auto left = operands.back();
    operands.pop_back();

    auto right = singleExpression;

    if (allowSlash && !parser.inParentheses && op == SassOperator::DIV
      && _isSlashOperand(left) && _isSlashOperand(right)) {

      singleExpression = SASS_MEMORY_NEW(BinaryOpExpression,
        SourceSpan::delta(left->pstate(), right->pstate()),
        op, std::move(opstate), left, right, true, isCalcSafe);

    }
    else {
      singleExpression = SASS_MEMORY_NEW(BinaryOpExpression,
        SourceSpan::delta(left->pstate(), right->pstate()),
        op, std::move(opstate), left, right, allowSlash = false, isCalcSafe);

      if (op == SassOperator::ADD || op == SassOperator::SUB) {
        // todo warn for deprecation
      }

    }
  }

  void ExpressionParser::resolveOperations()
  {
    if (operators.empty()) return;
    while (!operators.empty()) {
      resolveOneOperation();
    }
  }

  void ExpressionParser::addSingleExpression(ExpressionObj expression, bool number)
  {
    if (singleExpression != nullptr) {
		  // If we discover we're parsing a list whose first element is a division
		  // operation, and we're in parentheses, re-parse outside of a parent
		  // context. This ensures that `(1/2 1)` doesn't perform division on its
		  // first element.
		  if (parser.inParentheses) {
			  parser.inParentheses = false;
			  if (allowSlash) {
				  resetState();
				  return;
			  }
		  }

		  resolveOperations();
		  spaceExpressions.emplace_back(singleExpression);
		  allowSlash = true;
	  }
	  singleExpression = expression;

  }

  void ExpressionParser::resolveSpaceExpressions() {
    resolveOperations();

	  if (!spaceExpressions.empty()) {
		  spaceExpressions.emplace_back(singleExpression);
      SourceSpan span = SourceSpan::delta(
        spaceExpressions.front()->pstate(),
        spaceExpressions.back()->pstate());
      ListExpression* list = SASS_MEMORY_NEW(
        ListExpression, std::move(span), SASS_SPACE);
		  list->concat(std::move(spaceExpressions));
		  singleExpression = list;
		  spaceExpressions.clear();
	  }

    if (singleEqualsOperand && singleExpression) {
      singleExpression = SASS_MEMORY_NEW(BinaryOpExpression,
        SourceSpan::delta(singleEqualsOperand->pstate(), singleExpression->pstate()),
        SassOperator::IESEQ, parser.scanner.rawSpan(),
        singleEqualsOperand, singleExpression);
      singleEqualsOperand = {};

    }
      /*
	  // Seem to be for ms stuff
	  singleExpression = SASS_MEMORY_NEW(BinaryOpExpression,
	  "[pstateS2]", )

	  BinaryOperationExpression(
	  BinaryOperator.singleEquals, singleEqualsOperand, singleExpression);
	  singleEqualsOperand = null;
	  }
	  */

  }

}

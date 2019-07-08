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
	  if (parser.plainCss() && op != SassOperator::DIV) {
      parser.error("Operators aren't allowed in plain CSS.",
        parser.scanner.relevantSpanFrom(start));
        /* ,
																		position: scanner.position - operator.operator.length,
																		length : operator.operator.length */
	  }

	  allowSlash = allowSlash && op == SassOperator::DIV;
	  while ((!operators.empty()) &&
		  (sass_op_to_precedence(operators.back())
		  >= sass_op_to_precedence(op))) {
      resolveOneOperation();
    }
	  operators.emplace_back(op);

    // We started parsing with an operator
    if (singleExpression == nullptr) {
      SourceSpan pstate(parser.scanner.relevantSpanFrom(start));
      singleExpression = SASS_MEMORY_NEW(NullExpression,
        pstate, SASS_MEMORY_NEW(Null, pstate));
    }

	  // assert(singleExpression != null);

    operands.emplace_back(singleExpression);
	  parser.scanWhitespace();
	  allowSlash = allowSlash && parser.lookingAtNumber();
	  singleExpression = parser.readSingleExpression();
    allowSlash = allowSlash && singleExpression->isaNumberExpression();
  }

  ExpressionParser::ExpressionParser(StylesheetParser& parser) :
    start(parser.scanner.state()),
    commaExpressions(),
    singleEqualsOperand(),
    spaceExpressions(),
    operators(),
    operands(),
    allowSlash(false),
    singleExpression(),
    parser(parser)
  {
    allowSlash = parser.lookingAtNumber();
    singleExpression = parser.readSingleExpression();
  }

  void ExpressionParser::resetState()
  {
    commaExpressions.clear();
    spaceExpressions.clear();
    operators.clear();
    operands.clear();
    parser.scanner.backtrack(start);
    allowSlash = parser.lookingAtNumber();
    singleExpression = parser.readSingleExpression();
  }

  void ExpressionParser::resolveOneOperation()
  {
    enum SassOperator op = operators.back();
    operators.pop_back();
    if (op != SassOperator::DIV) allowSlash = false;
    if (allowSlash && parser.inParentheses) allowSlash = false;
    Expression* lhs = operands.back();
    singleExpression = SASS_MEMORY_NEW(BinaryOpExpression,
      SourceSpan::delta(lhs->pstate(),
        singleExpression->pstate()),
      op, lhs, singleExpression,
      allowSlash);
    operands.pop_back();
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
		  allowSlash = number;
	  }
	  else if (!number) {
		  allowSlash = false;
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
        SassOperator::IESEQ, singleEqualsOperand, singleExpression);
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

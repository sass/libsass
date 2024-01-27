/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast.hpp"

#include "cssize.hpp"
#include "inspect.hpp"
#include "exceptions.hpp"
#include "dart_helpers.hpp"

#include "debugger.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Needs to be in sync with SassOp enum
  uint8_t SassOpPresedence[15] = {
    1, 2, 3, 3, 4, 4, 4, 4,
    5, 5, 6, 6, 6, 9, 255
  };

  // Needs to be in sync with SassOp enum
  const char* SassOpName[16] = {
    "or", "and", "eq", "neq", "gt", "gte", "lt", "lte",
    "plus", "minus", "times", "div", "mod", "seq", "ieseq", "invalid"
  };

  // Needs to be in sync with SassOp enum
  const char* SassOpOperator[16] = {
    "||", "&&", "==", "!=", ">", ">=", "<", "<=",
    "+", "-", "*", "/", "%", "=", "=", "invalid"
  };

  // Precedence is used to decide order
  // in ExpressionParser::addOperator.
  uint8_t sass_op_to_precedence(enum SassOperator op)
  {
    return SassOpPresedence[op];
  }

  // Get readable name for error messages
  const char* sass_op_to_name(enum SassOperator op)
  {
    return SassOpName[op];
  }

  // Get readable name for operator (e.g. `==`)
  const char* sass_op_separator(enum SassOperator op)
  {
    return SassOpOperator[op];
  }

  // Get readable name for list operator (e.g. `,`, `/` or ` `)
  const char* sass_list_separator(enum SassSeparator op)
  {
    switch (op) {
    case SASS_COMMA: return ", ";
    case SASS_SPACE: return " ";
    case SASS_DIV: return " / ";
    default: return "";
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  sass::string Selector::inspect(int precision) const
  {
    OutputOptions out(
      SASS_STYLE_NESTED,
      precision);
    Inspect i(out);
    i.inspect = true;
    // Inspect must be const, accept isn't
    const_cast<Selector*>(this)->accept(&i);
    return i.get_buffer();
  }

  sass::string Value::inspect(int precision, bool quotes) const
  {
    OutputOptions out(
      SASS_STYLE_NESTED,
      precision);
    Inspect i(out);
    i.inspect = true;
    i.quotes = quotes;
    // Inspect must be const, accept isn't
    const_cast<Value*>(this)->accept(&i);
    return i.get_buffer();
  }

  AstNode* Value::simplify(Logger& logger) {
    callStackFrame frame(logger, pstate());
    throw Exception::SassScriptException(logger, pstate(),
      "Value " + inspect() + " can't be used in a calculation.");
  }

  sass::string Value::toCss(bool quote) const
  {
    OutputOptions out(
      SASS_STYLE_TO_CSS,
      SassDefaultPrecision);
    Cssize i(out);
    i.quotes = quote;
    // Inspect must be const, accept isn't
    const_cast<Value*>(this)->accept(&i);
    return i.get_buffer();
  }

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // Only used for nth sass function
  // Single values act like lists with 1 item
  // Doesn't allow overflow of index (throw error)
  // Allows negative index but no overflow either
  Value* Value::getValueAt(Value* index, Logger& logger)
  {
    // Check out of boundary access
    sassIndexToListIndex(index, logger, "n");
    // Return single value
    return this;
  }

  // Only used for nth sass function
  // Doesn't allow overflow of index (throw error)
  // Allows negative index but no overflow either
  Value* Map::getValueAt(Value* index, Logger& logger)
  {
    return getPairAsList(sassIndexToListIndex(index, logger, "n"));
  }

  // Search the position of the given value
  size_t List::indexOf(Value* value) {
    return Sass::indexOf(elements(), value);
  }

  // Only used for nth sass function
  // Doesn't allow overflow of index (throw error)
  // Allows negative index but no overflow either
  Value* List::getValueAt(Value* index, Logger& logger)
  {
    return get(sassIndexToListIndex(index, logger, "n"));
  }

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

}

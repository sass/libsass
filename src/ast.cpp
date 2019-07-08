/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast.hpp"

#include "cssize.hpp"
#include "inspect.hpp"
#include "dart_helpers.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Needs to be in sync with SassOp enum
  uint8_t SassOpPresedence[15] = {
    1, 2, 3, 3, 4, 4, 4, 4,
    5, 5, 6, 6, 6, 9, 255
  };

  // Needs to be in sync with SassOp enum
  const char* SassOpName[15] = {
    "or", "and", "eq", "neq", "gt", "gte", "lt", "lte",
    "plus", "minus", "times", "div", "mod", "seq", "invalid"
  };

  // Needs to be in sync with SassOp enum
  const char* SassOpOperator[15] = {
    "||", "&&", "==", "!=", ">", ">=", "<", "<=",
    "+", "-", "*", "/", "%", "=", "invalid"
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

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  sass::string Selector::inspect(int precision) const
  {
    SassOutputOptionsCpp out({
      SASS_STYLE_NESTED,
      precision });
    Inspect i(out, false);
    i.inspect = true;
    // Inspect must be const, accept isn't
    const_cast<Selector*>(this)->accept(&i);
    return i.get_buffer();
  }

  sass::string Value::inspect(int precision, bool quotes) const
  {
    SassOutputOptionsCpp out({
      SASS_STYLE_NESTED,
      precision });
    Inspect i(out, false);
    i.inspect = true;
    i.quotes = quotes;
    // Inspect must be const, accept isn't
    const_cast<Value*>(this)->accept(&i);
    return i.get_buffer();
  }

  sass::string Value::toCss(Logger& logger, bool quote) const
  {
    SassOutputOptionsCpp out({
      SASS_STYLE_TO_CSS,
      SassDefaultPrecision });
    Cssize i(out, false);
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

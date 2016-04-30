#ifndef SASS_AST_H
#define SASS_AST_H

#include <set>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <unordered_map>
#include "sass/base.h"

#ifdef __clang__

/*
 * There are some overloads used here that trigger the clang overload
 * hiding warning. Specifically:
 *
 * Type type() which hides string type() from Expression
 *
 * and
 *
 * Block* block() which hides virtual Block* block() from Statement
 *
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"

#endif

#include "util.hpp"
#include "units.hpp"
#include "context.hpp"
#include "position.hpp"
#include "constants.hpp"
#include "operation.hpp"
#include "position.hpp"
#include "inspect.hpp"
#include "source_map.hpp"
#include "environment.hpp"
#include "error_handling.hpp"
#include "ast_def_macros.hpp"
#include "ast_fwd_decl.hpp"
#include "source_map.hpp"

#include "sass.h"

#include "ast/common.hpp"
#include "ast/nodes.hpp"
#include "ast/containers.hpp"
#include "ast/statements.hpp"
#include "ast/expressions.hpp"
#include "ast/blocks.hpp"
#include "ast/values.hpp"
#include "ast/selectors.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////
  // Individual parameter objects for mixins and functions.
  /////////////////////////////////////////////////////////
  class Parameter : public AST_Node {
    ADD_PROPERTY(std::string, name)
    ADD_PROPERTY(Expression*, default_value)
    ADD_PROPERTY(bool, is_rest_parameter)
  public:
    Parameter(ParserState pstate,
              std::string n, Expression* def = 0, bool rest = false)
    : AST_Node(pstate), name_(n), default_value_(def), is_rest_parameter_(rest)
    {
      if (default_value_ && is_rest_parameter_) {
        error("variable-length parameter may not have a default value", pstate);
      }
    }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////////////////////////////////////////
  // Parameter lists -- in their own class to facilitate context-sensitive
  // error checking (e.g., ensuring that all optional parameters follow all
  // required parameters).
  /////////////////////////////////////////////////////////////////////////
  class Parameters : public AST_Node, public Vectorized<Parameter*> {
    ADD_PROPERTY(bool, has_optional_parameters)
    ADD_PROPERTY(bool, has_rest_parameter)
  protected:
    void adjust_after_pushing(Parameter* p)
    {
      if (p->default_value()) {
        if (has_rest_parameter_) {
          error("optional parameters may not be combined with variable-length parameters", p->pstate());
        }
        has_optional_parameters_ = true;
      }
      else if (p->is_rest_parameter()) {
        if (has_rest_parameter_) {
          error("functions and mixins cannot have more than one variable-length parameter", p->pstate());
        }
        has_rest_parameter_ = true;
      }
      else {
        if (has_rest_parameter_) {
          error("required parameters must precede variable-length parameters", p->pstate());
        }
        if (has_optional_parameters_) {
          error("required parameters must precede optional parameters", p->pstate());
        }
      }
    }
  public:
    Parameters(ParserState pstate)
    : AST_Node(pstate),
      Vectorized<Parameter*>(),
      has_optional_parameters_(false),
      has_rest_parameter_(false)
    { }
    ATTACH_OPERATIONS()
  };

}

#ifdef __clang__

#pragma clang diagnostic pop

#endif

#endif

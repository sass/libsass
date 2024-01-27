/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_supports.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // The abstract superclass of all Supports conditions.
  /////////////////////////////////////////////////////////////////////////

  SupportsCondition::SupportsCondition(
    const SourceSpan& pstate) :
    AstNode(pstate)
  {}

  /////////////////////////////////////////////////////////////////////////
  // An operator condition (e.g. `CONDITION1 and CONDITION2`).
  /////////////////////////////////////////////////////////////////////////

  SupportsOperation::SupportsOperation(
    const SourceSpan& pstate,
    SupportsCondition* lhs,
    SupportsCondition* rhs,
    Operand operand) :
    SupportsCondition(pstate),
    left_(lhs),
    right_(rhs),
    operand_(operand)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A supports function
  /////////////////////////////////////////////////////////////////////////

  SupportsFunction::SupportsFunction(
    const SourceSpan& pstate,
    Interpolation* name,
    Interpolation* args) :
    SupportsCondition(pstate),
    name_(name),
    args_(args)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A supports anything
  /////////////////////////////////////////////////////////////////////////

  SupportsAnything::SupportsAnything(
    const SourceSpan& pstate,
    Interpolation* contents) :
    SupportsCondition(pstate),
    contents_(contents)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A negation condition (`not CONDITION`).
  /////////////////////////////////////////////////////////////////////////

  SupportsNegation::SupportsNegation(
    const SourceSpan& pstate,
    SupportsCondition* condition) :
    SupportsCondition(pstate),
    condition_(condition)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A declaration condition (e.g. `(feature: value)`).
  /////////////////////////////////////////////////////////////////////////

  SupportsDeclaration::SupportsDeclaration(
    const SourceSpan& pstate,
    Expression* feature,
    Expression* value)
  : SupportsCondition(pstate),
    feature_(feature),
    value_(value)
  {}

  bool SupportsDeclaration::isCustomProperty() const
  {
    if (const auto& exp = feature_->isaStringExpression()) {
      if (exp->hasQuotes() == false) {
        const auto& text = exp->text()->getInitialPlain();
        return StringUtils::startsWith(text, "--", 2);
      }
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  // An interpolation condition (e.g. `#{$var}`).
  /////////////////////////////////////////////////////////////////////////

  SupportsInterpolation::SupportsInterpolation(
    const SourceSpan& pstate,
    Expression* value) :
    SupportsCondition(pstate),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

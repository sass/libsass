/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_SUPPORTS_HPP
#define SASS_AST_SUPPORTS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_expressions.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // The abstract superclass of all Supports conditions.
  /////////////////////////////////////////////////////////////////////////

  class SupportsCondition : public AstNode
  {
  public:

    // Value constructor
    SupportsCondition(
      const SourceSpan& pstate);

    // Declare up-casting methods
    DECLARE_ISA_CASTER(SupportsOperation);
    DECLARE_ISA_CASTER(SupportsFunction);
    DECLARE_ISA_CASTER(SupportsAnything);
    DECLARE_ISA_CASTER(SupportsNegation);
    DECLARE_ISA_CASTER(SupportsDeclaration);
    DECLARE_ISA_CASTER(SupportsInterpolation);
  };

  /////////////////////////////////////////////////////////////////////////
  // An operator condition (e.g. `CONDITION1 and CONDITION2`).
  /////////////////////////////////////////////////////////////////////////

  class SupportsOperation final : public SupportsCondition
  {
  public:

    enum Operand { AND, OR };

  private:

    ADD_CONSTREF(SupportsConditionObj, left);
    ADD_CONSTREF(SupportsConditionObj, right);
    ADD_CONSTREF(Operand, operand);

  public:

    // Value constructor
    SupportsOperation(
      const SourceSpan& pstate,
      SupportsCondition* lhs,
      SupportsCondition* rhs,
      Operand operand);

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsOperation);
  };

  /////////////////////////////////////////////////////////////////////////
  // A supports function
  /////////////////////////////////////////////////////////////////////////

  class SupportsFunction final : public SupportsCondition
  {
  private:

    ADD_CONSTREF(InterpolationObj, name);
    ADD_CONSTREF(InterpolationObj, args);

  public:

    // Value constructor
    SupportsFunction(
      const SourceSpan& pstate,
      Interpolation* name,
      Interpolation* args);

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsFunction);
  };

  /////////////////////////////////////////////////////////////////////////
  // A supports anything condition
  /////////////////////////////////////////////////////////////////////////

  class SupportsAnything final : public SupportsCondition
  {

  private:

    ADD_CONSTREF(InterpolationObj, contents);

  public:

    // Value constructor
    SupportsAnything(
      const SourceSpan& pstate,
      Interpolation* contents);

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsAnything);
  };

  /////////////////////////////////////////////////////////////////////////
  // A negation condition (`not CONDITION`).
  /////////////////////////////////////////////////////////////////////////

  class SupportsNegation final : public SupportsCondition
  {

  private:

    ADD_CONSTREF(SupportsConditionObj, condition);

  public:

    // Value constructor
    SupportsNegation(
      const SourceSpan& pstate,
      SupportsCondition* condition);

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsNegation);
  };

  /////////////////////////////////////////////////////////////////////////
  // A declaration condition (e.g. `(feature: value)`).
  /////////////////////////////////////////////////////////////////////////
  class SupportsDeclaration final : public SupportsCondition
  {
  private:

    ADD_CONSTREF(ExpressionObj, feature);
    ADD_CONSTREF(ExpressionObj, value);

  public:

    // Value constructor
    SupportsDeclaration(
      const SourceSpan& pstate,
      Expression* feature,
      Expression* value);

    bool isCustomProperty() const;

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsDeclaration);
  };

  /////////////////////////////////////////////////////////////////////////
  // An interpolation condition (e.g. `#{$var}`).
  /////////////////////////////////////////////////////////////////////////

  class SupportsInterpolation final : public SupportsCondition
  {
  private:

    ADD_CONSTREF(ExpressionObj, value);

  public:

    // Value constructor
    SupportsInterpolation(
      const SourceSpan& pstate,
      Expression* value);

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(SupportsInterpolation);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

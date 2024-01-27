/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "extension.hpp"

#include "callstack.hpp"
#include "ast_helpers.hpp"
#include "exceptions.hpp"
#include "ast_css.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Static function to create a copy with a new extender
  /////////////////////////////////////////////////////////////////////////
  Extension* Extension::withExtender(ComplexSelectorObj& newExtender) const
  {
    return SASS_MEMORY_NEW(Extension,
      newExtender->pstate(),
      newExtender, target, mediaContext, isOptional);
  }


  // Creates a one-off extension that's not intended to be modified over time.
  // If [specificity] isn't passed, it defaults to `extender.maxSpecificity`.

  Extension::Extension(
    const SourceSpan& pstate,
    ComplexSelectorObj& extender,
    const SimpleSelectorObj& target,
    const CssMediaRuleObj& mediaContext,
    bool isOriginal, bool isOptional) :
    pstate(pstate),
    extender(pstate, extender, 0, isOriginal, mediaContext),
    target(target),
    specificity(extender->maxSpecificity()),
    isOptional(isOptional),
    isOriginal(isOriginal),
    isConsumed(false),
    mediaContext(mediaContext)
  {
  }


//  Extension::Extension(Extender extender) :
//    extender(extender),
//    specificity(0),
//    isOptional(true),
//    isOriginal(false),
//    isConsumed(false)
//  {}
//
  // Copy constructor

  Extension::Extension(const Extension & extension) :
    extender(extension.extender),
    target(extension.target),
    specificity(extension.specificity),
    isOptional(extension.isOptional),
    isOriginal(extension.isOriginal),
    isConsumed(extension.isConsumed),
    mediaContext(extension.mediaContext)
  {}

  Extension::Extension() :
    extender(SourceSpan::internal("Ext"), {}, 0, false),
    specificity(0),
    isOptional(false),
    isOriginal(false),
    isConsumed(false)
  {}

  Extension& Extension::operator=(const Extension& other)
  {
    extender = other.extender;
    target = other.target;
    specificity = other.specificity;
    isOptional = other.isOptional;
    isOriginal = other.isOriginal;
    isConsumed = other.isConsumed;
    mediaContext = other.mediaContext;
    return *this;
  }

  /////////////////////////////////////////////////////////////////////////
  // Asserts that the [mediaContext] for a selector is
  // compatible with the query context for this extender.
  /////////////////////////////////////////////////////////////////////////
  void Extension::assertCompatibleMediaContext(CssMediaRuleObj mediaQueryContext, BackTraces& traces) const
  {

    if (this->mediaContext.isNull()) return;

    if (mediaQueryContext && mediaContext == mediaQueryContext) return;

    if (ObjEqualityFn<CssMediaRuleObj>(mediaQueryContext, mediaContext)) return;

    throw Exception::ExtendAcrossMedia(traces, this);

  }

  /////////////////////////////////////////////////////////////////////////
  // Asserts that the [mediaContext] for a selector is
  // compatible with the query context for this extender.
  /////////////////////////////////////////////////////////////////////////
  void Extender::assertCompatibleMediaContext(CssMediaRuleObj mediaQueryContext, BackTraces& traces) const
  {

    if (this->mediaContext.isNull()) return;

    if (mediaQueryContext && mediaContext == mediaQueryContext) return;

    if (ObjEqualityFn<CssMediaRuleObj>(mediaQueryContext, mediaContext)) return;

    throw Exception::ExtendAcrossMedia(traces, this);

  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_EXTENSION_HPP
#define SASS_EXTENSION_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"
#include "ast_selectors.hpp"
#include "backtrace.hpp"

namespace Sass {

  class Extender {
  public:

    // The span in which this selector was defined.
    SourceSpan pstate; // use from extender

    // The selector in which the `@extend` appeared.
    ComplexSelectorObj selector;

    // The minimum specificity required for any
    // selector generated from this extender.
    size_t specificity;

    // Whether this extender represents a selector that was originally
    // in the document, rather than one defined with `@extend`.
    bool isOriginal;

    // The extension that created this [Extender]. Not all [Extender]s
    // are created by extensions. Some simply represent the
    // original selectors that exist in the document.
    Extension* extension;

    // The media query context to which this extend is restricted,
    // or `null` if it can apply within any context.
    CssMediaRuleObj mediaContext;

    // Value constructor
    Extender(
      const SourceSpan& pstate, 
      ComplexSelector* extender,
      size_t specificity,
      bool isOriginal,
      CssMediaRuleObj media = {}) :
      pstate(pstate),
      selector(extender),
      specificity(specificity),
      isOriginal(isOriginal),
      extension(nullptr),
      mediaContext(media)
    {}

    Extender() :
      selector({}),
      specificity(0),
      isOriginal(false),
      extension(nullptr)
    {}

    // Asserts that the [mediaContext] for a selector is 
    // compatible with the query context for this extender.
    void assertCompatibleMediaContext(CssMediaRuleObj mediaContext, BackTraces& traces) const;

  };

  class Extension : public RefCounted {

  public:

    sass::string toString() const;

    SourceSpan pstate;

    // The selector in which the `@extend` appeared.
    Extender extender;

    // The selector that's being extended.
    // `null` for one-off extensions.
    SimpleSelectorObj target;

    // The minimum specificity required for any
    // selector generated from this extender.
    size_t specificity;

    // Whether this extension is optional.
    bool isOptional;

    // Whether this is a one-off extender representing a selector that was
    // originally in the document, rather than one defined with `@extend`.
    bool isOriginal;

    // Whether or not this extension was consumed.
    bool isConsumed;

    // The media query context to which this extend is restricted,
    // or `null` if it can apply within any context.
    CssMediaRuleObj mediaContext;

    // Creates a one-off extension that's not intended to be modified over time.
    // If [specificity] isn't passed, it defaults to `extender.maxSpecificity`.
    Extension(
      const SourceSpan& pstate,
      ComplexSelectorObj& extender,
      const SimpleSelectorObj& target,
      const CssMediaRuleObj& mediaContext = {},
      bool isOptional = false,
      bool isOriginal = true);

    // Copy constructor
    Extension(const Extension& extension);

    // Default constructor
    Extension();

    // Copy assignment operator
    Extension& operator=(const Extension& other);

    // Asserts that the [mediaContext] for a selector is 
    // compatible with the query context for this extender.
    void assertCompatibleMediaContext(CssMediaRuleObj mediaContext, BackTraces& traces) const;

    Extension* withExtender(ComplexSelectorObj& newExtender) const;

  };

}

#endif

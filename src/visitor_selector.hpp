/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VISITOR_SELECTOR_HPP
#define SASS_VISITOR_SELECTOR_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

namespace Sass {

  // An interface for [visitors][] that traverse selectors.
  // [visitors]: https://en.wikipedia.org/wiki/Visitor_pattern

  template <typename T>
  // AnySelectorVisitor
  class SelectorVisitor {
  public:

    virtual T visitAttributeSelector(AttributeSelector* attribute) = 0;
    virtual T visitClassSelector(ClassSelector* klass) = 0;
    virtual T visitComplexSelector(ComplexSelector* complex) = 0;
    virtual T visitCompoundSelector(CompoundSelector* compound) = 0;
    virtual T visitIDSelector(IDSelector* id) = 0;
    virtual T visitPlaceholderSelector(PlaceholderSelector* placeholder) = 0;
    virtual T visitPseudoSelector(PseudoSelector* pseudo) = 0;
    virtual T visitSelectorList(SelectorList* list) = 0;
    virtual T visitTypeSelector(TypeSelector* type) = 0;
    // The following two types have been optimized out in libsass
    // virtual T visitParentSelector(ParentSelector* parent) = 0;
    // virtual T visitUniversalSelector(UniversalSelector* universal) = 0;
    // virtual T visitSelectorCombinator(SelectorCombinator* combinator) = 0;

  };

  template <typename T>
  class SelectorVisitable {
  public:
    virtual T accept(SelectorVisitor<T>* visitor) = 0;
  };

}

#endif

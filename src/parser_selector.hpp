/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_SELECTOR_HPP
#define SASS_PARSER_SELECTOR_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class SelectorParser final : public Parser
  {
  public:

    // Whether this parser allows the parent selector `&`.
    bool allowParent;

    // Whether this parser allows placeholder selectors beginning with `%`.
    bool allowPlaceholder;

    // Value constructor
    SelectorParser(
      Compiler& context,
      SourceDataObj source,
      bool allowParent = true,
      bool allowPlaceholder = true) :
      Parser(context, source),
      allowParent(allowParent),
      allowPlaceholder(allowPlaceholder)
    {}

    // Parse content into selector list
    // Throws if not everything is consumed
    SelectorList* parseSelectorList();

    // Parse content into compound selector
    // Throws if not everything is consumed
    CompoundSelector* parseCompoundSelector();

    // Parse content into simple selector
    // Throws if not everything is consumed
    SimpleSelector* parseSimpleSelector();

  private:

    // Consumes a selector list.
    SelectorList* readSelectorList();

    // Consumes a complex selector.
    ComplexSelector* readComplexSelector(bool lineBreak = false);

    // Consumes a compound selector.
    CompoundSelector* readCompoundSelector();

    // Consumes a simple selector.
    SimpleSelector* readSimpleSelector(bool allowParent);

    // Consumes an attribute selector.
    AttributeSelector* readAttributeSelector();

    // Consumes an attribute name.
    struct QualifiedName readAttributeName();

    // Consumes an attribute operator.
    sass::string readAttributeOperator();

    // Consumes a class operator.
    ClassSelector* readClassSelector();

    // Consumes an in operator.
    IDSelector* readIdSelector();

    // Consumes a placeholder operator.
    PlaceholderSelector* readPlaceholderSelector();

    // Consumes a pseudo operator.
    PseudoSelector* readPseudoSelector();

    // Consumes an `an+b` expression.
    sass::string readAnPlusB();

    // Consumes a type of universal (simple) selector.
    SimpleSelector* readTypeOrUniversalSelector();

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

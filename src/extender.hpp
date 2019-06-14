#ifndef SASS_EXTENDER_H
#define SASS_EXTENDER_H

#include <set>
#include <map>
#include <string>

#include "ast_helpers.hpp"
#include "ast_fwd_decl.hpp"
#include "operation.hpp"
#include "extension.hpp"
#include "backtrace.hpp"
#include "ordered_map.hpp"

namespace Sass {


  typedef std::tuple<
    SelectorListObj, // modified
    SelectorListObj // original
  > ExtSelTuple;

  // This is special (ptrs!)
  typedef std::unordered_set<
    ComplexSelectorObj,
    ObjPtrHash,
    ObjPtrEquality
  > ExtCplxSelSet;

  typedef std::unordered_set<
    SimpleSelectorObj,
    ObjHash,
    ObjEquality
  > ExtSmplSelSet;

  typedef std::unordered_set<
    SelectorListObj,
    ObjPtrHash,
    ObjPtrEquality
  > ExtListSelSet;

  typedef std::unordered_map<
    SimpleSelectorObj,
    ExtListSelSet,
    ObjHash,
    ObjEquality
  > ExtSelMap;

  typedef ordered_map<
    ComplexSelectorObj,
    Extension,
    ObjHash,
    ObjEquality
  > ExtSelExtMapEntry;

  typedef std::unordered_map<
    SimpleSelectorObj,
    ExtSelExtMapEntry,
    ObjHash,
    ObjEquality
  > ExtSelExtMap;

  typedef std::unordered_map <
    SimpleSelectorObj,
    std::vector<
      Extension
    >,
    ObjHash,
    ObjEquality
  > ExtByExtMap;


  class Extender : public Operation_CRTP<void, Extender> {

  public:

    enum ExtendMode { TARGETS, REPLACE, NORMAL, };

  public:

    // The mode that controls this extender's behavior.
    ExtendMode mode;

    // Shared backtraces with context and expander. Needed the throw
    // errors when e.g. extending accross media query boundaries.
    Backtraces& traces;

    // A map from all simple selectors in the stylesheet to the rules that
    // contain them.This is used to find which rules an `@extend` applies to.
    ExtSelMap selectors;

    // A map from all extended simple selectors
    // to the sources of those extensions.
    ExtSelExtMap extensions;

    // A map from all simple selectors in extenders to
    // the extensions that those extenders define.
    ExtByExtMap extensionsByExtender;

    // A map from CSS rules to the media query contexts they're defined in.
    // This tracks the contexts in which each style rule is defined.
    // If a rule is defined at the top level, it doesn't have an entry.
    ordered_map<
      SelectorListObj,
      CssMediaRule_Obj,
      ObjPtrHash,
      ObjPtrEquality
    > mediaContexts;

    
    std::unordered_map<
      SimpleSelectorObj,
      size_t,
      ObjHash,
      ObjEquality
    > sourceSpecificity;

    // A set of [ComplexSelector]s that were originally part of their
    // component [SelectorList]s, as opposed to being added by `@extend`.
    // This allows us to ensure that we don't trim any selectors
    // that need to exist to satisfy the [first law of extend][].
    ExtCplxSelSet originals;


  public:

    Extender(Backtraces& traces);;
    ~Extender() {};


    Extender(ExtendMode mode, Backtraces& traces);

    ExtSmplSelSet getSimpleSelectors() const;

  public:
    static SelectorListObj extend(SelectorListObj selector, SelectorListObj source, SelectorListObj target, Backtraces& traces);
    static SelectorListObj replace(SelectorListObj selector, SelectorListObj source, SelectorListObj target, Backtraces& traces);

    // Extends [list] using [extensions].
    /*, List<CssMediaQuery> mediaQueryContext*/
    void addExtension(SelectorListObj extender, SimpleSelectorObj target, ExtendRule_Obj extend, CssMediaRule_Obj mediaQueryContext);
    SelectorListObj extendList(SelectorListObj list, ExtSelExtMap& extensions, CssMediaRule_Obj mediaContext);

    void extendExistingStyleRules(
      ExtListSelSet& rules,
      ExtSelExtMap& newExtensions);

    ExtSelExtMap extendExistingExtensions(
      std::vector<Extension> extensions,
      ExtSelExtMap& newExtensions);


    size_t maxSourceSpecificity(SimpleSelectorObj simple);
    size_t maxSourceSpecificity(CompoundSelectorObj compound);
    Extension extensionForSimple(SimpleSelectorObj simple);
    Extension extensionForCompound(std::vector<SimpleSelectorObj> simples);


    std::vector<ComplexSelectorObj> extendComplex(ComplexSelectorObj list, ExtSelExtMap& extensions, CssMediaRule_Obj mediaQueryContext);
    std::vector<ComplexSelectorObj> extendCompound(CompoundSelectorObj compound, ExtSelExtMap& extensions, CssMediaRule_Obj mediaQueryContext, bool inOriginal = false);
    std::vector<std::vector<Extension>> extendSimple(SimpleSelectorObj simple, ExtSelExtMap& extensions, CssMediaRule_Obj mediaQueryContext, ExtSmplSelSet* targetsUsed);

    std::vector<Pseudo_Selector_Obj> extendPseudo(Pseudo_Selector_Obj pseudo, ExtSelExtMap& extensions, CssMediaRule_Obj mediaQueryContext);

    std::vector<ComplexSelectorObj> trim(std::vector<ComplexSelectorObj> selectors, ExtCplxSelSet& set);
    


  private:
    std::vector<Extension> extendWithoutPseudo(SimpleSelectorObj simple, ExtSelExtMap& extensions, ExtSmplSelSet* targetsUsed);
    static SelectorListObj extendOrReplace(SelectorListObj selector, SelectorListObj source, SelectorListObj target, ExtendMode mode, Backtraces& traces);

  public:

    // An [Extender] that contains no extensions and can have no extensions added.
    // static const empty = EmptyExtender();

    // A map from all simple selectors in the
    // stylesheet to the rules that contain them.
    // This is used to find which rules an `@extend` applies to.
    // std::map<SimpleSelectorObj, Set<ModifiableCssStyleRule>> _selectors;

    // A map from all extended simple selectors to the sources of those extensions.
    // std::map<SimpleSelectorObj, std::map<ComplexSelectorObj, Extension, OrderNodes>> extensions;

    // A map from all simple selectors in extenders to the extensions that those extenders define.
    // std::map<SimpleSelectorObj, std::vector<Extension>> extensionsByExtender;

    /// A set of [ComplexSelector]s that were originally part of
    /// their component [SelectorList]s, as opposed to being added by `@extend`.
    ///
    /// This allows us to ensure that we don't trim any selectors that need to
    /// exist to satisfy the [first law of extend][].
    ///
    /// [first law of extend]: https://github.com/sass/sass/issues/324#issuecomment-4607184
    // std::set<ComplexSelectorObj> originals;





    // Adds [selector] to this extender, with [selectorSpan] as the span covering
    // the selector and [ruleSpan] as the span covering the entire style rule.
    // Extends [selector] using any registered extensions, then returns an empty
    // [ModifiableCssStyleRule] with the resulting selector. If any more relevant
    // extensions are added, the returned rule is automatically updated.
    // The [mediaContext] is the media query context in which the selector was
    // defined, or `null` if it was defined at the top level of the document.
    void addSelector(SelectorListObj selector, CssMediaRule_Obj mediaContext);

    // Registers the [SimpleSelector]s in [list]
    // to point to [rule] in [selectors].
    void registerSelector(SelectorListObj list, SelectorListObj rule);


  };

}

#endif

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "extender.hpp"

#include "permutate.hpp"
#include "callstack.hpp"
#include "exceptions.hpp"
#include "dart_helpers.hpp"
#include "ast_selectors.hpp"
#include "ast_helpers.hpp"
#include "ast_css.hpp"

#include "debugger.hpp"

namespace Sass {

  template <typename T>
  sass::string VecToString(sass::vector<T> exts) {
    sass::string msg = "[";
    bool joiner = false;
    for (auto& entry : exts) {
      if (joiner) msg += ", ";
      msg += entry.selector->inspect();
      joiner = true;
    }
    return msg + "]";

  }


  template <typename T>
  sass::string VecVecToString(sass::vector<T> exts) {
    sass::string msg = "[";
    bool joiner = false;
    for (auto& entry : exts) {
      if (joiner) msg += ", ";
      msg += VecToString(entry);
      joiner = true;
    }
    return msg + "]";

  }

  template <typename T>
  sass::string VecToString2(sass::vector<T> exts) {
    sass::string msg = "[";
    bool joiner = false;
    for (auto& entry : exts) {
      if (joiner) msg += ", ";
      msg += entry->inspect();
      joiner = true;
    }
    return msg + "]";

  }

  template <typename T>
  sass::string VecVecToString2(sass::vector<T> exts) {
    sass::string msg = "[";
    bool joiner = false;
    for (auto& entry : exts) {
      if (joiner) msg += ", ";
      msg += VecToString2(entry);
      joiner = true;
    }
    return msg + "]";

  }


  sass::string ExtSelToStr(const ExtSelExtMap& map) {
    sass::string msg;
    for (auto& entry : map) {
      msg += entry.first->inspect();
      msg += ": { ";
      for (auto& pair : entry.second) {
        ComplexSelector* sel = pair.first;
        Extension* extension = pair.second;
        msg += sel->inspect();
        msg += ": ";
        msg += extension->toString();
      }
      msg += " }";
      msg += ", ";
    }
    return msg;
  }

  sass::string ExtensionStore::toString() {
    sass::string msg;
    for (auto& entry : extensionsBySimpleSelector) {
      msg += entry.first->inspect();
      msg += ": { ";
      for (auto& pair : entry.second) {
        ComplexSelector* sel = pair.first;
        Extension* extension = pair.second;
        msg += sel->inspect();
        msg += ": ";
        msg += extension->toString();
      }
      msg += " }";
      msg += ", ";
    }
    return msg;
  }

  sass::string Extension::toString() const {
    sass::string msg;
    msg += extender.selector->inspect();
    msg += "{@extend ";
    msg += target->inspect();
    if (isOptional) {
      msg += " !optional";
    }
    msg += "}";
    return msg;
  }

  /////////////////////////////////////////////////////////////////////////
  // Constructor with specific [mode].
  // [traces] are needed to throw errors.
  /////////////////////////////////////////////////////////////////////////
  ExtensionStore::ExtensionStore(ExtendMode mode, BackTraces& traces) :
    mode(mode),
    traces(&traces),
    selectors54(),
//    extensionsBySimpleSelector(),
    extensionsByExtender(),
    mediaContexts(),
    sourceSpecificity(),
    originals()
  {
    // std::cerr << "!! Create extension store\n";
  }

  ExtensionStore::ExtensionStore() :
    mode(NORMAL),
    traces(nullptr),
    selectors54(),
    //    extensionsBySimpleSelector(),
    extensionsByExtender(),
    mediaContexts(),
    sourceSpecificity(),
    originals()
  {
    // std::cerr << "!! Create empty extension store\n";
  }

  // extensionsWhereTarget((target) = > !originalSelectors.contains(target))
  void ExtensionStore::addNonOriginalSelectors(
    ExtSmplSelSet originalSelectors,
    ExtSet& unsatisfiedExtensions)
  {
    for (auto& entry : extensionsBySimpleSelector) {
      if (originalSelectors.count(entry.first)) continue;
      for (auto& extension : entry.second) {
        // ToDo: check why we have this here!?
        if (extension.second->isOptional) continue;
        if (extension.second->target.isNull()) {
          //std::cerr << "has no target, skip\n";
          continue;
        }
        unsatisfiedExtensions.insert(extension.second);
      }
    }
  }

  // extensionsWhereTarget((target) = > !originalSelectors.contains(target))
  void ExtensionStore::delNonOriginalSelectors(
    ExtSmplSelSet originalSelectors,
    ExtSet& unsatisfiedExtensions)
  {
    for (auto& entry : extensionsBySimpleSelector) {
//      std::cerr << "! Check " << entry.first->inspect() << "\n";
      if (!originalSelectors.count(entry.first)) continue;
      for (auto& extension : entry.second) {
//        std::cerr << "! Check ext " << extension.first->inspect() << "\n";
        if (extension.second.isNull()) continue;
        if (extension.second->isOptional) continue;
        // if (extension.second->target.isNull()) continue;
//        std::cerr << ">> Yield " << extension.second->toString() << "\n";
        unsatisfiedExtensions.erase(extension.second);
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////
  // Extends [selector] with [source] extender and [targets] extendees.
  // This works as though `source {@extend target}` were written in the
  // stylesheet, with the exception that [target] can contain compound
  // selectors which must be extended as a unit.
  /////////////////////////////////////////////////////////////////////////
  SelectorListObj ExtensionStore::extend(
    const SelectorListObj& selector,
    const SelectorListObj& source,
    const SelectorListObj& targets,
    Logger& logger)
  {
    return extendOrReplace(selector, source, targets, ExtendMode::TARGETS, logger);
  }
  // EO ExtensionStore::extend

  /////////////////////////////////////////////////////////////////////////
  // Returns a copy of [selector] with [targets] replaced by [source].
  /////////////////////////////////////////////////////////////////////////
  SelectorListObj ExtensionStore::replace(
    const SelectorListObj& selector,
    const SelectorListObj& source,
    const SelectorListObj& targets,
    Logger& logger)
  {
    return extendOrReplace(selector, source, targets, ExtendMode::REPLACE, logger);
  }
  // EO ExtensionStore::replace


  sass::string SetToString(ExtCplxSelSet& set) {
    sass::string msg;
    for (auto& item : set) {
      msg += item->inspect();
      msg += ", ";
    }
    return msg;
  }

  /////////////////////////////////////////////////////////////////////////
  // A helper function for [extend] and [replace].
  /////////////////////////////////////////////////////////////////////////
  SelectorListObj ExtensionStore::extendOrReplace(
    const SelectorListObj& selector,
    const SelectorListObj& source,
    const SelectorListObj& targets,
    const ExtendMode mode,
    Logger& logger)
  {

    // sass::vector<ComplexSelectorObj> results;

    ExtensionStoreObj extender = SASS_MEMORY_NEW(ExtensionStore, mode, logger);

    if (selector->isInvisible() == false) {
      for (auto& original : selector->elements())
        extender->originals.insert(original);
    }

    SelectorListObj results = selector; // SASS_MEMORY_COPY(selector);

    //std::cerr << "selector " << selector->inspect() << "\n";
    //std::cerr << "source " << source->inspect() << "\n";
    //std::cerr << "targets " << targets->inspect() << "\n";

    for (auto& complex : targets->elements()) {

      if (auto* compound = complex->getSingleCompound()) {

        ExtSelExtMap extensions;

        for (auto& simple : compound->elements()) {
          for (auto& src : source->elements()) {
            // std::cerr << "Add ext key " << simple->inspect() << "\n";
            extensions[simple][src] = SASS_MEMORY_NEW(Extension,
              complex->pstate(), src, simple, {}, false, true);
          }
        }

        // std::cerr << "1) extend " << ExtSelToStr(extensions) << "\n";
        extender->extendList(results, extensions, {}, results->elements());

      }
      else {
        throw Exception::RuntimeException(logger,
          "Can't extend complex selector " +
          complex->inspect() + ".");
      }
    }

    return results;
    /*
    sass::vector< CompoundSelectorObj> compoundTargets;

    for (auto& complex : targets->elements()) {
      if (complex->empty()) continue;
      if (complex->size() > 1) {
        throw Exception::RuntimeException(logger,
          "Can't extend complex selector " +
          complex->inspect() + ".");
      }
      if (CompoundSelector* compound = complex->first()->selector()) {
        compoundTargets.emplace_back(compound);
      }
    }

    ExtSelExtMap extensions;

    for (auto& compound : compoundTargets) {
      for (auto& simple : compound->elements()) {
        for (auto& complex : source->elements()) {
          extensions[simple][complex] =
            SASS_MEMORY_NEW(Extension,
              complex->pstate(),
              complex, simple, {}, false, true);
          // extensions[simple][complex]->specificity = complex->minSpecificity();
        }
      }
    }

    ExtensionStoreObj extender = SASS_MEMORY_NEW(ExtensionStore, mode, logger);

    if (!selector->isInvisible()) {
      for (auto& complex : selector->elements()) {
        extender->originals.insert(complex);
      }
    }

    // std::cerr << "Sel: " << selector->inspect() << "\n";
    // std::cerr << "Ext: " << ExtSelToStr(extensions) << "\n";
    // std::cerr << "Org: " << SetToString(extender->originals) << "\n";

    extender->extendList(selector, extensions,
      {}, selector->elements());

    return selector;
    */
  }
  // EO extendOrReplace

  /////////////////////////////////////////////////////////////////////////
  // The set of all simple selectors in style rules handled
  // by this extender. This includes simple selectors that
  // were added because of downstream extensions.
  /////////////////////////////////////////////////////////////////////////
  ExtSmplSelSet ExtensionStore::getSimpleSelectors() const
  {
    ExtSmplSelSet set;
    for (auto& entry : selectors54) {
      set.insert(entry.first);
    }
    return set;
  }
  // EO getSimpleSelectors

  /////////////////////////////////////////////////////////////////////////
  // Check for extends that have not been satisfied.
  // Returns true if any non-optional extension did not
  // extend any selector. Updates the passed reference
  // to point to that Extension for further analysis.
  /////////////////////////////////////////////////////////////////////////
  bool ExtensionStore::checkForUnsatisfiedExtends2(Extension& unsatisfied) const
  {
    //ExtSmplSelSet originals;
    //for (auto& entry : selectors) {
    //  originals.insert(entry.first);
    //}
    //for (auto& mod : upstreams)
    //

    // if (selectors.empty()) return false; // Remove?
    ExtSmplSelSet originals = getSimpleSelectors();


//    for (auto target : extensionsBySimpleSelector) {
//      SimpleSelector* key = target.first;
//      const ExtSelExtMapEntry& val = target.second;
//      for (auto qwe : val) {
//        auto asd = qwe.second;
//        if (asd->isOriginal) {
//          if (!asd->isConsumed) {
//            if (!asd->isOptional) {
//              unsatisfied = asd;
//              return true;
//            }
//          }
//        }
//      }
//      // if (originals.find(key) == originals.end()) {
      //   const Extension& extension = val.begin()->second;
      //   if (extension.isOptional) continue;
      //   unsatisfied = extension;
      //   return true;
      // }
//    }
    // for (auto asd : upstream)
    return false;
  }
  // EO checkUnsatisfiedExtends

  /////////////////////////////////////////////////////////////////////////
  // Returns an extension that combines [left] and [right]. Throws
  // a [SassException] if [left] and [right] have incompatible
  // media contexts. Throws an [ArgumentError] if [left]
  // and [right] don't have the same extender and target.
  /////////////////////////////////////////////////////////////////////////
  Extension* ExtensionStore::mergeExtension(
    Extension* lhs, Extension* rhs)
  {

    if (rhs == nullptr) return lhs;
    if (lhs == nullptr) return rhs;

    // If one extension is optional and doesn't add a
    // special media context, it doesn't need to be merged.
    if (rhs->isOptional && rhs->mediaContext.isNull()) return lhs;
    if (lhs->isOptional && lhs->mediaContext.isNull()) return rhs;

    Extension* rv = SASS_MEMORY_NEW(Extension, *lhs);
    // ToDo: is this right?
    rv->isOptional = true;
    rv->isOriginal = false;
    return rv;
  }
  // EO mergeExtension

  /////////////////////////////////////////////////////////////////////////
  // Helper function to copy extension between maps
  /////////////////////////////////////////////////////////////////////////
  // Seems only relevant for sass 4.0 modules
  /////////////////////////////////////////////////////////////////////////
  /* void mapCopyExts(
    ExtSelExtMap& dest,
    const ExtSelExtMap& source)
  {
    for (auto it : source) {
      SimpleSelectorObj key = it.first;
      ExtSelExtMapEntry& inner = it.second;
      ExtSelExtMap::iterator dmap = dest.find(key);
      if (dmap == dest.end()) {
        dest.insert(std::make_pair(key, inner));
      }
      else {
        ExtSelExtMapEntry& imap = dmap->second;
        // ToDo: optimize ordered_map API!
        // ToDo: we iterate and fetch the value
        for (ComplexSelectorObj& it2 : inner) {
          imap.insert(it2, inner.get(it2));
        }
      }
    }
  } */
  // EO mapCopyExts

  /////////////////////////////////////////////////////////////////////////
  // Adds [selector] to this extender, with [selectorSpan] as the span covering
  // the selector and [ruleSpan] as the span covering the entire style rule.
  // Extends [selector] using any registered extensions, then returns an empty
  // [ModifiableCssStyleRule] with the resulting selector. If any more relevant
  // extensions are added, the returned rule is automatically updated.
  // The [mediaContext] is the media query context in which the selector was
  // defined, or `null` if it was defined at the top level of the document.
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::addSelector(
    const SelectorListObj& selector,
    const CssMediaRuleObj& mediaContext)
  {

    // std::cerr << "add selector " << selector->inspect() << "\n";

    if (!selector->isInvisible()) {
      for (auto complex : selector->elements()) {
        originals.insert(complex);
      }
    }

    if (!extensionsBySimpleSelector.empty()) {
      extendList(selector, extensionsBySimpleSelector,
        mediaContext, selector->elements());
      // Dart-Sass upgrades error here
    }

    if (!mediaContext.isNull()) {
      mediaContexts[selector] = mediaContext;
    }

    _registerSelector(selector, selector);

    // Dart-Sass returns wrapped in modifiable
  }
  // EO addSelector (checked)

  /////////////////////////////////////////////////////////////////////////
  // Registers the [SimpleSelector]s in [list]
  // to point to [rule] in [selectors].
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::_registerSelector(
    const SelectorListObj& list,
    const SelectorListObj& rule,
    bool onlyPublic)
  {
    if (list.isNull() || list->empty()) return;
    // std::cerr << "Reg selector " << rule.ptr() << " - " << rule->inspect() << " => " << list->inspect() << "\n";
    for (auto complex : list->elements()) {
      for (auto component : complex->elements()) {
        if (auto compound = component->selector()) {
          for (const SimpleSelectorObj& simple : compound->elements()) {
            // Creating this structure can take up to 5%
            if (auto ph = simple->isaPlaceholderSelector()) {
              if (!onlyPublic || !ph->isPrivate93()) {
                selectors54[simple].insert(rule);
              }
            }
            else {
              selectors54[simple].insert(rule);
            }
            if (auto pseudo = simple->isaPseudoSelector()) {
              if (pseudo->selector()) {
                auto selectorInPseudo = pseudo->selector();
                //std::cerr << "Register inner pseudo\n";
                _registerSelector(selectorInPseudo, rule);
              }
            }
          }
        }
      }
    }
  }
  // EO _registerSelector (checked)

  /////////////////////////////////////////////////////////////////////////
  // Adds an extension to this extender. The [extender] is the selector for the
  // style rule in which the extension is defined, and [target] is the selector
  // passed to `@extend`. The [extend] provides the extend span and indicates
  // whether the extension is optional. The [mediaContext] defines the media query
  // context in which the extension is defined. It can only extend selectors
  // within the same context. A `null` context indicates no media queries.
  /////////////////////////////////////////////////////////////////////////
  // ToDo: rename extender to parent, since it is not involved in extending stuff
  // ToDo: check why dart sass passes the ExtendRule around (is this the main selector?)
  /////////////////////////////////////////////////////////////////////////
  // Note: this function could need some logic cleanup
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::addExtension(
    const SelectorListObj& extender,
    const SimpleSelectorObj& target,
    const CssMediaRuleObj& mediaContext,
    const ExtendRuleObj& extend,
    bool is_optional)
  {

    auto rules = selectors54.find(target);
    bool hasRule = rules != selectors54.end();

    ExtSelExtMapEntry newExtensions;

    // ToDo: we check this here first and fetch the same? item again after the loop!?
    bool hasExistingExtensions = extensionsByExtender.find(target) != extensionsByExtender.end();

    // Get existing extensions for the given target (SimpleSelector)
    ExtSelExtMapEntry& sources = extensionsBySimpleSelector[target];

    for (auto& complex : extender->elements()) {
      if (complex->isUseless()) continue;
      // std::cerr << "+++ " << complex->inspect() << "\n";
      ExtensionObj extension = SASS_MEMORY_NEW(Extension,
        extend->pstate(),
        complex, target, mediaContext, false, is_optional);
      auto existingExtension = sources.find(complex);
      if (existingExtension != sources.end()) {
        // If there's already an extend from [extender] to [target],
        // we don't need to re-run the extension. We may need to
        // mark the extension as mandatory, though.
        sources[complex] = mergeExtension(
          existingExtension.value(), extension);
        // ToDo: implement behavior once use case is found!?
        // ToDo: mergeExtension needs error checks etc.
        continue;
      }

      // std::cerr << "==== " << complex->inspect() << " adding " << extension->toString() << "\n";
      sources[complex] = extension;

      for (auto& component : complex->elements()) {
        if (auto compound = component->selector()) {
          for (auto& simple : compound->elements()) {
            extensionsByExtender[simple].emplace_back(extension);
            if (sourceSpecificity.find(simple) == sourceSpecificity.end()) {
              // Only source specificity for the original selector is relevant.
              // Selectors generated by `@extend` don't get new specificity.
              // ToDo: two map structures with the same key is inefficient
              sourceSpecificity[simple] = complex->maxSpecificity();
            }
          }
        }
      }

      if (hasRule || hasExistingExtensions) {
        newExtensions[complex] = extension;
      }

    }
    // EO foreach complex

    if (newExtensions.empty()) {
      return;
    }

    ExtSelExtMap newExtensionsByTarget;
    newExtensionsByTarget.insert(std::make_pair(target, newExtensions));
    // ToDo: do we really need to fetch again (see top off fn)
    auto existingExtensions = extensionsByExtender.find(target);
    if (existingExtensions != extensionsByExtender.end()) {
      if (hasExistingExtensions && !existingExtensions->second.empty()) {
        // Seems only relevant for sass 4.0 modules
        ExtSelExtMap additionalExtensions = _extendExistingExtensions(
          existingExtensions->second, newExtensionsByTarget);
        // Seems only relevant for sass 4.0 modules
        if (!additionalExtensions.empty()) {
          //std::cerr << "--- Check if we are doing it right?\n";
          mapAddAll2(newExtensionsByTarget, additionalExtensions);
        }
      }
    }

    if (hasRule) {
      _extendExistingSelectors(selectors54[target], newExtensionsByTarget);
    }

  }
  // EO addExtension (checked)


  /////////////////////////////////////////////////////////////////////////
  // Extend [extensions] using [newExtensions]. Note that this does duplicate
  // some work done by [_extendExistingStyleRules],  but it's necessary to
  // expand each extension's extender separately without reference to the full
  // selector list, so that relevant results don't get trimmed too early.
  //
  // Returns extensions that should be added to [newExtensions] before
  // extending selectors in order to properly handle extension loops such as:
  //
  //     .c {x: y; @extend .a}
  //     .x.y.a {@extend .b}
  //     .z.b {@extend .c}
  //
  // Returns `null` (Note: empty map) if there are no extensions to add.
  /////////////////////////////////////////////////////////////////////////
  // Note: maybe refactor to return `bool` (and pass reference)
  // Note: dart-sass throws an error in here
  /////////////////////////////////////////////////////////////////////////
  ExtSelExtMap ExtensionStore::_extendExistingExtensions(
    const sass::vector<ExtensionObj>& oldExtensions,
    const ExtSelExtMap& newExtensions)
  {

    ExtSelExtMap additionalExtensions;

    // During the loop `oldExtensions` vector might be changed.
    // Callers normally pass this from `extensionsByExtender` and
    // that points back to the `sources` vector from `extensions`.
    for (size_t i = 0, iL = oldExtensions.size(); i < iL; i += 1) {

      Extension* extension = oldExtensions[i];
      Extender& extender = extension->extender;
      SimpleSelector* target = extension->target;
      CssMediaRule* mediaContext = extension->mediaContext;

      // Get all registered extensions for this (SimpleSelector) target
      ExtSelExtMapEntry& sources = extensionsBySimpleSelector[target];

      // Do the actual extending of the selector on extender
      sass::vector<ComplexSelectorObj> selectors(extendComplex(
        extender.selector, newExtensions, mediaContext));
      // Dart-sass upgrades error?

      if (selectors.empty()) {
        // Should not happen?
        continue;
      }

      bool first = false, containsExtension =
        ObjEqualityFn(selectors.front(), extender.selector);

      for (ComplexSelectorObj& complex : selectors) {

        // If the output contains the original complex
        // selector, there's no need to recreate it.
        if (containsExtension && first) {
          first = false;
          continue;
        }

        Extension* withExtender =
          extension->withExtender(complex);
        auto it = sources.find(complex);
        if (it != sources.end()) {
          sources[complex] = mergeExtension(
            it->second, withExtender);
        }
        else {
          sources[complex] = withExtender;
          // Seems only relevant for sass 4.0 modules
          for (auto& component : complex->elements()) {
            if (auto compound = component->selector()) {
              for (auto& simple : compound->elements()) {
                extensionsByExtender[simple].emplace_back(withExtender);
              }
            }
          }
          if (newExtensions.find(target) != newExtensions.end()) {
            ExtSelExtMapEntry& entry = additionalExtensions[target];
            entry.emplace(std::make_pair(complex, withExtender));
          }
        }
      }

      // If [selectors] doesn't contain [extension.extender],
      // for example if it was replaced due to :not() expansion,
      // we must get rid of the old version.
      // Seems only relevant for sass 4.0 modules
      if (!containsExtension) {
        // sources.erase(extension.extender);
      }

    }

    return additionalExtensions;

  }
  // EO _extendExistingExtensions (checked/complex)

  /////////////////////////////////////////////////////////////////////////
  // Extend [extensions] using [newExtensions].
  /////////////////////////////////////////////////////////////////////////
  // Note: dart-sass throws an error in here
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::_extendExistingSelectors(
    const ExtListSelSet& selectors,
    const ExtSelExtMap& newExtensions)
  {
    // Is a modifyableCssStyleRUle in dart sass
    for (const SelectorListObj& selector : selectors) {
      CssMediaRule* mediaContext = nullptr;
      auto it = mediaContexts.find(selector);
      if (it != mediaContexts.end()) {
        mediaContext = it->second;
      }

      // std::cerr << "Extend existing selector " << selector->toString() << "\n";

      // If no extends happened (for example because unification
      // failed), we don't need to re-register the selector again.
      // LibSass Note: we pass the result instead of assigning
      // Instead it passes back if the extend succeeded therefore
      // we do not need the `identical` check that Dart-Sass has
      // std::cerr << "extend selector [" << selector->inspect() << "]\n";
      if (extendList(selector, newExtensions, mediaContext, selector->elements())) {
        //std::cerr << "Register existing selector " << selector->toString() << "\n";
        _registerSelector(selector, selector);
      }

    }
  }
  // EO _extendExistingSelectors (checked)

  /////////////////////////////////////////////////////////////////////////
  /// Extends [this] with all the extensions in [extensions].
  /// These extensions will extend all selectors already in [this],
  /// but they will *not* extend other extensions from [extenders].
  /////////////////////////////////////////////////////////////////////////

  void ExtensionStore::addExtensions(
    sass::vector<ExtensionStoreObj>& extensionStores) {

    // Extensions already in [this] whose extenders are extended by
    // [extensions], and thus which need to be updated.
    sass::vector<ExtensionObj> extensionsToExtend;

    // Selectors that contain simple selectors that are extended by
    // [extensions], and thus which need to be extended themselves.
    ExtListSelSet selectorsToExtend;

    // An extension map with the same structure as [_extensions] that only
    // includes extensions from [extensionStores].
    // Map<SimpleSelector, Map<ComplexSelector, Extension>> ? newExtensions;
    ExtSelExtMap newExtensions;

    bool hasSelectors = false;
    bool hasExtensions = false;

    for (ExtensionStore* extensionStore : extensionStores) {
      if (extensionStore->isEmpty()) continue;
      mapAddAll(sourceSpecificity, extensionStore->sourceSpecificity);

      auto& extensions = extensionStore->extensionsBySimpleSelector;
      for (auto extension : extensions) {
        auto& target = extension.first;
        auto& newSources = extension.second;

        // Private selectors can't be extended across module boundaries.
        if (auto* placeholder = target->isaPlaceholderSelector()) {
          if (placeholder->isPrivate93()) return; // continue?
        }

        // Find existing extensions to extend.
        auto extensionsForTargetIt = extensionsByExtender.find(target);
        if ((hasExtensions = (extensionsForTargetIt != extensionsByExtender.end()))) {
          auto& extensionsForTarget = extensionsForTargetIt->second;
          extensionsToExtend.insert(extensionsToExtend.end(),
            extensionsForTarget.begin(), extensionsForTarget.end());
        }

        // Find existing selectors to extend.
        auto selectorsForTargetIt = selectors54.find(target);
        if ((hasSelectors = (selectorsForTargetIt != selectors54.end()))) {
          ExtListSelSet& selectorsForTarget = selectorsForTargetIt->second;
          selectorsToExtend.insert(selectorsForTarget.begin(), selectorsForTarget.end());
        }

        // Add [newSources] to [_extensions].
        auto existingSourcesIt = extensionsBySimpleSelector.find(target);
        if (existingSourcesIt == extensionsBySimpleSelector.end()) {
          //std::cerr << "==== Set newSources for target " << target->inspect() << " ON " << this << "\n";
          extensionsBySimpleSelector[target] = newSources;
          if (hasExtensions || hasSelectors) {
            //std::cerr << "= Set newSources for newExt\n";
            newExtensions[target] = newSources;
          }
        }
        else {
          auto& existingSources = existingSourcesIt->second;
          for (auto& source : newSources) {
            auto& extender = source.first;
            auto& extension = source.second;
            // If [extender] already extends [target] in [_extensions], we don't
            // need to re-run the extension.
            if (existingSources.count(extender) == 0) {
              existingSources[extender] = extension;
            }

            if (!hasExtensions || !hasSelectors) {
              newExtensions[target][extender] = extension;
            }
          }
        }
      }
    }

    // We can ignore the return value here because it's only useful for extend
    // loops, which can't exist across module boundaries.
    _extendExistingExtensions(extensionsToExtend, newExtensions);
    _extendExistingSelectors(selectorsToExtend, newExtensions);

  }
  // EO addExtensions (checked/new)

  /////////////////////////////////////////////////////////////////////////
  // Extends [list] using [extensions].
  /////////////////////////////////////////////////////////////////////////
  bool ExtensionStore::extendList(
    const SelectorListObj& list,
    const ExtSelExtMap& extensions,
    const CssMediaRuleObj& mediaQueryContext,
    sass::vector<ComplexSelectorObj>& result)
  {
    // std::cerr << "extend list\n";
    // This could be written more simply using [List.map], but we want to
    // avoid any allocations in the common case where no extends apply.
    sass::vector<ComplexSelectorObj> extended;
    for (size_t i = 0; i < list->size(); i++) {
      const ComplexSelectorObj& complex = list->get(i);
      // std::cerr << "extend [" << ExtSelToStr(extensions) << "]\n";
      sass::vector<ComplexSelectorObj> result =
        extendComplex(complex, extensions, mediaQueryContext);
      // std::cerr << "extended [" << VecToString2(result) << "]\n";
      if (result.empty()) {
        if (!extended.empty()) {
          extended.emplace_back(complex);
        }
      }
      else {
        if (extended.empty()) {
          for (size_t n = 0; n < i; n += 1) {
            extended.emplace_back(list->get(n));
          }
        }
        for (auto& sel : result) {
          extended.emplace_back(std::move(sel));
        }
      }
    }

    if (extended.empty()) {
      return false;
    }

    // std::cerr << "RESULT ";
    // for (auto& qwe : extended) {
    //   std::cerr << qwe->inspect() << ", ";
    // }
    // std::cerr << "\n";

    result = std::move(extended);
    trim(result, originals);

    
    return true;
  }
  // EO extendList (review)

  /////////////////////////////////////////////////////////////////////////
  // Extends [complex] using [extensions], and
  // returns the contents of a [SelectorList].
  /////////////////////////////////////////////////////////////////////////
  sass::vector<ComplexSelectorObj> ExtensionStore::extendComplex(
    // Taking in a reference here makes MSVC debug stuck!?
    const ComplexSelectorObj& complex,
    const ExtSelExtMap& extensions,
    const CssMediaRuleObj& mediaQueryContext)
  {

    if (complex->leadingCombinators().size() > 1) return {};

      // The complex selectors that each compound selector in [complex.components]
    // can expand to.
    //
    // For example, given
    //
    //     .a .b {...}
    //     .x .y {@extend .b}
    //
    // this will contain
    //
    //     [
    //       [.a],
    //       [.b, .x .y]
    //     ]
    //
    // This could be written more simply using [List.map], but we want to avoid
    // any allocations in the common case where no extends apply.

    //if (complex->inspect() == "%x %y") {
      // std::cerr << "nope\n";
    //}

    //std::cerr << "extend complex [" << complex->inspect() << "]\n";

    //std::cerr << "_extendComplex\n";
    bool addedToExtendedNotExpanded = false;
    sass::vector<ComplexSelectorObj> result;
    sass::vector<sass::vector<ComplexSelectorObj>> extendedNotExpanded;
    bool isOriginal = originals.find(complex) != originals.end();
    for (size_t i = 0; i < complex->size(); i += 1) {
      const CplxSelComponentObj& component = complex->get(i);
      if (CompoundSelector* compound = component->selector()) {
        // std::cerr << "extend compound [" << compound->inspect() << "]\n";
        //std::cerr << "extend compound [" << component->toString() << "]\n";
        sass::vector<ComplexSelectorObj> extended = extendCompound(
          component, extensions, mediaQueryContext,
          complex->leadingCombinators(), isOriginal);

        //std::cerr << "- extended result " << VecToString2(extended) << "\n";
        //std::cerr << "- extendedNotExpanded " << VecVecToString2(extendedNotExpanded) << "\n";


        for (auto res : extended) {
          //std::cerr << "extended compound [" << res->inspect() << "]\n";
        }
        if (extended.empty()) {
          if (!extendedNotExpanded.empty()) {
            auto s = SASS_MEMORY_NEW(ComplexSelector, complex->pstate(),
              {}, { component }, complex->hasLineBreak());
            //std::cerr << "ADD 1 [" << s->inspect() << "]\n";
            extendedNotExpanded.push_back({

              s

              // compound->wrapInComplex(
              //   complex->leadingCombinators(),
              //   component->combinators())
            });
            //std::cerr << "- ADDED 1 " << VecVecToString2(extendedNotExpanded) << "\n";
          }
        }
        else {

          // Note: dart-sass checks for null!?
          if (!extendedNotExpanded.empty()) {
            //std::cerr << "ADD 2\n";
            addedToExtendedNotExpanded = true;
            extendedNotExpanded.push_back(extended);
            // for (size_t n = 0; n < i; n++) {
            //     extendedNotExpanded.push_back({
            //       // complex->at(n)->wrapInComplex2()
            //       complex->at(n)->wrapInComplex(
            //         complex->leadingCombinators())
            //     });
            // }
          }
          else if (i != 0) {
            sass::vector<CplxSelComponentObj> components;
            for (size_t n = 0; n < i; n++) {
              components.push_back(complex->get(n));
            }
            auto s = SASS_MEMORY_NEW(ComplexSelector, complex->pstate(),
              complex->leadingCombinators(), components,
              complex->hasLineBreak());
            //std::cerr << "ADD 3 [" << s->inspect() << "]\n";
            addedToExtendedNotExpanded = true;
            extendedNotExpanded = { {
              s
            }, extended };
          }
          else if (complex->leadingCombinators().empty()) {
            //std::cerr << "ADD 4 " << VecToString2(extended) << "\n";
            addedToExtendedNotExpanded = true;
            extendedNotExpanded = { extended }; //.emplace_back(extended);
          }
          else {
            //std::cerr << "ADD 5\n";
            addedToExtendedNotExpanded = true;
            sass::vector<ComplexSelectorObj> add;
            for (auto newComplex : extended) {
              if (newComplex->leadingCombinators().empty() ||
                ListEquality(complex->leadingCombinators(), newComplex->leadingCombinators(), PtrObjEqualityFn<SelectorCombinator>)) {
                if (complex->hasPreLineFeed() || newComplex->hasPreLineFeed()) {
                  std::cerr << "has pre line feed\n";
                }
                add.push_back(SASS_MEMORY_NEW(ComplexSelector, complex->pstate(),
                  complex->leadingCombinators(), newComplex->elements(),
                  complex->hasLineBreak() || newComplex->hasLineBreak() ||
                  complex->hasPreLineFeed() || newComplex->hasPreLineFeed()));
              }
              else {
                //add.push_back(newComplex);
              }
            }
            extendedNotExpanded.emplace_back(add);
          }
        }
      }
      else {
        std::cerr << "WHAT THE FUCK IS THIS?\n";
        // Note: dart-sass checks for null!?
        if (!extendedNotExpanded.empty()) {
          extendedNotExpanded.push_back({
            // component->wrapInComplex2()
            component->wrapInComplex(
              complex->leadingCombinators())
          });
        }
      }
    }

    //std::cerr << "=> extendedNotExpanded" << VecVecToString2(extendedNotExpanded) << "\n";

    // Note: dart-sass checks for null!?
    if (extendedNotExpanded.empty()) {
      return {};
    }


    for (auto a : extendedNotExpanded) {
      for (auto b : a) {
        //std::cerr << "not expanded [" << b->inspect() << "]\n";
      }
    }

    for (auto b : result) {
      // std::cerr << "res [" << b->inspect() << "]\n";
    }

    bool first = true;

    // ToDo: either change weave or paths to work with the same data?
    sass::vector<sass::vector<ComplexSelectorObj>>
      paths = permutate(extendedNotExpanded);

    for (auto a : paths) {
      for (auto b : a) {
        //std::cerr << "path [" << b->inspect() << "]\n";
      }
    }

    for (const sass::vector<ComplexSelectorObj>& path : paths) {
      // Unpack the inner complex selector to component list
      sass::vector<ComplexSelectorObj> _paths;
      for (const ComplexSelectorObj& sel : path) {
        _paths.insert(_paths.end(), sel);
      }

      for (auto a : _paths) {
        //std::cerr << "_path [" << a->inspect() << "]\n";
      }


      sass::vector<ComplexSelectorObj> weaved = weave(_paths);

      for (auto b : weaved) {
        //std::cerr << "woven [" << b->inspect() << "]\n";
      }

      for (ComplexSelectorObj& components : weaved) {
        auto others(components->elements());
        ComplexSelectorObj cplx = SASS_MEMORY_NEW(ComplexSelector,
          complex->pstate(), components->leadingCombinators(), std::move(others));
        // std::cerr << "woven [" << cplx->inspect() << "]\n";

        cplx->hasPreLineFeed(complex->hasPreLineFeed());
        for (auto& pp : path) {
          if (pp->hasPreLineFeed()) {
            cplx->hasPreLineFeed(true);
          }
        }

        // Make sure that copies of [complex] retain their status
        // as "original" selectors. This includes selectors that
        // are modified because a :not() was extended into.
        if (first) {
          auto it = originals.begin();
          while (it != originals.end()) {
            if (ObjEqualityFn(*it, complex)) break;
            it++;
          }
          if (it != originals.end()) {
            // std::cerr << "insert org [" << cplx->inspect() << "]\n";
            originals.insert(cplx);
          }
          first = false;
        }

        // Make sure we don't append any copies
        auto it = result.begin();
        while (it != result.end()) {
          if (ObjEqualityFn(*it, cplx)) break;
          it++;
        }
        if (it == result.end()) {
          // std::cerr << "insert res [" << cplx->inspect() << "]\n";
          result.push_back(cplx);
        }

        if (result.size() > 500) {
          traces->push_back(complex->pstate());
          throw Exception::EndlessExtendError(*traces);
        }

      }

    }

    // Here we must have two

    // std::cerr << "Complex: " << VecToString2(result) << "\n";

    return result;
  }
  // EO extendComplex (review)

  /////////////////////////////////////////////////////////////////////////
  // Extends [compound] using [extensions], and returns the
  // contents of a [SelectorList]. The [inOriginal] parameter
  // indicates whether this is in an original complex selector,
  // meaning that [compound] should not be trimmed out.
  /////////////////////////////////////////////////////////////////////////
  sass::vector<ComplexSelectorObj> ExtensionStore::extendCompound(
    const CplxSelComponentObj& component,
    const ExtSelExtMap& extensions,
    const CssMediaRuleObj& mediaQueryContext,
    const SelectorCombinatorVector& prefixes,
    bool inOriginal)
  {

    auto compound = component->selector();

    // If there's more than one target and they all need to
    // match, we track which targets are actually extended.
    std::unique_ptr<ExtSmplSelSet> targetsUsed;
    if (mode != ExtendMode::NORMAL && extensions.size() > 1) {
      targetsUsed.reset(new ExtSmplSelSet());
    }

    sass::vector<ComplexSelectorObj> result;
    // The complex selectors produced from each component of [compound].
    sass::vector<sass::vector<Extender>> options;
    for (size_t i = 0; i < compound->size(); i++) {
      const SimpleSelectorObj& simple = compound->get(i);
      auto extended = extendSimple(simple, extensions,
        mediaQueryContext, targetsUsed.get());

      if (extended.empty()) {
        if (!options.empty()) {
          auto foo = extenderForSimple(simple);
          //std::cerr << "Extended is nullptr "<< simple->toString() <<"\n";
          options.push_back({ foo });
          }
      }
      else {
        if (options.empty()) {
          if (i != 0) {
            sass::vector<SimpleSelectorObj> children;
            children.insert(children.begin(),
              compound->begin(), compound->begin() + i);
            // std::cerr << "Options Add 2\n";
            Extender sel = extenderForCompound(SASS_MEMORY_NEW(
              CompoundSelector, compound->pstate(), std::move(children)),
              {}, component->combinators());
            //std::cerr << "extender for compound [" << sel.selector->inspect() << "]\n";
            options.push_back({ sel });
          }
        }
        //std::cerr << "PUT all extended\n";
        // std::cerr << "Options Add 3\n";
        options.insert(options.end(),
          extended.begin(), extended.end());
      }
    }

    if (options.empty()) {
      return {};
    }

    // std::cerr << "OPTIONS " << options.size() << "\n";

    // If [_mode] isn't [ExtendMode.normal] and we didn't use all
    // the targets in [extensions], extension fails for [compound].
    if (targetsUsed != nullptr) {

      if (targetsUsed->size() != extensions.size()) {
        if (!targetsUsed->empty()) {
          return {};
        }
      }
    }

    // Optimize for the simple case of a single simple
    // selector that doesn't need any unification.
    if (options.size() == 1) {
      sass::vector<Extender>& exts = options[0];
      for (size_t n = 0; n < exts.size(); n += 1) {
        auto extender = exts[n];
        if (!extender.mediaContext.isNull()) {
          SourceSpan span(extender.pstate);
          callStackFrame outer(*traces, BackTrace(span, Strings::extendRule));
          callStackFrame inner(*traces, BackTrace(compound->pstate()));
          extender.assertCompatibleMediaContext(mediaQueryContext, *traces);
        }
        ComplexSelectorObj complex = extender.selector->
          withAdditionalCombinators(component->combinators());
        //std::cerr << "new complex " << complex->inspect() << "\n";
        if (complex->isUseless()) continue;
        result.emplace_back(complex);
      }

      //std::cerr << "Compound1: " << VecToString2(result) << "\n";

      return result;
    }

    // Find all paths through [options]. In this case, each path represents a
    // different unification of the base selector. For example, if we have:
    //
    //     .a.b {...}
    //     .w .x {@extend .a}
    //     .y .z {@extend .b}
    //
    // then [options] is `[[.a, .w .x], [.b, .y .z]]` and `paths(options)` is
    //
    //     [
    //       [.a, .b],
    //       [.a, .y .z],
    //       [.w .x, .b],
    //       [.w .x, .y .z]
    //     ]
    //
    // We then unify each path to get a list of complex selectors:
    //
    //     [
    //       [.a.b],
    //       [.y .a.z],
    //       [.w .x.b],
    //       [.w .y .x.z, .y .w .x.z]
    //     ]

    bool first = mode != ExtendMode::REPLACE;
    sass::vector<ComplexSelectorObj> unifiedPaths;

    for (auto qwe2 : options) {
      //std::cerr << "options [" << InspectSelector(qwe2) << "]\n";
    }
    sass::vector<sass::vector<Extender>> prePaths = permutate(options);
    for (auto qwe2 : prePaths) {
      //std::cerr << "prePaths [" << InspectSelector(qwe2) << "]\n";
    }

    for (size_t i = 0; i < prePaths.size(); i += 1) {
      sass::vector<ComplexSelectorObj> complexes;
      auto& path = prePaths[i];
      if (first) {
        // The first path is always the original selector. We can't just
        // return [compound] directly because pseudo selectors may be
        // modified, but we don't have to do any unification.
        first = false;
        CompoundSelectorObj mergedSelector =
          SASS_MEMORY_NEW(CompoundSelector,
            compound->pstate());
        for (size_t n = 0; n < path.size(); n += 1) {
          const ComplexSelectorObj& sel = path[n].selector;
          if (CompoundSelector* compound = sel->last()->selector()) {
            mergedSelector->concat(compound->elements());
          }
        }
        //std::cerr << "Add initial [" << mergedSelector->inspect() << "]\n";
        complexes.push_back(mergedSelector->wrapInComplex(
          {}, component->combinators()));
      }
      else {
        sass::vector<SimpleSelectorObj> originals;
        sass::vector<ComplexSelectorObj> toUnify;

        for (auto& extender : path) {
          if (extender.isOriginal) {
            const ComplexSelectorObj& sel = extender.selector;
            if (CompoundSelector* compound = sel->last()->selector()) {
              originals.insert(originals.end(), compound->last());
            }
          }
          else if (extender.selector->isUseless()) {
            return {};
          }
          else {
            //std::cerr << "++ add extender [" << extender.selector->inspect() << "]\n";
            toUnify.emplace_back(extender.selector);
          }
        }
        if (!originals.empty()) {
          CompoundSelectorObj merged =
            SASS_MEMORY_NEW(CompoundSelector,
              compound->pstate());
          merged->concat(originals);
          //std::cerr << "++ add first " << VecToString2(originals) << "\n";
          toUnify.insert(toUnify.begin(), { merged->wrapInComplex3() });
        }
        //std::cerr << "unifyComplexes: " << VecToString2(toUnify) << "\n";
        auto complexes2 = _unifyComplex(toUnify, compound->pstate());
        for (auto& cplx : complexes2) {
          ComplexSelectorObj r = cplx->withAdditionalCombinators(component->combinators());
          if (r->isUseless()) continue;
          complexes.push_back(r);
        }

        //std::cerr << "== resulted: " << VecToString2(complexes) << "\n";
        if (complexes.empty()) {
          continue;
        }

      }

      //std::cerr << "=> result: " << VecToString2(complexes) << "\n";


      bool lineBreak = false;
      // var specificity = _sourceSpecificityFor(compound);
      for (auto& state : path) {
        if (!state.mediaContext.isNull()) {
          SourceSpan span(state.pstate);
          callStackFrame outer(*traces, BackTrace(span, Strings::extendRule));
          callStackFrame inner(*traces, BackTrace(compound->pstate()));
          state.assertCompatibleMediaContext(mediaQueryContext, *traces);
        }
        lineBreak = lineBreak || state.selector->hasPreLineFeed();
        // specificity = math.max(specificity, state.specificity);
      }

      for (ComplexSelectorObj& sel2 : complexes) {
        auto sel = SASS_MEMORY_NEW(ComplexSelector,
          compound->pstate(),
            sel2->leadingCombinators(),
          std::move(sel2->elements()));
        sel->hasPreLineFeed(lineBreak);
        unifiedPaths.emplace_back(sel);
      }

    }

    //std::cerr << "unifiedPaths: " << VecToString2(unifiedPaths) << "\n";

    return unifiedPaths;
  }
  // EO extendCompound (review)

  /////////////////////////////////////////////////////////////////////////
  // Extends [simple] without extending the
  // contents of any selector pseudos it contains.
  /////////////////////////////////////////////////////////////////////////
  sass::vector<Extender> ExtensionStore::extendWithoutPseudo(
    const SimpleSelectorObj& simple,
    const ExtSelExtMap& extensions,
    ExtSmplSelSet* targetsUsed) const
  {
    auto extensionIt = extensions.find(simple);
    if (extensionIt == extensions.end()) return {};
    auto& extensionsForSimple = extensionIt->second;

      if (targetsUsed != nullptr) {
      targetsUsed->insert(simple);
    }

    sass::vector<Extender> result;
    if (mode != ExtendMode::REPLACE) {
      result.emplace_back(extenderForSimple(simple));
    }

    for (auto& ext : extensionsForSimple) {
      result.push_back(ext.second->extender);
    }

    return std::move(result);
  }
  // EO extendWithoutPseudo

  /////////////////////////////////////////////////////////////////////////
  // Extends [simple] and also extending the
  // contents of any selector pseudos it contains.
  /////////////////////////////////////////////////////////////////////////
  sass::vector<sass::vector<Extender>> ExtensionStore::extendSimple(
    const SimpleSelectorObj& simple,
    const ExtSelExtMap& extensions,
    const CssMediaRuleObj& mediaQueryContext,
    ExtSmplSelSet* targetsUsed)
  {
    if (PseudoSelector* pseudo = simple->isaPseudoSelector()) {
      if (pseudo->selector()) {
        sass::vector<sass::vector<Extender>> merged;
        sass::vector<PseudoSelectorObj> extended =
          extendPseudo(pseudo, extensions, mediaQueryContext);
        for (PseudoSelectorObj& extend : extended) {
          SimpleSelectorObj simple = extend.ptr();
          sass::vector<Extender> result =
            extendWithoutPseudo(simple, extensions, targetsUsed);
          if (result.empty()) result = { extenderForSimple(extend.ptr()) };
          merged.emplace_back(result);
        }
        if (!extended.empty()) {
          return merged;
        }
      }
    }
    sass::vector<Extender> result =
      extendWithoutPseudo(simple, extensions, targetsUsed);
    // std::cerr << "Simple " << simple->inspect() << "\n";
    // std::cerr << "Without " << VecToString(result) << "\n";
    // std::cerr << "Exts " << ExtSelToStr(extensions) << "\n";
    if (result.empty()) return {};
    return { result };
  }
  // extendSimple

  /////////////////////////////////////////////////////////////////////////
  // Returns a one-off [Extension] whose extender is composed
  // solely of a compound selector containing [simples].
  /////////////////////////////////////////////////////////////////////////
  Extender ExtensionStore::extenderForCompound(
    const CompoundSelectorObj& compound,
    const SelectorCombinatorVector& prefixes,
    const SelectorCombinatorVector& postfixes) const
  {
    ComplexSelector* complex = compound->wrapInComplex(prefixes, {});
    return Extender{
      complex->pstate(),
      complex, maxSourceSpecificity(compound), true };
  }
  // EO extenderForCompound

  /////////////////////////////////////////////////////////////////////////
  // Returns a one-off [Extension] whose
  // extender is composed solely of [simple].
  /////////////////////////////////////////////////////////////////////////
  Extender ExtensionStore::extenderForSimple(
    const SimpleSelectorObj& simple) const
  {
    ComplexSelector* complex = simple->wrapInComplex({});
    return Extender{
      complex->pstate(),
      complex, maxSourceSpecificity(simple), true };
  }
  // ExtensionStore::extenderForSimple

  /////////////////////////////////////////////////////////////////////////
  // Inner loop helper for [extendPseudo] function
  /////////////////////////////////////////////////////////////////////////
  sass::vector<ComplexSelectorObj> ExtensionStore::extendPseudoComplex(
    const ComplexSelectorObj& complex,
    const PseudoSelectorObj& pseudo,
    const CssMediaRuleObj& mediaQueryContext)
  {

    if (complex->size() != 1) { return { complex }; }
    auto compound = complex->get(0)->selector();
    if (compound == nullptr) { return { complex }; }
    if (compound->size() != 1) { return { complex }; }
    auto innerPseudo = compound->get(0)->isaPseudoSelector();
    if (innerPseudo == nullptr) { return { complex }; }
    if (!innerPseudo->selector()) { return { complex }; }

    sass::string name(pseudo->normalized());

    if (name == "not") {
      // In theory, if there's a `:not` nested within another `:not`, the
      // inner `:not`'s contents should be unified with the return value.
      // For example, if `:not(.foo)` extends `.bar`, `:not(.bar)` should
      // become `.foo:not(.bar)`. However, this is a narrow edge case and
      // supporting it properly would make this code and the code calling it
      // a lot more complicated, so it's not supported for now.
      if (innerPseudo->normalized() != "matches" &&
        innerPseudo->normalized() != "where" &&
        innerPseudo->normalized() != "is") return {};
      return innerPseudo->selector()->elements();
    }
    else if (isSubselectorPseudo(name) || name == "current") {
      // As above, we could theoretically support :not within :matches, but
      // doing so would require this method and its callers to handle much
      // more complex cases that likely aren't worth the pain.
      if (innerPseudo->name() != pseudo->name()) return {};
      if (innerPseudo->argument() != pseudo->argument()) return {};
      return innerPseudo->selector()->elements();
    }
    else if (name == "has" || name == "host" || name == "host-context" || name == "slotted") {
      // We can't expand nested selectors here, because each layer adds an
      // additional layer of semantics. For example, `:has(:has(img))`
      // doesn't match `<div><img></div>` but `:has(img)` does.
      return { complex };
    }

    return {};

  }
  // EO extendPseudoComplex

  /////////////////////////////////////////////////////////////////////////
  // Extends [pseudo] using [extensions], and returns
  // a list of resulting pseudo selectors.
  /////////////////////////////////////////////////////////////////////////
  sass::vector<PseudoSelectorObj> ExtensionStore::extendPseudo(
    const PseudoSelectorObj& pseudo,
    const ExtSelExtMap& extensions,
    const CssMediaRuleObj& mediaQueryContext)
  {
    sass::vector<ComplexSelectorObj> extended;
    // Call extend and abort if nothing was extended
    if (!pseudo || !pseudo->selector() ||
      !extendList(pseudo->selector(), extensions,
        mediaQueryContext, extended)) {
      // Abort pseudo extend
      return {};
    }

    // For `:not()`, we usually want to get rid of any complex selectors because
    // that will cause the selector to fail to parse on all browsers at time of
    // writing. We can keep them if either the original selector had a complex
    // selector, or the result of extending has only complex selectors, because
    // either way we aren't breaking anything that isn't already broken.
    sass::vector<ComplexSelectorObj> complexes = extended;

    if (pseudo->normalized() == "not") {
      if (!hasAny(pseudo->selector()->elements(), hasMoreThanOne)) {
        if (hasAny(extended, hasExactlyOne)) {
          complexes.clear();
          for (auto& complex : extended) {
            if (complex->size() <= 1) {
              complexes.emplace_back(complex);
            }
          }
        }
      }
    }

    sass::vector<ComplexSelectorObj> expanded = expand(
      std::move(complexes), extendPseudoComplex,
      pseudo, mediaQueryContext);

    // Older browsers support `:not`, but only with a single complex selector.
    // In order to support those browsers, we break up the contents of a `:not`
    // unless it originally contained a selector list.
    if (pseudo->normalized() == "not") {
      if (pseudo->selector()->size() == 1) {
        sass::vector<PseudoSelectorObj> pseudos;
        for (size_t i = 0; i < expanded.size(); i += 1) {
          pseudos.emplace_back(pseudo->withSelector(
            expanded[i]->wrapInList()
          ));
        }
        return pseudos;
      }
    }

    SelectorListObj list = SASS_MEMORY_NEW(SelectorList,
      pseudo->pstate(), std::move(expanded));
    return { pseudo->withSelector(list) };

  }
  // EO extendPseudo

  /////////////////////////////////////////////////////////////////////////
  // Removes elements from [selectors] if they're subselectors of other
  // elements. The [isOriginal] callback indicates which selectors are
  // original to the document, and thus should never be trimmed.
  /////////////////////////////////////////////////////////////////////////
  // Note: for adaption I pass in the set directly, there is some
  // code path in selector-trim that might need this special callback
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::trim(
    sass::vector<ComplexSelectorObj>& selectors,
    const ExtCplxSelSet& existing) const
  {

    // Avoid truly horrific quadratic behavior.
    // TODO(nweiz): I think there may be a way to get perfect trimming
    // without going quadratic by building some sort of trie-like
    // data structure that can be used to look up superselectors.
    // TODO(mgreter): Check how this performs in C++ (up the limit)
    if (selectors.size() > 100) return;

    // This is n² on the sequences, but only comparing between separate sequences
    // should limit the quadratic behavior. We iterate from last to first and reverse
    // the result so that, if two selectors are identical, we keep the first one.
    sass::vector<ComplexSelectorObj> result; size_t numOriginals = 0;

    size_t i = selectors.size();
  outer: // Use label to continue loop
    while (--i != std::string::npos) {

      const ComplexSelectorObj& complex1 = selectors[i];
      // Check if selector in known in existing "originals"
      // For custom behavior dart-sass had `isOriginal(complex1)`
      if (existing.find(complex1) != existing.end()) {
        // Make sure we don't include duplicate originals, which could
        // happen if a style rule extends a component of its own selector.
        for (size_t j = 0; j < numOriginals; j++) {
          if (ObjEqualityFn(result[j], complex1)) {
            rotateSlice(result, 0, j + 1);
            goto outer;
          }
        }
        result.insert(result.begin(), complex1);
        numOriginals++;
        continue;
      }

      // The maximum specificity of the sources that caused [complex1]
      // to be generated. In order for [complex1] to be removed, there
      // must be another selector that's a superselector of it *and*
      // that has specificity greater or equal to this.
      size_t maxSpecificity = 0;
      for (const CplxSelComponentObj& component : complex1->elements()) {
        if (const CompoundSelectorObj compound = component->selector()) {
          maxSpecificity = std::max(maxSpecificity, maxSourceSpecificity(compound));
        }
      }


      // Look in [result] rather than [selectors] for selectors after [i]. This
      // ensures we aren't comparing against a selector that's already been trimmed,
      // and thus that if there are two identical selectors only one is trimmed.
      if (hasAny(result, dontTrimComplex, complex1, maxSpecificity)) {
        continue;
      }

      // Check if any element (up to [i]) from [selector] returns true
      // when passed to [dontTrimComplex]. The arguments [complex1] and
      // [maxSepcificity] will be passed to the invoked function.
      if (hasSubAny(selectors, i, dontTrimComplex, complex1, maxSpecificity)) {
        continue;
      }

      // ToDo: Maybe use deque for front insert?
      result.insert(result.begin(), complex1);

    }

    selectors = std::move(result);

  }
  // EO trim

  /////////////////////////////////////////////////////////////////////////
  // Returns the maximum specificity of the given [simple] source selector.
  /////////////////////////////////////////////////////////////////////////
  size_t ExtensionStore::maxSourceSpecificity(const SimpleSelectorObj& simple) const
  {
    auto it = sourceSpecificity.find(simple);
    if (it == sourceSpecificity.end()) return 0;
    return it->second;
  }
  // EO maxSourceSpecificity(SimpleSelectorObj)

  /////////////////////////////////////////////////////////////////////////
  // Returns the maximum specificity for sources that went into producing [compound].
  /////////////////////////////////////////////////////////////////////////
  size_t ExtensionStore::maxSourceSpecificity(const CompoundSelectorObj& compound) const
  {
    size_t specificity = 0;
    for (auto simple : compound->elements()) {
      size_t src = maxSourceSpecificity(simple);
      specificity = std::max(specificity, src);
    }
    return specificity;
  }
  // EO maxSourceSpecificity(CompoundSelectorObj)

  /////////////////////////////////////////////////////////////////////////
  // Helper function used as callbacks on lists
  /////////////////////////////////////////////////////////////////////////
  bool ExtensionStore::dontTrimComplex(
    const ComplexSelector* complex2,
    const ComplexSelector* complex1,
    const size_t maxSpecificity)
  {
    // std::cerr << "IS " << complex2->isSuperselectorOf(complex1) << "\n";
    return complex2->minSpecificity() >= maxSpecificity &&
      complex2->isSuperselectorOf(complex1);
  }
  // EO dontTrimComplex

  /////////////////////////////////////////////////////////////////////////
  // Rotates the element in list from [start] (inclusive) to [end] (exclusive)
  // one index higher, looping the final element back to [start].
  /////////////////////////////////////////////////////////////////////////
  void ExtensionStore::rotateSlice(
    sass::vector<ComplexSelectorObj>& list,
    size_t start, size_t end)
  {
    auto element = list[end - 1];
    for (size_t i = start; i < end; i++) {
      auto next = list[i];
      list[i] = element;
      element = next;
    }
  }
  // EO rotateSlice

  /////////////////////////////////////////////////////////////////////////
  // Helper function used as callbacks on lists
  /////////////////////////////////////////////////////////////////////////
  bool ExtensionStore::hasExactlyOne(const ComplexSelectorObj& vec)
  {
    return vec->size() == 1;
  }
  // EO hasExactlyOne

  /////////////////////////////////////////////////////////////////////////
  // Helper function used as callbacks on lists
  /////////////////////////////////////////////////////////////////////////
  bool ExtensionStore::hasMoreThanOne(const ComplexSelectorObj& vec)
  {
    return vec->size() > 1;
  }
  // hasMoreThanOne

}

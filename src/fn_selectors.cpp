/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "fn_selectors.hpp"

#include "extender.hpp"
#include "source.hpp"
#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"
#include "ast_selectors.hpp"
#include "parser_selector.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Adds a [ParentSelector] to the beginning of [compound],
    // or returns `null` if that wouldn't produce a valid selector.
    CompoundSelector* prependParent(CompoundSelector* compound) {
      SimpleSelector* first = compound->first();
      if (first->isUniversal()) return nullptr;
      if (TypeSelector * type = first->isaTypeSelector()) {
        if (type->hasNs()) return nullptr;
        CompoundSelector* copy = SASS_MEMORY_COPY(compound);
        copy->withExplicitParent(true);
        return copy;
      }
      else {
        CompoundSelector* copy = SASS_MEMORY_COPY(compound);
        copy->withExplicitParent(true);
        return copy;
      }
    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Selectors {

      /*******************************************************************/

      BUILT_IN_FN(nest)
      {
        // Not enough parameters
        if (arguments[0]->lengthAsList() == 0) {
          throw Exception::RuntimeException(compiler,
            "$selectors: At least one selector must be passed.");
        }
        SelectorListObj result;
        // Iterate over the rest argument list
        for (Value* arg : arguments[0]->start()) {
          if (arg->isNull()) {
            callStackFrame csf(compiler, arg->pstate());
            throw Exception::RuntimeException(compiler, // "$selectors: "
              "null is not a valid selector: it must be a string,\n"
              "a list of strings, or a list of lists of strings.");
          }
          // Read and parse argument into selectors
          SelectorListObj slist(arg->assertSelector(
            compiler, Strings::empty, !result.isNull()));
          // First item is just taken as it is
          if (result.isNull()) { result = slist; continue; }
          // Otherwise resolve it with the previous selector
          result = slist->resolveParentSelectors(result, compiler);
        }
        // Convert and return value
        return result->toValue();
      }

      /*******************************************************************/

      BUILT_IN_FN(append)
      {
        // Not enough parameters
        if (arguments[0]->lengthAsList() == 0) {
          throw Exception::RuntimeException(compiler,
            "$selectors: At least one selector must be passed.");
        }

        // return SASS_MEMORY_NEW(Null, pstate);

        SelectorListObj reduced;
        for (Value* arg : arguments[0]->start()) {
          if (arg->isNull()) {
            throw Exception::RuntimeException( // "$selectors: "
              "null is not a valid selector: it must be a string,\n"
              "a list of strings, or a list of lists of strings.",
              compiler, arg->pstate());
          }
          SelectorListObj slist(arg->assertSelector(
            compiler, Strings::empty, false));
          // First item is just taken as it is
          if (reduced.isNull()) { reduced = slist; continue; }

          // Combine selector list with parent
          SelectorListObj cp = SASS_MEMORY_COPY(slist);
          for (ComplexSelector* complex : slist->elements()) {
            if (!complex->leadingCombinators().empty()) {
              throw Exception::RuntimeException(compiler,
                "Can't append " + slist->inspect() + " to " +
                reduced->inspect() + ".");
            }
            if (complex->empty()) continue; // TODO
            CplxSelComponent* component = complex->first();
            if (CompoundSelector* compound = component->selector()) {
              compound = prependParent(compound);
              if (compound == nullptr) {
                throw Exception::RuntimeException(compiler,
                  "Can't append " + slist->inspect() + " to " +
                  reduced->inspect() + ".");
              }
              complex->set(0, compound->wrapInComponent(component->combinators()));
            }
            else {
              throw Exception::RuntimeException(compiler,
                "Can't append " + slist->inspect() + " to " +
                reduced->inspect() + ".");
            }
          }

          // Otherwise resolve it with the previous selector
          reduced = cp->resolveParentSelectors(reduced, compiler, false);

        }

        return reduced->toValue();
      }

      /*******************************************************************/

      BUILT_IN_FN(extend)
      {
        SelectorListObj selector = arguments[0]->
          assertSelector(compiler, "selector")
          ->assertNotBogus("selector");
        SelectorListObj target = arguments[1]->
          assertSelector(compiler, "extendee")
          ->assertNotBogus("extendee");
        SelectorListObj source = arguments[2]->
          assertSelector(compiler, "extender")
          ->assertNotBogus("extender");
        SelectorListObj result = ExtensionStore::extend(selector, source, target, compiler);
        if (result.isNull()) return SASS_MEMORY_NEW(Null, pstate);
        return result->toValue();
      }

      BUILT_IN_FN(replace)
      {
        SelectorListObj selector = arguments[0]->
          assertSelector(compiler, "selector")
          ->assertNotBogus("selector");
        SelectorListObj target = arguments[1]->
          assertSelector(compiler, "original")
          ->assertNotBogus("original");
        SelectorListObj source = arguments[2]->
          assertSelector(compiler, "replacement")
          ->assertNotBogus("replacement");
        SelectorListObj result = ExtensionStore::replace(selector, source, target, compiler);
        if (result.isNull()) return SASS_MEMORY_NEW(Null, pstate);
        return result->toValue();
      }

      BUILT_IN_FN(unify)
      {
        SelectorListObj selector1 = arguments[0]->
          assertSelector(compiler, "selector1");
        SelectorListObj selector2 = arguments[1]->
          assertSelector(compiler, "selector2");
        //std::cerr << "selector1 " << selector1->inspect() << "\n";
        //std::cerr << "selector2 " << selector2->inspect() << "\n";
        SelectorListObj result = selector1->unifyWith(selector2);
        if (result.isNull()) return SASS_MEMORY_NEW(Null, pstate);
        return result->toValue();
      }

      BUILT_IN_FN(isSuper)
      {
        SelectorListObj sel_sup = arguments[0]
          ->assertSelector(compiler, "super")
          ->assertNotBogus("super");
          
        SelectorListObj sel_sub = arguments[1]
          ->assertSelector(compiler, "sub")
          ->assertNotBogus("super");
        bool result = sel_sup->isSuperselectorOf(sel_sub);
        return SASS_MEMORY_NEW(Boolean, pstate, result);
      }

      BUILT_IN_FN(simple)
      {
        CompoundSelectorObj selector = arguments[0]->
          assertCompoundSelector(compiler, "selector");
        ValueVector results;
        for (auto child : selector->elements()) {
          results.emplace_back(SASS_MEMORY_NEW(String,
            child->pstate(), child->inspect()));
        }
        // Return new value list
        return SASS_MEMORY_NEW(List,
          selector->pstate(),
          std::move(results),
          SASS_COMMA);
      }

      BUILT_IN_FN(parse)
      {
        SelectorListObj selector = arguments[0]->
          assertSelector(compiler, "selector");
        return selector->toValue();
      }

      /*******************************************************************/

      void registerFunctions(Compiler& ctx)
	    {
        BuiltInMod& module(ctx.createModule("selector"));
        module.addFunction(key_nest, ctx.registerBuiltInFunction(key_selector_nest, "$selectors...", nest));
		    module.addFunction(key_append, ctx.registerBuiltInFunction(key_selector_append, "$selectors...", append));
		    module.addFunction(key_extend, ctx.registerBuiltInFunction(key_selector_extend, "$selector, $extendee, $extender", extend));
		    module.addFunction(key_replace, ctx.registerBuiltInFunction(key_selector_replace, "$selector, $original, $replacement", replace));
		    module.addFunction(key_unify, ctx.registerBuiltInFunction(key_selector_unify, "$selector1, $selector2", unify));
		    module.addFunction(key_is_superselector, ctx.registerBuiltInFunction(key_is_superselector, "$super, $sub", isSuper));
		    module.addFunction(key_simple_selectors, ctx.registerBuiltInFunction(key_simple_selectors, "$selector", simple));
		    module.addFunction(key_parse, ctx.registerBuiltInFunction(key_selector_parse, "$selector", parse));
	    }

      /*******************************************************************/

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}

#include "fn_lists.hpp"

#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"

namespace Sass {

  namespace Functions {

    namespace Lists {

      /*******************************************************************/

      BUILT_IN_FN(length)
      {
        return SASS_MEMORY_NEW(Number,
          arguments[0]->pstate(),
          (double) arguments[0]->lengthAsList());
      }

      /*******************************************************************/

      BUILT_IN_FN(nth)
      {
        Value* list = arguments[0];
        Value* index = arguments[1];
        return list->getValueAt(index, compiler);
      }

      /*******************************************************************/

      BUILT_IN_FN(setNth)
      {
        Value* input = arguments[0];
        Value* index = arguments[1];

        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (selfAssign && input->refcount < AssignableRefCount) {
          if (List* lst = input->isaList()) {
            size_t idx = input->sassIndexToListIndex(index, compiler, "n");
            lst->at(idx) = arguments[2];
            return lst;
          }
        }
        #endif

        auto it = input->iterator();
        size_t idx = input->
          sassIndexToListIndex(
            index, compiler, "n");
        ValueVector values(it.begin(), it.end());
        values[idx] = arguments[2];
        return SASS_MEMORY_NEW(List,
          input->pstate(), std::move(values),
          input->separator(), input->hasBrackets());
      }

      /*******************************************************************/

      BUILT_IN_FN(join)
      {
        Value* list1 = arguments[0];
        Value* list2 = arguments[1];
        String* separatorParam = arguments[2]->assertString(compiler, "separator");
        Value* bracketedParam = arguments[3];

        SassSeparator separator = SASS_UNDEF;
        if (separatorParam->value() == "auto") {
          if (list1->separator() != SASS_UNDEF) {
            separator = list1->separator();
          }
          else if (list2->separator() != SASS_UNDEF) {
            separator = list2->separator();
          }
          else {
            separator = SASS_SPACE;
          }
        }
        else if (separatorParam->value() == "space") {
          separator = SASS_SPACE;
        }
        else if (separatorParam->value() == "comma") {
          separator = SASS_COMMA;
        }
        else {
          throw Exception::SassScriptException(
            "$separator: Must be \"space\", \"comma\", or \"auto\".",
            compiler, pstate);
        }

        bool bracketed = bracketedParam->isTruthy();
        if (String * str = bracketedParam->isaString()) {
          if (str->value() == "auto") {
            bracketed = list1->hasBrackets();
          }
        }

        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (selfAssign && list2->refcount < AssignableRefCount) {
          if (List* lst = list2->isaList()) {
            ValueVector& values(lst->elements());
            lst->separator(separator);
            lst->hasBrackets(bracketed);
            auto it2 = list2->iterator();
            values.insert(values.end(),
              it2.begin(), it2.end());
            return lst;
          }
        }
        #endif

        ValueVector values;
        auto it1 = list1->iterator();
        values.insert(values.end(),
          it1.begin(), it1.end());
        auto it2 = list2->iterator();
        values.insert(values.end(),
          it2.begin(), it2.end());
        return SASS_MEMORY_NEW(List, pstate,
          std::move(values), separator, bracketed);
      }

      /*******************************************************************/

      BUILT_IN_FN(append)
      {
        Value* list = arguments[0]->assertValue(compiler, "list");
        Value* value = arguments[1]->assertValue(compiler, "val");
        String* separatorParam = arguments[2]->assertString(compiler, "separator");
        SassSeparator separator = SASS_UNDEF;
        if (separatorParam->value() == "auto") {
          separator = list->separator() == SASS_UNDEF
            ? SASS_SPACE : list->separator();
        }
        else if (separatorParam->value() == "space") {
          separator = SASS_SPACE;
        }
        else if (separatorParam->value() == "comma") {
          separator = SASS_COMMA;
        }
        else {
          throw Exception::SassScriptException(
            "$separator: Must be \"space\", \"comma\", or \"auto\".",
            compiler, pstate);
        }

        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (selfAssign && list->refcount < AssignableRefCount) {
          if (List* lst = list->isaList()) {
            lst->separator(separator);
            lst->append(value);
            return lst;
          }
        }
        #endif

        ValueVector values;
        auto it = list->iterator();
        values.insert(values.end(),
          it.begin(), it.end());
        values.emplace_back(value);
        return SASS_MEMORY_NEW(List,
          list->pstate(), std::move(values),
          separator, list->hasBrackets());
      }

      /*******************************************************************/

      BUILT_IN_FN(zip)
      {
        size_t shortest = sass::string::npos;
        sass::vector<ValueVector> lists;
        for (Value* arg : arguments[0]->iterator()) {
          Values it = arg->iterator();
          ValueVector inner;
          inner.reserve(arg->lengthAsList());
          std::copy(it.begin(), it.end(),
            std::back_inserter(inner));
          shortest = std::min(shortest, inner.size());
          lists.emplace_back(inner);
        }
        if (lists.empty()) {
          return SASS_MEMORY_NEW(List,
            pstate, {}, SASS_COMMA);
        }
        ValueVector result;
        if (!lists.empty()) {
          for (size_t i = 0; i < shortest; i++) {
            ValueVector inner;
            inner.reserve(lists.size());
            for (ValueVector& arg : lists) {
              inner.push_back(arg[i]);
            }
            result.emplace_back(SASS_MEMORY_NEW(
              List, pstate, inner, SASS_SPACE));
          }
        }
        return SASS_MEMORY_NEW(
          List, pstate, result, SASS_COMMA);
      }

      /*******************************************************************/

      BUILT_IN_FN(index)
      {
        Value* value = arguments[1];
        size_t index = arguments[0]->indexOf(value);
        if (index == NPOS) {
          return SASS_MEMORY_NEW(Null,
            arguments[0]->pstate());
        }
        return SASS_MEMORY_NEW(Number,
          arguments[0]->pstate(),
          (double) index + 1);
      }

      /*******************************************************************/

      BUILT_IN_FN(separator)
      {
        return SASS_MEMORY_NEW(String, arguments[0]->pstate(),
          std::move(arguments[0]->separator() == SASS_COMMA ? "comma" : "space"));
      }

      /*******************************************************************/

      BUILT_IN_FN(isBracketed)
      {
        return SASS_MEMORY_NEW(Boolean, pstate,
          arguments[0]->hasBrackets());
      }

      /*******************************************************************/

      void registerFunctions(Compiler& ctx)
	    {
		    ctx.registerBuiltInFunction("length", "$list", length);
		    ctx.registerBuiltInFunction("nth", "$list, $n", nth);
		    ctx.registerBuiltInFunction("set-nth", "$list, $n, $value", setNth);
		    ctx.registerBuiltInFunction("join", "$list1, $list2, $separator: auto, $bracketed: auto", join);
		    ctx.registerBuiltInFunction("append", "$list, $val, $separator: auto", append);
		    ctx.registerBuiltInFunction("zip", "$lists...", zip);
		    ctx.registerBuiltInFunction("index", "$list, $value", index);
		    ctx.registerBuiltInFunction("list-separator", "$list", separator);
		    ctx.registerBuiltInFunction("is-bracketed", "$list", isBracketed);
	    }

      /*******************************************************************/

    }
  }

}

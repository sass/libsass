/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "fn_maps.hpp"

#include "eval.hpp"
#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Maps {

      /*******************************************************************/

      // Merges [map1] and [map2], with values in [map2] taking precedence.
      // If both [map1] and [map2] have a map value associated with
      // the same key, this recursively merges those maps as well.
      Map* deepMergeImpl(Map* map1, Map* map2)
      {

        if (map2->empty()) return map1;
        if (map1->empty()) return map2;

        // Avoid making a mutable copy of `map2` if it would totally overwrite `map1`
        // anyway.
        // var mutable = false;
        // var result = map2.contents;
        // void _ensureMutable() {
        //   if (mutable) return;
        //   mutable = true;
        //   result = Map.of(result);
        // }

        MapObj result = SASS_MEMORY_COPY(map1);

        // Because values in `map2` take precedence over `map1`, we just check if any
        // entries in `map1` don't have corresponding keys in `map2`, or if they're
        // maps that need to be merged in their own right.
        // Note: this changes insertion order, bad!?
        for (auto kv : map2->elements()) {
          Value* key = kv.first;
          Value* value = kv.second;
          auto it = result->find(key);
          if (it != result->end()) {
            Map* valueMap = value->isaMap();
            Map* resultMap = it->second->isaMap();
            if (resultMap != nullptr && valueMap != nullptr) {
              Map* merged = deepMergeImpl(resultMap, valueMap);
              // if (identical(merged, resultMap)) return;
              it.value() = merged;
            }
            else {
              // Might got an empty list that could be a map
              List* resultList = kv.second->isaList();
              if (resultList && resultList->empty()) {
                // it.value() = kv.second;
              }
              else {
                it.value() = kv.second;
              }
            }
          }
          else {
            result->insert(kv);
          }
        }
        //std::cerr << "Result " << result->inspect() << "\n";
        return result.detach();
      }


      // Merges [map1] and [map2], with values in [map2] taking precedence.
      // If both [map1] and [map2] have a map value associated with
      // the same key, this recursively merges those maps as well.
      Map* deepMergeImplOptimized(Map* map1, Map* map2)
      {

        if (map2->empty()) return map1;

        // Avoid making a mutable copy of `map2` if it would totally overwrite `map1`
        // anyway.
        // var mutable = false;
        // var result = map2.contents;
        // void _ensureMutable() {
        //   if (mutable) return;
        //   mutable = true;
        //   result = Map.of(result);
        // }

        MapObj result = SASS_MEMORY_COPY(map2);

        // Because values in `map2` take precedence over `map1`, we just check if any
        // entries in `map1` don't have corresponding keys in `map2`, or if they're
        // maps that need to be merged in their own right.
        // Note: this changes insertion order, bad!?
        for (auto kv : map1->elements()) {
          Value* key = kv.first;
          Value* value = kv.second;
          auto it = result->find(key);
          if (it != result->end()) {
            Map* valueMap = value->isaMap();
            Map* resultMap = it->second->isaMap();
            if (resultMap != nullptr && valueMap != nullptr) {
              Map* merged = deepMergeImplOptimized(valueMap, resultMap);
              // if (identical(merged, resultMap)) return;
              it.value() = merged;
            }
            else {
              // Might got an empty list that could be a map
              List* resultList = it->second->isaList();
              if (resultList && resultList->empty()) {
                it.value() = kv.second;
              }
            }
          }
          else {
            result->insert(kv);
          }
        }

        return result.detach();
      }

      /*******************************************************************/

      BUILT_IN_FN(get)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        Value* key = arguments[1]->assertValue(compiler, Strings::key);


        auto it = map->find(key);
        if (it != map->end()) {
          Value* rv = it->second;

          auto cur = arguments[2]->start();
          auto end = arguments[2]->stop();
          if (cur == end) {
            return rv;
          }
          while (cur != end) {
            if (auto deep = rv->isaMap()) {
              auto dada = deep->find(*cur);
              if (dada != deep->end()) {
                rv = dada.value();
              }
              else {
                return SASS_MEMORY_NEW(Null, pstate);
              }
            }
            else {
              return SASS_MEMORY_NEW(Null, pstate);
            }
            ++cur;
          }

          return rv;
        }
        return SASS_MEMORY_NEW(Null, pstate);
      }


      /*******************************************************************/

      BUILT_IN_FN(fnMapSetThreeArgs)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        auto copy = SASS_MEMORY_COPY(map); // Can be optimized!
        auto it = copy->find(arguments[1]);
        if (it == copy->end()) {
          copy->insert({ arguments[1], arguments[2] });
        }
        else {
          it.value() = arguments[2];
        }
        return copy;
      }

      /*******************************************************************/

      BUILT_IN_FN(fnMapSetTwoArgs)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        MapObj copy = map = SASS_MEMORY_COPY(map); // Can be optimized
        auto cur = arguments[1]->start();
        auto end = arguments[1]->stop();
        auto size = arguments[1]->lengthAsList();
        if (size == 0) {
          throw Exception::RuntimeException(compiler,
            "Expected $args to contain a key.");
        }
        else if (size == 1) {
          throw Exception::RuntimeException(compiler,
            "Expected $args to contain a value.");
        }


        while (cur != end) {
          auto qwe = *cur;
          auto it = map->find(qwe);
          if (it != map->end()) {
            ValueObj inner = it->second;
            if (auto qwe = inner->isaMap()) {
              map = qwe;
            }
            else {  
              if (cur == end - 2) {
                ++cur;
                it.value() = *cur;
                break;
              }
              else {
                auto newMap = SASS_MEMORY_NEW(Map,
                  map->pstate(), {});
                it.value() = newMap;
                map = newMap;
              }
            }
          }
          else {
            if (cur == end - 2) {
              ++cur;
              map->insert({ qwe, *cur });
              break;
            }
            else {
              auto newMap = SASS_MEMORY_NEW(Map,
                map->pstate(), {});
              map->insert({ qwe, newMap });
              map = newMap;
            }
          }
          ++cur;
        }

        return copy.detach();
      }

      /*******************************************************************/

      BUILT_IN_FN(merge)
      {
        MapObj map1 = arguments[0]->assertMap(compiler, Strings::map1);
        MapObj map2 = arguments[1]->assertMap(compiler, Strings::map2);
        // We assign to ourself, so we can optimize this
        // This can shave off a few percent of run-time
        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (eval.assigne && eval.assigne->ptr() == map1.ptr()) {
          if (map1->refcount < AssignableRefCount + 1) {
            for (auto kv : map2->elements()) { map1->insertOrSet(kv); }
            return map1.detach();
          }
        }
        #endif
        Map* copy = SASS_MEMORY_COPY(map1);
        for (auto kv : map2->elements()) {
          copy->insertOrSet(kv); }
        return copy;
      }

      /*******************************************************************/

      BUILT_IN_FN(merge_many)
      {
        MapObj map1 = arguments[0]->assertMap(compiler, Strings::map1);

        auto size = arguments[1]->lengthAsList();

        if (size == 0) {
          throw Exception::RuntimeException(compiler,
            "Expected $args to contain a key.");
        }
        else if (size == 1) {
          throw Exception::RuntimeException(compiler,
            "Expected $args to contain a map.");
        }

        auto cur = arguments[1]->start();
        auto end = arguments[1]->stop() - 1;

        MapObj last = (end)->assertMap(compiler, Strings::map2);
        MapObj copy = map1 = SASS_MEMORY_COPY(map1);

        while (cur != end) {

          auto it = copy->find(*cur);
          if (it != copy->end()) {
            if (auto inner = it->second->isaMap()) {
              copy = SASS_MEMORY_COPY(inner);
              it.value() = copy;
            }
            else {
              Map* empty = SASS_MEMORY_NEW(Map,
                it->second->pstate());
              it.value() = empty;
              copy = empty;
            }
          }
          else {
            Map* empty = SASS_MEMORY_NEW(Map,
              last->pstate());
            copy->insert({ *cur, empty });
            copy = empty;
          }

          // if (!cur->isaMap()) {
          //   *cur = SASS_MEMORY_NEW(Map, )
          // }

          ++cur;
        }

        for (auto kv : last->elements()) {
          copy->insertOrSet(kv); }
        return map1.detach();
      }

      /*******************************************************************/

      // Because the signature below has an explicit `$key` argument, it doesn't
      // allow zero keys to be passed. We want to allow that case, so we add an
      // explicit overload for it.
      BUILT_IN_FN(remove_one)
      {
        return arguments[0]->assertMap(compiler, Strings::map);
      }

      /*******************************************************************/

      BUILT_IN_FN(remove_many)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);

        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (eval.assigne && eval.assigne->ptr() == map.ptr() && map->refcount < AssignableRefCount + 1) {
          map->erase(arguments[1]);
          for (Value* key : arguments[2]->start()) {
            map->erase(key);
          }
          return map.detach();
        }
        #endif

        MapObj copy = SASS_MEMORY_COPY(map);
        copy->erase(arguments[1]);
        for (Value* key : arguments[2]->start()) {
          copy->erase(key);
        }
        return copy.detach();
      }

      /*******************************************************************/

      BUILT_IN_FN(keys)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        return SASS_MEMORY_NEW(List, pstate, std::move(map->keys()), SASS_COMMA);
      }

      /*******************************************************************/

      BUILT_IN_FN(values)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        return SASS_MEMORY_NEW(List, pstate, std::move(map->values()), SASS_COMMA);
      }

      /*******************************************************************/

      BUILT_IN_FN(hasKey)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        Value* key = arguments[1]->assertValue(compiler, Strings::key);
        if (arguments[2]->lengthAsList() == 0) {
          return SASS_MEMORY_NEW(Boolean, pstate, map->has(key));
        }
        else {
          Map* current = map;
          auto first = current->find(key);
          if (first != current->end()) {
            current = first->second->isaMap();
            if (!current) {
              return SASS_MEMORY_NEW(Boolean, pstate, false);
            }
            auto cur = arguments[2]->start();
            auto end = arguments[2]->stop() - 1;
            Value* last = *end;
            while (cur != end) {
              auto inner = current->find(*cur);
              if (inner != current->end()) {
                if (auto imap = inner->second->isaMap()) {
                  current = imap;
                }
                else {
                  return SASS_MEMORY_NEW(Boolean, pstate, false);
                }
              }
              else {
                return SASS_MEMORY_NEW(Boolean, pstate, false);
              }
              ++cur;
            }

            // Still here, check now
            auto rv = current->find(last);
            return SASS_MEMORY_NEW(Boolean, pstate,
               rv != current->end());

          }
          else {
            return SASS_MEMORY_NEW(Boolean, pstate, false);
          }

        }
        return SASS_MEMORY_NEW(Boolean, pstate, false);
      }

      /*******************************************************************/

      BUILT_IN_FN(fnDeepMerge)
      {
        MapObj map1 = arguments[0]->assertMap(compiler, Strings::map1);
        MapObj map2 = arguments[1]->assertMap(compiler, Strings::map2);
        MapObj result = deepMergeImpl(map1, map2);
        return result.detach();
      }
      
      /*******************************************************************/

      BUILT_IN_FN(fnDeepRemove)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        MapObj result = SASS_MEMORY_COPY(map);
        Map* level = result;

        auto cur = arguments[2]->start();
        auto end = arguments[2]->stop();

        if (cur == end) {
          level->erase(arguments[1]);
          return result.detach();
        }
        else {
          auto child = level->find(arguments[1]);
          if (child == level->end()) return result.detach();
          auto childMap = child->second->isaMap();
          if (childMap == nullptr) return result.detach();
          child.value() = level = SASS_MEMORY_COPY(childMap);
        }

        while (cur != end) {
          if (cur.isLast()) {
            level->erase(*cur);
            return result.detach();
          }
          else {
            // Go further down one key
            auto child = level->find(*cur);
            if (child == level->end()) return result.detach();
            auto childMap = child->second->isaMap();
            if (childMap == nullptr) return result.detach();
            child.value() = level = SASS_MEMORY_COPY(childMap);
          }
          ++cur;
        }
        return result.detach();
      }

      /*******************************************************************/

      void registerFunctions(Compiler& ctx)
      {
        BuiltInMod& module(ctx.createModule("map"));

        module.addFunction(key_set, ctx.createBuiltInOverloadFns(key_map_set, {
          std::make_pair("$map, $key, $value", fnMapSetThreeArgs),
          std::make_pair("$map, $args...", fnMapSetTwoArgs)
        }));

        module.addFunction(key_get, ctx.registerBuiltInFunction(key_map_get, "$map, $key, $keys...", get));

        module.addFunction(key_merge, ctx.registerBuiltInOverloadFns(key_map_merge, {
          std::make_pair("$map1, $map2", merge),
          std::make_pair("$map1, $args...", merge_many)
          }));

        module.addFunction(key_remove, ctx.registerBuiltInOverloadFns(key_map_remove, {
          std::make_pair("$map", remove_one),
          std::make_pair("$map, $key, $keys...", remove_many)
          }));

        module.addFunction(key_keys, ctx.registerBuiltInFunction(key_map_keys, "$map", keys));
        module.addFunction(key_values, ctx.registerBuiltInFunction(key_map_values, "$map", values));
        module.addFunction(key_has_key, ctx.registerBuiltInFunction(key_map_has_key, "$map, $key, $keys...", hasKey));

        module.addFunction(key_deep_merge, ctx.createBuiltInFunction(key_deep_merge, "$map1, $map2", fnDeepMerge));
        module.addFunction(key_deep_remove, ctx.createBuiltInFunction(key_deep_remove, "$map, $key, $keys...", fnDeepRemove));

      }

      /*******************************************************************/

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}

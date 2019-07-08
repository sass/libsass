/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "fn_maps.hpp"

#include "compiler.hpp"
#include "ast_values.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Maps {

      /*******************************************************************/

      BUILT_IN_FN(get)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        Value* key = arguments[1]->assertValue(compiler, Strings::key);
        auto it = map->find(key);
        if (it != map->end()) return it->second;
        return SASS_MEMORY_NEW(Null, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(merge)
      {
        MapObj map1 = arguments[0]->assertMap(compiler, Strings::map1);
        MapObj map2 = arguments[1]->assertMap(compiler, Strings::map2);
        // We assign to ourself, so we can optimize this
        // This can shave off a few percent of run-time
        #ifdef SASS_OPTIMIZE_SELF_ASSIGN
        if (selfAssign && map1->refcount < AssignableRefCount + 1) {
          for (auto kv : map2->elements()) { map1->insertOrSet(kv); }
          return map1.detach();
        }
        #endif
        Map* copy = SASS_MEMORY_COPY(map1);
        for (auto kv : map2->elements()) {
          copy->insertOrSet(kv); }
        return copy;
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
        if (selfAssign && map->refcount < AssignableRefCount + 1) {
          map->erase(arguments[1]);
          for (Value* key : arguments[2]->iterator()) {
            map->erase(key);
          }
          return map.detach();
        }
        #endif

        MapObj copy = SASS_MEMORY_COPY(map);
        copy->erase(arguments[1]);
        for (Value* key : arguments[2]->iterator()) {
          copy->erase(key);
        }
        return copy.detach();
      }

      /*******************************************************************/

      BUILT_IN_FN(keys)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        return SASS_MEMORY_NEW(List,
          pstate, map->keys(), SASS_COMMA);
      }

      /*******************************************************************/

      BUILT_IN_FN(values)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        return SASS_MEMORY_NEW(List,
          pstate, map->values(), SASS_COMMA);
      }

      /*******************************************************************/

      BUILT_IN_FN(hasKey)
      {
        MapObj map = arguments[0]->assertMap(compiler, Strings::map);
        Value* key = arguments[1]->assertValue(compiler, Strings::key);
        return SASS_MEMORY_NEW(Boolean, pstate, map->has(key));
      }

      /*******************************************************************/

      void registerFunctions(Compiler& ctx)
	    {
		    ctx.registerBuiltInFunction("map-get", "$map, $key", get);
		    ctx.registerBuiltInFunction("map-merge", "$map1, $map2", merge);
		    ctx.registerBuiltInOverloadFns("map-remove", {
			    std::make_pair("$map", remove_one),
			    std::make_pair("$map, $key, $keys...", remove_many)
        });
		    ctx.registerBuiltInFunction("map-keys", "$map", keys);
		    ctx.registerBuiltInFunction("map-values", "$map", values);
		    ctx.registerBuiltInFunction("map-has-key", "$map, $key", hasKey);
	    }

      /*******************************************************************/

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}

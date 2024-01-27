/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ENVIRONMENT_CNT_H
#define SASS_ENVIRONMENT_CNT_H

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "flat_map.hpp"
#include "ast_fwd_decl.hpp"
#include "environment_key.hpp"

namespace Sass {

  template<typename T>
  using EnvKeyMap = UnorderedMap<
    const EnvKey, T, hashEnvKey, equalsEnvKey,
    Sass::Allocator<std::pair<const EnvKey, T>>
  >;

  using EnvKeySet = UnorderedSet<
    EnvKey, hashEnvKey, equalsEnvKey,
    Sass::Allocator<EnvKey>
  >;

  template<typename T>
  using ModuleMap = UnorderedMap<
    const sass::string, T, hashString, equalsString,
    std::allocator<std::pair<const sass::string, T>>
  >;

  using ModuleSet = UnorderedSet<
    sass::string, hashString, equalsString,
    std::allocator<sass::string>
  >;

  template<typename T>
  // Performance comparison on MSVC and bolt-bench:
  // tsl::hopscotch_map is 10% slower than Sass::FlatMap
  // std::unordered_map a bit faster than tsl::hopscotch_map
  // Sass::FlapMap is 10% faster than any other container
  // Note: only due to our very specific usage patterns!
  using EnvKeyFlatMap = FlatMap<EnvKey, T>;

  typedef sass::vector<EnvKey> EnvKeys;
  typedef EnvKeyFlatMap<ValueObj> ValueFlatMap;
  typedef EnvKeyFlatMap<ExpressionObj> ExpressionFlatMap;

  // Helper typedefs to test implementations
  // We seem to need order preserving at least for globals
  typedef EnvKeyFlatMap<uint32_t> VidxEnvKeyMap;
  typedef EnvKeyFlatMap<uint32_t> MidxEnvKeyMap;
  typedef EnvKeyFlatMap<uint32_t> FidxEnvKeyMap;

};

#endif

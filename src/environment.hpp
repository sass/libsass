/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ENVIRONMENT_HPP
#define SASS_ENVIRONMENT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_nodes.hpp"
#include "environment_key.hpp"
#include "environment_cnt.hpp"

namespace std {
  template <>
  struct hash<Sass::EnvKey> {
  public:
    inline size_t operator()(const Sass::EnvKey& name) const
    {
      return name.hash();
    }
  };
};

namespace Sass {

  void mergeForwards(
    EnvRefs* idxs,
    Module* module,
    bool isShown,
    bool isHidden,
    const sass::string prefix,
    const std::set<EnvKey>& toggledVariables,
    const std::set<EnvKey>& toggledCallables,
    Logger& logger);

}

#endif

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_MODULES_HPP
#define SASS_MODULES_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"
#include "environment_cnt.hpp"
#include "environment_stack.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // A module is first and foremost a unit that provides variables,
  // functions and mixins. We know built-in modules, which don't have
  // any content ever and are always loaded and available. Custom
  // modules are loaded from any given url. They are related and
  // linked to regular @imports, as those are also some kind of
  // special modules. An @import will load as a module, but that
  // module with not be compiled until used by @forward or @use.
  /////////////////////////////////////////////////////////////////////////
  // When a module is @imported, all its local root variables
  // are "exposed" to the caller's scope. Those are actual new
  // instances of the variables, as e.g. the same file might be
  // imported into different style-rules. The variables will then
  // really exist in the env of those style-rules. Similarly when
  // importing on the root scope, the variables are not shared with
  // the internal ones (the ones we reference when being @used).
  /////////////////////////////////////////////////////////////////////////
  // To support this, we can't statically optimize variable accesses
  // across module boundaries. We need to mark environments to remember
  // in which context a module was brought into the tree. For imports
  // we simply skip the whole frame. Instead the lookup should find
  // the variable we created in the caller's scope (Optimizations ToDo).
  /////////////////////////////////////////////////////////////////////////

  class Module : public Env
  {
  public:

    // Flag for internal modules
    // They don't have any content
    bool isBuiltIn = false;

    // Only makes sense for non built-ins
    // True once the content has been loaded
    bool isLoaded = false;

    // Only makes sense for non built-ins
    // True once the module is compiled and ready
    bool isCompiled = false;

    // The compiled AST-Tree
    CssParentNodeObj compiled;

    // All @forward rules get merged into these objects
    // Those are not available on the local scope, they
    // are only used when another module consumes us!
    // On @use they must be merged into local scope!
    VidxEnvKeyMap mergedFwdVar;
    MidxEnvKeyMap mergedFwdMix;
    FidxEnvKeyMap mergedFwdFn;

    // Modules that this module uses.
    sass::vector<Root*> upstream;

    ModuleMap<std::pair<EnvRefs*, Module*>> moduse;

    // The extensions defined in this module, which is also able to update
    // [css]'s style rules in-place based on downstream extensions.
    ExtensionStoreObj extender = nullptr;

    // Special set with global assignments
    // Needed for imports within style-rules
    // ToDo: not really tested via specs yet?
    // EnvKeySet globals;

  public:

    Module(EnvRefs* idxs);

    // Check if there are any unsatisfied extends (will throw)
    bool checkForUnsatisfiedExtends3(Extension& unsatisfied) const;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class BuiltInMod : public Module
  {
  public:
    void addFunction(const EnvKey& name, uint32_t offset);
    void addVariable(const EnvKey& name, uint32_t offset);
    void addMixin(const EnvKey& name, uint32_t offset);
    BuiltInMod(EnvRoot& root);
    ~BuiltInMod();
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#include "extender.hpp"

#endif

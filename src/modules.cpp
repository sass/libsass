/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "modules.hpp"

#include "stylesheet.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Module::Module(EnvRefs* idxs) :
    Env(idxs),
    extender()
  {}

  // Check if there are any unsatisfied extends (will throw)

  bool Module::checkForUnsatisfiedExtends3(Extension & unsatisfied) const
  {
    ExtSmplSelSet originals;
    for (auto& entry : extender->selectors54) {
      originals.insert(entry.first);
    }

    if (extender->checkForUnsatisfiedExtends2(unsatisfied)) {
      return true;
    }
    for (auto& asd : upstream) {
      if (asd->checkForUnsatisfiedExtends3(unsatisfied)) {
        return true;
      }
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BuiltInMod::BuiltInMod(EnvRoot& root) :
    Module(new EnvRefs(
      root,
      nullptr,
      false,  // isImport
      true,   // isInternal
      false)) // isSemiGlobal
  {
    isBuiltIn = true;
    isLoaded = true;
    isCompiled = true;
  }

  BuiltInMod::~BuiltInMod()
  {
    delete idxs;
  }

  void BuiltInMod::addFunction(const EnvKey& name, uint32_t offset)
  {
    idxs->fnIdxs[name] = offset;
  }

  void BuiltInMod::addVariable(const EnvKey& name, uint32_t offset)
  {
    idxs->varIdxs[name] = offset;
  }

  void BuiltInMod::addMixin(const EnvKey& name, uint32_t offset)
  {
    idxs->mixIdxs[name] = offset;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

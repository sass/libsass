/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_variable.hpp"

#include "compiler.hpp"

using namespace Sass;

extern "C" {

  // Getter for lexical variable (lexical to scope where function is called).
  // Note: C-API function can only access existing variables and not create new ones!
  struct SassValue* ADDCALL sass_env_get_lexical (struct SassCompiler* compiler, const char* name) {
    EnvRef ref = Compiler::unwrap(compiler).varRoot.findVarIdx(EnvKey{ name }, "", false);
    if (ref.isValid()) return Value::wrap(Compiler::unwrap(compiler).varRoot.getVariable(ref));
    return nullptr;
  }

  // Setter for lexical variable (lexical to scope where function is called).
  // Returns true if variable was set or false if it does not exist (we can't create it)
  // Note: C-API function can only access existing variables and not create new ones!
  bool ADDCALL sass_env_set_lexical (struct SassCompiler* compiler, const char* name, struct SassValue* val) {
    EnvRef ref = Compiler::unwrap(compiler).varRoot.findVarIdx(EnvKey{ name }, "", false);
    if (ref.isValid()) Compiler::unwrap(compiler).varRoot.setVariable(ref, &Value::unwrap(val), false);
    return ref.isValid();
  }

  // Getter for global variable (only variables on the root scope are considered).
  // Note: C-API function can only access existing variables and not create new ones!
  struct SassValue* ADDCALL sass_env_get_global (struct SassCompiler* compiler, const char* name) {
    EnvRef ref = Compiler::unwrap(compiler).varRoot.findVarIdx(EnvKey{ name }, "", true);
    if (ref.isValid()) return Value::wrap(Compiler::unwrap(compiler).varRoot.getVariable(ref));
    return nullptr;
  }

  // Setter for global variable (only variables on the root scope are considered).
  // Returns true if variable was set or false if it does not exist (we can't create it)
  // Note: C-API function can only access existing variables and not create new ones!
  bool ADDCALL sass_env_set_global (struct SassCompiler* compiler, const char* name, struct SassValue* val) {
    EnvRef ref = Compiler::unwrap(compiler).varRoot.findVarIdx(EnvKey{ name }, "", true);
    if (ref.isValid()) Compiler::unwrap(compiler).varRoot.setVariable(ref, &Value::unwrap(val), false);
    return ref.isValid();
  }

}

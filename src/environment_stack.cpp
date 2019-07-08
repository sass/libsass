/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "environment_stack.hpp"

#include "ast_expressions.hpp"
#include "ast_statements.hpp"
#include "exceptions.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Each parsed scope gets its own environment frame
  /////////////////////////////////////////////////////////////////////////

  // The root is used for all runtime state
  // Also contains parsed root scope stack
  EnvRoot::EnvRoot(
    EnvFrameVector& stack) :
    EnvFrame(*this, stack)
  {
    // Initialize as not active yet
    root.varFramePtr.push_back(0xFFFFFFFF);
    root.mixFramePtr.push_back(0xFFFFFFFF);
    root.fnFramePtr.push_back(0xFFFFFFFF);
    // Account for allocated memory
    root.scopes.push_back(idxs);
    // Push onto our stack
    stack.push_back(this);
  }
  // EO EnvRoot ctor

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Root-frame constructor
  // Invoked by EnvRoot ctor
  EnvFrame::EnvFrame(
    EnvRoot& root,
    EnvFrameVector& stack) :
    stack(stack),
    permeable(false),
    parent(root),
    root(root),
    idxs(new VarRefs(
      nullptr, 0, 0, 0,
      false)),
    varIdxs(idxs->varIdxs),
    mixIdxs(idxs->mixIdxs),
    fnIdxs(idxs->fnIdxs),
    assignments(idxs->assignments),
    variables(idxs->variables)
  {
    // Don't access root, not yet initialized
  }
  // EO EnvFrame ctor

  // Value constructor
  EnvFrame::EnvFrame(
    EnvFrameVector& stack,
    bool permeable) :
    stack(stack),
    permeable(permeable),
    parent(*stack.back()),
    root(stack.back()->root),
    idxs(new VarRefs(parent.idxs,
      uint32_t(root.varFramePtr.size()),
      uint32_t(root.mixFramePtr.size()),
      uint32_t(root.fnFramePtr.size()),
      permeable)),
    varIdxs(idxs->varIdxs),
    mixIdxs(idxs->mixIdxs),
    fnIdxs(idxs->fnIdxs),
    assignments(idxs->assignments),
    variables(idxs->variables)
  {
    // Initialize stacks as not active yet
    root.varFramePtr.push_back(0xFFFFFFFF);
    root.mixFramePtr.push_back(0xFFFFFFFF);
    root.fnFramePtr.push_back(0xFFFFFFFF);
    // Account for allocated memory
    root.scopes.push_back(idxs);
    // Check and prevent stack smashing
    if (stack.size() > MAX_NESTING) {
      throw Exception::RecursionLimitError();
    }
    // Push onto our stack
    stack.push_back(this);
  }
  // EO EnvFrame ctor

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Remove frame from stack on destruction
  EnvFrame::~EnvFrame()
  {
    // Pop from stack
    stack.pop_back();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Register new variable on local stack
  // Invoked mostly by stylesheet parser
  VarRef EnvFrame::createVariable(
    const EnvKey& name)
  {
    // Get local offset to new variable
    uint32_t offset = (uint32_t)varIdxs.size();
    // Remember the variable name
    varIdxs[name] = offset;
    // Return stack index reference
    return { idxs->varFrame, offset };
  }
  // EO createVariable

  // Register new function on local stack
  // Mostly invoked by built-in functions
  // Then invoked for custom C-API function
  // Finally for every parsed function rule
  VarRef EnvFrame::createFunction(
    const EnvKey& name)
  {
    // Check for existing function
    auto it = fnIdxs.find(name);
    if (it != fnIdxs.end()) {
      return { idxs->fnFrame, it->second };
    }
    // Get local offset to new function
    uint32_t offset = (uint32_t)fnIdxs.size();
    // Remember the function name
    fnIdxs[name] = offset;
    // Return stack index reference
    return { idxs->fnFrame, offset };
  }
  // EO createFunction

  // Register new mixin on local stack
  // Only invoked for mixin rules
  // But also for content blocks
  VarRef EnvFrame::createMixin(
    const EnvKey& name)
  {
    // Check for existing mixin
    auto it = mixIdxs.find(name);
    if (it != mixIdxs.end()) {
      return { idxs->mixFrame, it->second };
    }
    // Get local offset to new mixin
    uint32_t offset = (uint32_t)mixIdxs.size();
    // Remember the mixin name
    mixIdxs[name] = offset;
    // Return stack index reference
    return { idxs->mixFrame, offset };
  }
  // EO createMixin

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Get local variable by name, needed for most simplistic case
  // for static variable optimization in loops. When we know that
  // there is an existing local variable, we can always use that!
  VarRef EnvFrame::getLocalVariableIdx(const EnvKey& name)
  {
    auto it = varIdxs.find(name);
    if (it == varIdxs.end()) return {};
    return { idxs->varFrame, it->second };
  }
  // EO getLocalVariableIdx

  // Return lookups in lexical manner. If [passThrough] is false,
  // we abort the lexical lookup on any non-permeable scope frame.
  VarRef EnvFrame::getMixinIdx(const EnvKey& name, bool passThrough)
  {
    EnvFrame* current = this;
    while (current != nullptr) {
      // Check if we already have this var
      auto it = current->mixIdxs.find(name);
      if (it != current->mixIdxs.end()) {
        return { current->idxs->mixFrame, it->second };
      }
      current = current->getParent(passThrough);
    }
    // Not found
    return VarRef{};
  }
  // EO getMixinIdx

  // Return lookups in lexical manner. If [passThrough] is false,
  // we abort the lexical lookup on any non-permeable scope frame.
  VarRef EnvFrame::getFunctionIdx(const EnvKey& name, bool passThrough)
  {
    EnvFrame* current = this;
    while (current != nullptr) {
      // Check if we already have this var
      auto it = current->fnIdxs.find(name);
      if (it != current->fnIdxs.end()) {
        return { current->idxs->fnFrame, it->second };
      }
      current = current->getParent(passThrough);
    }
    // Not found
    return VarRef{};
  }
  // EO getFunctionIdx

  // Return lookups in lexical manner. If [passThrough] is false,
  // we abort the lexical lookup on any non-permeable scope frame.
  VarRef EnvFrame::getVariableIdx(const EnvKey& name, bool passThrough)
  {
    EnvFrame* current = this;
    while (current != nullptr) {
      auto it = current->varIdxs.find(name);
      if (it != current->varIdxs.end()) {
        return { current->idxs->fnFrame, it->second };
      }
      current = current->getParent(passThrough);
    }
    // Not found
    return {};
  }
  // EO getVariableIdx
  
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Update variable references and assignments
  void EnvRoot::finalizeScopes()
  {

    // Process every scope ever seen
    for (VarRefs* scope : scopes) {

      // Process all variable expression (e.g. variables used in sass-scripts).
      // Move up the tree to find possible parent scopes also containing this
      // variable. On runtime we will return the first item that has a value set.
      for (VariableExpression* variable : scope->variables)
      {
        VarRefs* current = scope;
        const EnvKey& name(variable->name());
        while (current != nullptr) {
          // Find the variable name in current scope
          auto found = current->varIdxs.find(name);
          if (found != current->varIdxs.end()) {
            // Append alternative
            variable->vidxs()
              .emplace_back(VarRef{
                current->varFrame,
                found->second
              });
          }
          // Process the parent scope
          current = current->pscope;
        }
      }
      // EO variables

      // Process all variable assignment rules. Assignments can bleed up to the
      // parent scope under certain conditions. We bleed up regular style rules,
      // but not into the root scope itself (until it is semi global scope).
      for (AssignRule* assignment : scope->assignments)
      {
        VarRefs* current = scope;
        while (current != nullptr) {
          // If current scope is rooted we don't look further, as we
          // already created the local variable and assigned reference.
          if (!current->permeable) break;
          // Start with the parent
          current = current->pscope;
          // Find the variable name in current scope
          auto found = current->varIdxs
            .find(assignment->variable());
          if (found != current->varIdxs.end()) {
            // Append another alternative
            assignment->vidxs()
              .emplace_back(VarRef{
                current->varFrame,
                found->second
              });
          }
          // EO found current var
        }
        // EO while parent
      }
      // EO assignments

    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Get value instance by stack index reference
  // Just converting and returning reference to array offset
  ValueObj& EnvRoot::getVariable(const VarRef& vidx)
  {
    return variables[size_t(varFramePtr[vidx.frame]) + vidx.offset];
  }
  // EO getVariable

  // Get function instance by stack index reference
  // Just converting and returning reference to array offset
  UserDefinedCallableObj& EnvRoot::getFunction(const VarRef& fidx)
  {
    return functions[size_t(fnFramePtr[fidx.frame]) + fidx.offset];
  }
  // EO getFunction

  // Get mixin instance by stack index reference
  // Just converting and returning reference to array offset
  UserDefinedCallableObj& EnvRoot::getMixin(const VarRef& midx)
  {
    return mixins[size_t(mixFramePtr[midx.frame]) + midx.offset];
  }
  // EO getMixin

  // Set items on runtime/evaluation phase via references
  // Just converting reference to array offset and assigning
  void EnvRoot::setVariable(const VarRef& vidx, ValueObj value)
  {
    variables[size_t(varFramePtr[vidx.frame]) + vidx.offset] = value;
  }
  // EO setVariable

  // Set items on runtime/evaluation phase via references
  // Just converting reference to array offset and assigning
  void EnvRoot::setVariable(uint32_t frame, uint32_t offset, ValueObj value)
  {
    variables[size_t(varFramePtr[frame]) + offset] = value;
  }
  // EO setVariable

  // Set items on runtime/evaluation phase via references
  // Just converting reference to array offset and assigning
  void EnvRoot::setFunction(const VarRef& fidx, UserDefinedCallableObj value)
  {
    functions[size_t(fnFramePtr[fidx.frame]) + fidx.offset] = value;
  }
  // EO setFunction

  // Set items on runtime/evaluation phase via references
  // Just converting reference to array offset and assigning
  void EnvRoot::setMixin(const VarRef& midx, UserDefinedCallableObj value)
  {
    mixins[size_t(mixFramePtr[midx.frame]) + midx.offset] = value;
  }
  // EO setMixin

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Get a mixin associated with the under [name].
  // Will lookup from the last runtime stack scope.
  // We will move up the runtime stack until we either
  // find a defined mixin or run out of parent scopes.
  UserDefinedCallable* EnvRoot::getMixin(const EnvKey& name) const
  {
    if (stack.empty()) return {};
    uint32_t idx = uint32_t(stack.size() - 1);
    const VarRefs* current = stack[idx];
    while (current) {
      auto it = current->mixIdxs.find(name);
      if (it != current->mixIdxs.end()) {
        const VarRef vidx{ current->mixFrame, it->second };
        UserDefinedCallableObj& value = root.getMixin(vidx);
        if (!value.isNull()) return value;
      }
      if (current->pscope == nullptr) break;
      else current = current->pscope;
    }
    return {};
  }
  // EO getMixin

  // Get a function associated with the under [name].
  // Will lookup from the last runtime stack scope.
  // We will move up the runtime stack until we either
  // find a defined function or run out of parent scopes.
  UserDefinedCallable* EnvRoot::getFunction(const EnvKey& name) const
  {
    if (stack.empty()) return {};
    uint32_t idx = uint32_t(stack.size() - 1);
    const VarRefs* current = stack[idx];
    while (current) {
      auto it = current->fnIdxs.find(name);
      if (it != current->fnIdxs.end()) {
        const VarRef vidx{ current->fnFrame, it->second };
        UserDefinedCallableObj& value = root.getFunction(vidx);
        if (!value.isNull()) return value;
      }
      if (current->pscope == nullptr) break;
      else current = current->pscope;
    }
    return {};
  }
  // EO getFunction

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Get a value associated with the variable under [name].
  // If [global] flag is given, the lookup will be in the root.
  // Otherwise lookup will be from the last runtime stack scope.
  // We will move up the runtime stack until we either find a 
  // defined variable with a value or run out of parent scopes.
  Value* EnvRoot::getVariable(const EnvKey& name, bool global) const
  {
    if (stack.empty()) return {};
    uint32_t idx = global ? 0 :
      (uint32_t)stack.size() - 1;
    const VarRefs* current = stack[idx];
    while (current) {
      auto it = current->varIdxs.find(name);
      if (it != current->varIdxs.end()) {
        const VarRef vidx{ current->varFrame, it->second };
        ValueObj& value = root.getVariable(vidx);
        if (value != nullptr) { return value; }
      }
      if (current->pscope == nullptr) break;
      else current = current->pscope;
    }
    return {};
  }
  // EO getVariable

  // Set a value associated with the variable under [name].
  // If [global] flag is given, the lookup will be in the root.
  // Otherwise lookup will be from the last runtime stack scope.
  // We will move up the runtime stack until we either find a 
  // defined variable with a value or run out of parent scopes.
  void EnvRoot::setVariable(const EnvKey& name, ValueObj val, bool global)
  {
    if (stack.empty()) return;
    uint32_t idx = global ? 0 :
      (uint32_t)stack.size() - 1;
    const VarRefs* current = stack[idx];
    while (current) {
      auto it = current->varIdxs.find(name);
      if (it != current->varIdxs.end()) {
        const VarRef vidx{ current->varFrame, it->second };
        ValueObj& value = root.getVariable(vidx);
        if (value != nullptr) { value = val; return; }
      }
      if (current->pscope == nullptr) break;
      else current = current->pscope;
    }
  }
  // EO setVariable

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Very small helper for debugging
  sass::string VarRef::toString() const
  {
    sass::sstream strm;
    strm << frame << ":" << offset;
    return strm.str();
  }
  // EO toString

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

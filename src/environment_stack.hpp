/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VAR_STACK_HPP
#define SASS_VAR_STACK_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"
#include "environment_cnt.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Forward declare
  class VarRef;
  class VarRefs;
  class EnvRoot;
  class EnvFrame;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Helper typedefs to test implementations
  typedef EnvKeyMap<uint32_t> VidxEnvKeyMap;
  typedef EnvKeyMap<uint32_t> MidxEnvKeyMap;
  typedef EnvKeyMap<uint32_t> FidxEnvKeyMap;

  // Helper typedef for our frame stack type
  typedef sass::vector<EnvFrame*> EnvFrameVector;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Base class/struct for variable references. Each variable belongs to an
  // environment frame (determined as we see variables during parsing). This
  // is similar to how C organizes local stack variables via frame offsets.
  class VarRef {

  public:

    uint32_t frame;
    uint32_t offset;

    VarRef() :
      frame(0xFFFFFFFF),
      offset(0xFFFFFFFF)
    {}

    VarRef(
      uint32_t frame,
      uint32_t offset,
      bool overwrites = false) :
      frame(frame),
      offset(offset)
    {}

    bool operator==(const VarRef& rhs) const {
      return frame == rhs.frame
        && offset == rhs.offset;
    }

    bool operator<(const VarRef& rhs) const {
      if (frame < rhs.frame) return true;
      return offset < rhs.offset;
    }

    bool isValid() const { // 3%
      return offset != 0xFFFFFFFF
        && frame != 0xFFFFFFFF;
    }

    operator bool() const {
      return isValid();
    }

    // Very small helper for debugging
    sass::string toString() const;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Runtime query structure
  // Created for every EnvFrame
  // Survives the actual EnvFrame
  class VarRefs {
  public:

    // Parent is needed during runtime for
    // dynamic setter and getter by EnvKey.
    VarRefs* pscope;

    // Rules like `@if`, `@for` etc. are semi-global (permeable).
    // Assignments directly in those can bleed to the root scope.
    bool permeable;

    // Parents for specific types
    uint32_t varFrame;
    uint32_t mixFrame;
    uint32_t fnFrame;

    // Remember named mappings
    // Used for runtime lookups
    // ToDo: test EnvKeyFlatMap
    VidxEnvKeyMap varIdxs;
    MidxEnvKeyMap mixIdxs;
    FidxEnvKeyMap fnIdxs;

    // Keep track of assignments and variables for dynamic runtime lookups.
    // This is needed only for loops, due to sass "weird" variable scoping.
    std::vector<AssignRuleObj> assignments;
    std::vector<VariableExpressionObj> variables;

    // Value constructor
    VarRefs(VarRefs* pscope,
      uint32_t varFrame,
      uint32_t mixFrame,
      uint32_t fnFrame,
      bool permeable) :
      pscope(pscope),
      permeable(permeable),
      varFrame(varFrame),
      mixFrame(mixFrame),
      fnFrame(fnFrame)
    {}

  };

  /////////////////////////////////////////////////////////////////////////
  // EnvFrames are created during the parsing phase.
  /////////////////////////////////////////////////////////////////////////

  class EnvFrame {

    // We work together
    friend class EnvRoot;

  public:

    // Reference to stack
    // We manage it ourself
    EnvFrameVector& stack;

    // New variables are hoisted at closest non-permeable.
    // Lookups are still looking at all parents and root.
    bool permeable;

    // Reference to parent
    EnvFrame& parent;

    // Cache root reference
    EnvRoot& root;

    // Our runtime object
    VarRefs* idxs;

    // References into runtime object
    VidxEnvKeyMap& varIdxs;
    MidxEnvKeyMap& mixIdxs;
    FidxEnvKeyMap& fnIdxs;

    // Keep track of assignments and variables for dynamic runtime lookups.
    // This is needed only for loops, due to sass "weird" variable scoping.
    std::vector<AssignRuleObj>& assignments;
    std::vector<VariableExpressionObj>& variables;

  private:

    // Root-frame constructor
    // Invoked by EnvRoot ctor
    EnvFrame(
      EnvRoot& root,
      EnvFrameVector& stack);

  public:

    // Value constructor
    EnvFrame(
      EnvFrameVector& stack,
      // Rules like `@if`, `@for` etc. are semi-global (permeable).
      // Assignments directly in those can bleed to the root scope.
      bool permeable = false);

    // Destructor
    ~EnvFrame();

    // Test if we are top frame
    bool isRoot() const {
      // Check if raw pointers are equal
      return this == (EnvFrame*)&root;
    }

    // Get next parent, but break on root
    EnvFrame* getParent(bool passThrough = false) {
      if (isRoot())
        return nullptr;
      if (!passThrough)
        if (!permeable)
          return nullptr;
      return &parent;
    }

    /////////////////////////////////////////////////////////////////////////
    // Register an occurrence during parsing, reserving the offset.
    // Only structures are create when calling this, the real work
    // is done on runtime, where actual stack objects are queried.
    /////////////////////////////////////////////////////////////////////////

    // Register new variable on local stack
    // Invoked mostly by stylesheet parser
    VarRef createVariable(const EnvKey& name);

    // Register new function on local stack
    // Mostly invoked by built-in functions
    // Then invoked for custom C-API function
    // Finally for every parsed function rule
    VarRef createFunction(const EnvKey& name);

    // Register new mixin on local stack
    // Only invoked for mixin rules
    // But also for content blocks
    VarRef createMixin(const EnvKey& name);

    // Get local variable by name, needed for most simplistic case
    // for static variable optimization in loops. When we know that
    // there is an existing local variable, we can always use that!
    VarRef getLocalVariableIdx(const EnvKey& name);

    // Return mixin in lexical manner. If [passThrough] is false,
    // we abort the lexical lookup on any non-permeable scope frame.
    VarRef getMixinIdx(const EnvKey& name, bool passThrough = true);

    // Return function in lexical manner. If [passThrough] is false,
    // we abort the lexical lookup on any non-permeable scope frame.
    VarRef getFunctionIdx(const EnvKey& name, bool passThrough = true);

    // Return variable in lexical manner. If [passThrough] is false,
    // we abort the lexical lookup on any non-permeable scope frame.
    VarRef getVariableIdx(const EnvKey& name, bool passThrough = false);
      
 };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class EnvRoot :
    public EnvFrame {

  private:

    // We work together
    friend class Compiler;
    friend class EnvScope;
    friend class EnvFrame;

    // Growable runtime stack (get offset by xxxFramePtr).
    // These vectors are the main stacks during runtime.
    // When a scope with two variables is executed, two
    // new items are added to the variables stack. If the
    // same scope is called more than once, its variables
    // are added multiple times so we can revert to them.
    // variables[varFramePtr[vidx.frame] + vidx.offset]
    sass::vector<ValueObj> variables;
    sass::vector<UserDefinedCallableObj> mixins;
    sass::vector<UserDefinedCallableObj> functions;

    // Every scope we execute in sass gets an entry here.
    // The value stored here is the base address of the
    // active scope, used to calculate the final offset.
    // Gives current offset into growable runtime stack.
    // Old values are restored when scopes are exited.
    // Access it by absolute `frameOffset`
    sass::vector<uint32_t> varFramePtr;
    sass::vector<uint32_t> mixFramePtr;
    sass::vector<uint32_t> fnFramePtr;

    // All created runtime variable objects.
    // Needed to track the memory allocations
    // And useful to resolve parents indirectly
    // Access it by absolute `frameOffset`
    sass::vector<VarRefs*> scopes;

    // The current runtime stack
    sass::vector<const VarRefs*> stack;

  public:

    // Value constructor
    EnvRoot(EnvFrameVector& stack);

    // Destructor
    ~EnvRoot() {
      // Take care of scope pointers
      for (VarRefs* idx : scopes) {
        delete idx;
      }
    }

    // Update variable references and assignments
    // Process all variable expression (e.g. variables used in sass-scripts).
    // Move up the tree to find possible parent scopes also containing this
    // variable. On runtime we will return the first item that has a value set.
    // Process all variable assignment rules. Assignments can bleed up to the
    // parent scope under certain conditions. We bleed up regular style rules,
    // but not into the root scope itself (until it is semi global scope).
    void finalizeScopes();

    // Runtime check to see if we are currently in global scope
    bool isGlobal() const { return root.stack.size() == 1; }

    // Get value instance by stack index reference
    // Just converting and returning reference to array offset
    ValueObj& getVariable(const VarRef& vidx);

    // Get function instance by stack index reference
    // Just converting and returning reference to array offset
    UserDefinedCallableObj& getFunction(const VarRef& vidx);

    // Get mixin instance by stack index reference
    // Just converting and returning reference to array offset
    UserDefinedCallableObj& getMixin(const VarRef& midx);

    // Set items on runtime/evaluation phase via references
    // Just converting reference to array offset and assigning
    void setVariable(const VarRef& vidx, ValueObj value);

    // Set items on runtime/evaluation phase via references
    // Just converting reference to array offset and assigning
    void setVariable(uint32_t frame, uint32_t offset, ValueObj value);

    // Set items on runtime/evaluation phase via references
    // Just converting reference to array offset and assigning
    void setFunction(const VarRef& fidx, UserDefinedCallableObj value);

    // Set items on runtime/evaluation phase via references
    // Just converting reference to array offset and assigning
    void setMixin(const VarRef& midx, UserDefinedCallableObj value);

    // Get a mixin associated with the under [name].
    // Will lookup from the last runtime stack scope.
    // We will move up the runtime stack until we either
    // find a defined mixin or run out of parent scopes.
    UserDefinedCallable* getMixin(const EnvKey& name) const;

    // Get a function associated with the under [name].
    // Will lookup from the last runtime stack scope.
    // We will move up the runtime stack until we either
    // find a defined function or run out of parent scopes.
    UserDefinedCallable* getFunction(const EnvKey& name) const;

    // Get a value associated with the variable under [name].
    // If [global] flag is given, the lookup will be in the root.
    // Otherwise lookup will be from the last runtime stack scope.
    // We will move up the runtime stack until we either find a 
    // defined variable with a value or run out of parent scopes.
    Value* getVariable(const EnvKey& name, bool global = false) const;

    // Set a value associated with the variable under [name].
    // If [global] flag is given, the lookup will be in the root.
    // Otherwise lookup will be from the last runtime stack scope.
    // We will move up the runtime stack until we either find a 
    // defined variable with a value or run out of parent scopes.
    void setVariable(const EnvKey& name, ValueObj val, bool global = false);

  };

  /////////////////////////////////////////////////////////////////////////
  // EnvScopes are created during evaluation phase. When we enter a parsed
  // scope, e.g. a function, mixin or style-rule, we create a new EnvScope
  // object on the stack and pass it the runtime environment and the current
  // stack frame (in form of a VarRefs pointer). We will "allocate" the needed
  // space for scope items and update any offset pointers. Once we go out of
  // scope the previous state is restored by unwinding the runtime stack.
  /////////////////////////////////////////////////////////////////////////

  class EnvScope {

  private:

    // Runtime environment
    EnvRoot& env;

    // Frame stack index references
    VarRefs* idxs;

    // Remember previous "addresses"
    // Restored when we go out of scope
    uint32_t oldVarFrame;
    uint32_t oldVarOffset;
    uint32_t oldMixFrame;
    uint32_t oldMixOffset;
    uint32_t oldFnFrame;
    uint32_t oldFnOffset;

  public:

    // Put frame onto stack
    EnvScope(
      EnvRoot& env,
      VarRefs* idxs) :
      env(env),
      idxs(idxs),
      oldVarFrame(0),
      oldVarOffset(0),
      oldMixFrame(0),
      oldMixOffset(0),
      oldFnFrame(0),
      oldFnOffset(0)
    {

      // The frame might be fully empty
      // Meaning it no scoped items at all
      if (idxs == nullptr) return;

      // Check if we have scoped variables
      if (idxs->varIdxs.size() != 0) {
        // Get offset into variable vector
        oldVarOffset = (uint32_t)env.variables.size();
        // Remember previous frame "addresses"
        oldVarFrame = env.varFramePtr[idxs->varFrame];
        // Update current frame offset address
        env.varFramePtr[idxs->varFrame] = oldVarOffset;
        // Create space for variables in this frame scope
        env.variables.resize(oldVarOffset + idxs->varIdxs.size());
      }

      // Check if we have scoped mixins
      if (idxs->mixIdxs.size() != 0) {
        // Get offset into mixin vector
        oldMixOffset = (uint32_t)env.mixins.size();
        // Remember previous frame "addresses"
        oldMixFrame = env.mixFramePtr[idxs->mixFrame];
        // Update current frame offset address
        env.mixFramePtr[idxs->mixFrame] = oldMixOffset;
        // Create space for mixins in this frame scope
        env.mixins.resize(oldMixOffset + idxs->mixIdxs.size());
      }

      // Check if we have scoped functions
      if (idxs->fnIdxs.size() != 0) {
        // Get offset into function vector
        oldFnOffset = (uint32_t)env.functions.size();
        // Remember previous frame "addresses"
        oldFnFrame = env.fnFramePtr[idxs->fnFrame];
        // Update current frame offset address
        env.fnFramePtr[idxs->fnFrame] = oldFnOffset;
        // Create space for functions in this frame scope
        env.functions.resize(oldFnOffset + idxs->fnIdxs.size());
      }

      // Push frame onto stack
      // Mostly for dynamic lookups
      env.stack.push_back(idxs);

    }
    // EO ctor

    // Restore old state on destruction
    ~EnvScope()
    {

      // The frame might be fully empty
      // Meaning it no scoped items at all
      if (idxs == nullptr) return;

      // Check if we had scoped variables
      if (idxs->varIdxs.size() != 0) {
        // Truncate variable vector
        env.variables.resize(
          oldVarOffset);
        // Restore old frame address
        env.varFramePtr[idxs->varFrame] =
          oldVarFrame;
      }

      // Check if we had scoped mixins
      if (idxs->mixIdxs.size() != 0) {
        // Truncate existing vector
        env.mixins.resize(
          oldMixOffset);
        // Restore old frame address
        env.mixFramePtr[idxs->mixFrame] =
          oldMixFrame;
      }

      // Check if we had scoped functions
      if (idxs->fnIdxs.size() != 0) {
        // Truncate existing vector
        env.functions.resize(
          oldFnOffset);
        // Restore old frame address
        env.fnFramePtr[idxs->fnFrame] =
          oldFnFrame;
      }

      // Pop frame from stack
      env.stack.pop_back();

    }
    // EO dtor

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

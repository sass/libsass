#ifndef SASS_AST_DEF_MACROS_H
#define SASS_AST_DEF_MACROS_H

#include "memory/allocator.hpp"

#ifndef MAX_NESTING
// Note that this limit is not an exact science
// it depends on various factors, which some are
// not under our control (compile time or even OS
// dependent settings on the available stack size)
// It should fix most common segfault cases though.
#define MAX_NESTING 512
#endif

// Helper class to switch a flag and revert once we go out of scope
template <class T>
class LocalOption final {
  private:
    T* var; // pointer to original variable
    T orig; // copy of the original option
  public:
    LocalOption(T& var, T current) :
      var(&var), orig(var)
    {
      *(this->var) = current;
    }
    ~LocalOption() {
      *(this->var) = this->orig;
    }
};

  // Helper class to put something on a vector
  // and revert once we go out of scope.
  template <class T>
  class LocalStack final {
  private:
    sass::vector<T>& cnt; // container
  public:
    LocalStack(sass::vector<T>& cnt, T push) :
      cnt(cnt)
    {
      cnt.emplace_back(push);
    }
    ~LocalStack() {
      cnt.pop_back();
    }
  };

  // typedef LocalOptions<bool> 

// Macros to help create and maintain local and recursive flag states
#define LOCAL_FLAG(name,opt) LocalOption<bool> flag_##name(name, opt)
#define LOCAL_PTR(var,name,opt) LocalOption<var*> flag_##name(name, opt)
#define LOCAL_SELECTOR(name,opt) LocalStack<SelectorListObj> stack_##name(name, opt)

#define NESTING_GUARD(name) \
  LocalOption<size_t> cnt_##name(name, name + 1); \
  if (name > MAX_NESTING) throw Exception::RecursionLimitError(); \

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#define ADD_REF(type, name) \
protected: \
  type name##_; \
public: \
  type& name() { return name##_; } \
  const type& name() const { return name##_; } \
  void name(type&& name##__) { name##_ = std::move(name##__); } \
  void name(const type& name##__) { name##_ = name##__; } \
private:

#define ADD_CONSTREF(type, name)\
protected:\
  type name##_;\
public:\
  const type& name() const { return name##_; } \
  void name(type&& name##__) { name##_ = std::move(name##__); } \
  void name(const type& name##__) { name##_ = name##__; } \
private:

#define ADD_PROPERTY(type, name)\
protected:\
  type name##_;\
public:\
  type name() const { return name##_; }\
  void name(type name##__) { name##_ = name##__; }\
private:

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  #ifdef DEBUG_SHARED_PTR

    #define SASS_MEMORY_PARAMS file, line,
    #define SASS_MEMORY_PARAMS_VOID file, line
    #define SASS_MEMORY_POS __FILE__, __LINE__
    #define SASS_MEMORY_POS_VOID __FILE__, __LINE__
    #define SASS_MEMORY_ARGS sass::string file, size_t line,
    #define SASS_MEMORY_ARGS_VOID sass::string file, size_t line

  #else

    #define SASS_MEMORY_PARAMS
    #define SASS_MEMORY_PARAMS_VOID
    #define SASS_MEMORY_POS
    #define SASS_MEMORY_POS_VOID
    #define SASS_MEMORY_ARGS
    #define SASS_MEMORY_ARGS_VOID

  #endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Inverting these functions is a bit tricky as there are consequences
  // Easiest would be to just switch out the operand, but that would mean
  // that the right hand side would determine the implementation. It would
  // also invert the arguments given, e.g. for error reporting. Another way
  // is to make both comparisons in order to keep left and right arguments.
  // #ifdef SASS_OPTIMIZE_CMP_OPS
  // bool operator>(const AstNode& rhs) const { return rhs < *this; }
  // bool operator<=(const AstNode& rhs) const { return !(rhs < *this); }
  // #else
  // bool operator>(const AstNode& rhs) const { return !(*this < rhs || *this == rhs); }
  // bool operator<=(const AstNode& rhs) const { return *this < rhs || *this == rhs; }
  // #endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  #define DECLARE_ISA_CASTER(klass) \
    public: virtual klass* isa##klass() { return nullptr; } \
    public: virtual const klass* isa##klass() const { return nullptr; } \

  #define IMPLEMENT_ISA_CASTER(klass) \
    public: klass* isa##klass() final override { return this; } \
    public: const klass* isa##klass() const final override { return this; } \

  #define IMPLEMENT_ACCEPT(type, visitor, klass) \
    public: type accept(visitor##Visitor<type>* visitor) override final { \
      return visitor->visit##klass(this); \
    } \

  #define IMPLEMENT_EQ_OPERATOR(subklass, klass) \
    public: bool operator==(const subklass& rhs) const override final { \
      auto sel = rhs.isa##klass(); \
      return sel ? *this == *sel : false; \
    } \
    public: bool operator==(const klass& rhs) const; \

  // Childless argument is passed to ctor
  #define IMPLEMENT_SEL_COPY_CHILDREN(klass) \
    public: klass* copy(SASS_MEMORY_ARGS bool childless) const override final { \
      return SASS_MEMORY_NEW_DBG(klass, this, childless); \
    } \

  // Childless argument is ignored on ctor
  #define IMPLEMENT_SEL_COPY_IGNORE(klass) \
    public: klass* copy(SASS_MEMORY_ARGS bool childless) const override final { \
      return SASS_MEMORY_NEW_DBG(klass, this); \
    } \

  /////////////////////////////////////////////////////////////////////////
  /* Wrap c++ pointers for C-API to anon-structs */
  /////////////////////////////////////////////////////////////////////////
  #define CAPI_WRAPPER(klass, strukt) \
  struct strukt* wrap() \
  { \
    /* This is a compile time cast and doesn't cost anything */ \
    return reinterpret_cast<struct strukt*>(this); \
  }; \
  /* Wrap the pointer for C-API */ \
  const struct strukt* wrap() const \
  { \
    /* This is a compile time cast and doesn't cost anything */ \
    return reinterpret_cast<const struct strukt*>(this); \
  }; \
  /* Wrap the pointer for C-API */ \
  static struct strukt* wrap(klass* unwrapped) \
  { \
    /* Ensure we at least catch the most obvious stuff */ \
    if (unwrapped == nullptr) throw std::runtime_error( \
      "Null-Pointer passed to " #klass "::unwrap"); \
    /* Just delegate to wrap */ \
    return unwrapped->wrap(); \
  }; \
  /* Wrap the pointer for C-API */ \
  static const struct strukt* wrap(const klass* unwrapped) \
  { \
    /* Ensure we at least catch the most obvious stuff */ \
    if (unwrapped == nullptr) throw std::runtime_error( \
      "Null-Pointer passed to " #klass "::unwrap"); \
    /* Just delegate to wrap */ \
    return unwrapped->wrap(); \
  }; \
  /* Unwrap the pointer for C-API (potentially unsafe). */ \
  /* You must pass in a pointer you've got via wrap API. */ \
  /* Passing anything else will result in undefined behavior! */ \
  static klass& unwrap(struct strukt* wrapped) \
  { \
    /* Ensure we at least catch the most obvious stuff */ \
    if (wrapped == nullptr) throw std::runtime_error( \
      "Null-Pointer passed to " #klass "::unwrap"); \
    /* This is a compile time cast and doesn't cost anything */ \
    return *reinterpret_cast<klass*>(wrapped); \
  }; \
  /* Unwrap the pointer for C-API (potentially unsafe). */ \
  /* You must pass in a pointer you've got via wrap API. */ \
  /* Passing anything else will result in undefined behavior! */ \
  static const klass& unwrap(const struct strukt* wrapped) \
  { \
    /* Ensure we at least catch the most obvious stuff */ \
    if (wrapped == nullptr) throw std::runtime_error( \
      "Null-Pointer passed to " #klass "::unwrap"); \
    /* This is a compile time cast and doesn't cost anything */ \
    return *reinterpret_cast<const klass*>(wrapped); \
  }; \


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif

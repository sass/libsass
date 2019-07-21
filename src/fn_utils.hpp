#ifndef SASS_FN_UTILS_H
#define SASS_FN_UTILS_H

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include "units.hpp"
#include "backtrace.hpp"
#include "environment.hpp"
#include "ast_fwd_decl.hpp"
#include "error_handling.hpp"

namespace Sass {

  #define FN_PROTOTYPE \
    Env& env, \
    Env& d_env, \
    Context& ctx, \
    Signature sig, \
    ParserState pstate, \
    Backtraces& traces, \
    SelectorStack selector_stack, \
    SelectorStack original_stack \

  typedef const char* Signature;
  typedef PreValue* (*Native_Function)(FN_PROTOTYPE);
  #define BUILT_IN(name) PreValue* name(FN_PROTOTYPE)

  #define ARG(argname, argtype, type) get_arg<argtype>(argname, env, sig, pstate, traces, type)
  #define ARGCOL(argname) get_arg<Color>(argname, env, sig, pstate, traces, "a color")
  #define ARGNUM(argname) get_arg<Number>(argname, env, sig, pstate, traces, "a number")
  #define ARGLIST(argname) get_arg<List>(argname, env, sig, pstate, traces, "a list")
  #define ARGSTRC(argname) get_arg<String_Constant>(argname, env, sig, pstate, traces, "a string")
  // special function for weird hsla percent (10px == 10% == 10 != 0.1)
  #define ARGVAL(argname) get_arg_val(argname, env, sig, pstate, traces) // double

  Definition* make_native_function(Signature, Native_Function, Context& ctx);
  Definition* make_c_function(Sass_Function_Entry c_func, Context& ctx);

  namespace Functions {

    std::string envValueToString(Env& env, const std::string& name);

    template <typename T>
    T* get_arg(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces, std::string type)
    {
      AST_Node* node = env.get_local(argname);
      T* val = Cast<T>(node);
      if (node && !val) {
        error(argname + ": " + envValueToString(env, argname) + " is not " + type + ".", pstate, traces);
      }
      return val;
    }

    Map* get_arg_m(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces); // maps only
    Number* get_arg_n(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces); // numbers only
    double alpha_num(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces); // colors only
    double color_num(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces); // colors only
    double get_arg_r(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces, double lo, double hi, std::string unit = ""); // colors only
    double get_arg_val(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces); // shared
    SelectorListObj get_arg_sels(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces, Context& ctx); // selectors only
    CompoundSelectorObj get_arg_sel(const std::string& argname, Env& env, Signature sig, ParserState pstate, Backtraces traces, Context& ctx); // selectors only

    int assertInt(const std::string& name, double nr, ParserState pstate, Backtraces traces);

  }

}

#endif

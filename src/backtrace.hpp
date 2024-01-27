#ifndef SASS_BACKTRACE_H
#define SASS_BACKTRACE_H

#include "strings.hpp"
#include "source_span.hpp"
#include "ast_def_macros.hpp"

// During runtime we need stack traces in order to produce meaningful
// error messages. Since the error catching might be done outside of
// the main compile function, certain values might already be garbage
// collected. Therefore we need to carry copies of those in any error.
// In order to optimize runtime, we don't want to create these copies
// during the evaluation stage, as most of the time we would throw them
// out right away. Therefore we only keep references during that phase
// (BackTrace), and copy them once an actual error is thrown (StackTrace).

namespace Sass {

  class Traced {
  public:
    CAPI_WRAPPER(Traced, SassTrace);
    virtual const SourceSpan& getPstate() const = 0;
    virtual const sass::string& getName() const = 0;
    virtual bool isFn() const = 0;
    virtual ~Traced() {};
  };

  // Holding actual copies
  class StackTrace : public Traced {

  public:

    SourceSpan pstate;
    sass::string name;
    bool fn;

    StackTrace(
      SourceSpan pstate,
      sass::string name = Strings::empty,
      bool fn = false) :
      pstate(pstate),
      name(name),
      fn(fn)
    {}

    const SourceSpan& getPstate() const override final {
      return pstate;
    }

    const sass::string& getName() const override final {
      return name;
    }

    bool operator==(const StackTrace& other) const {
      return pstate == other.pstate &&
        name == other.name && fn == other.fn;
    }

    bool isFn() const override final {
      return fn;
    }

  };

  // Holding only references
  class BackTrace : public Traced {

  public:

    const SourceSpan& pstate;
    const sass::string& name;
    bool fn;

    BackTrace(
      const SourceSpan& pstate,
      const sass::string& name = Strings::empty,
      bool fn = false) :
      pstate(pstate),
      name(name),
      fn(fn)
    {}

    const SourceSpan& getPstate() const override final {
      return pstate;
    }

    const sass::string& getName() const override final {
      return name;
    }

    bool isFn() const override final {
      return fn;
    }

    // Create copies on convert
    operator StackTrace()
    {
      return StackTrace(
        pstate, name, fn);
    }

  };

  // Some related and often used aliases
  typedef sass::vector<Traced> Traces;
  typedef sass::vector<BackTrace> BackTraces;
  typedef sass::vector<StackTrace> StackTraces;

}

#endif

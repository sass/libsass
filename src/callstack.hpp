/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CALLSTACK_HPP
#define SASS_CALLSTACK_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "backtrace.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Utility class to add a frame onto a call-stack (RAII).
  // Cleans up automatically when object goes out of scope.
  // We assume this happens in well defined order, as
  // we do not check if we actually remove ourself!
  // ToDo: rename to callTrace
  class callStackFrame {

  private:

    // The shared callStack
    BackTraces& backTraces;

    // The current stack frame
    BackTrace frame;

    // Are we invoked by `call`
    bool viaCall;

  public:

    // Create object and add frame to stack
    callStackFrame(BackTraces& backTraces,
      const BackTrace& frame,
      bool viaCall = false) :
      backTraces(backTraces),
      frame(frame),
      viaCall(viaCall)
    {
      // Append frame to stack
      if (!viaCall) backTraces.push_back(frame);
    }

    // Remove frame from stack on destruction
    ~callStackFrame()
    {
      // Pop frame from stack
      if (!viaCall) backTraces.pop_back();
    }

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

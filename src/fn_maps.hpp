/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_FN_MAPS_HPP
#define SASS_FN_MAPS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "fn_utils.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Maps {

      // Register all functions at compiler
      void registerFunctions(Compiler& ctx);

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}

#endif

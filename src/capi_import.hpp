/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_IMPORT_HPP
#define SASS_CAPI_IMPORT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <sass/import.h>

// Some structures are simple c++ vectors/queues.
// There might be a more efficient way to achieve this?
// Although compiler optimization should see this case easily!
struct SassImportList : std::deque<struct SassImport*> {};

#endif

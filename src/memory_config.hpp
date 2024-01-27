/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_MEMORY_CONFIG_H
#define SASS_MEMORY_CONFIG_H

#include "settings.hpp"

/////////////////////////////////////////////////////////////////////////
// Memory allocator configurations
/////////////////////////////////////////////////////////////////////////

// Define memory alignment requirements
#define SASS_MEM_ALIGN sizeof(unsigned int)

// The number of bytes we use for our book-keeping before every
// memory fragment. Needed to know to which bucket we belongs on
// deallocations, or if it should go directly to the `free` call.
#define SassAllocatorBookSize sizeof(unsigned int)

// Bytes reserve for book-keeping on the arenas
// Currently unused and for later optimization
#define SassAllocatorArenaHeadSize 0

/////////////////////////////////////////////////////////////////////////
// Below settings should only be changed if you know what you do!
/////////////////////////////////////////////////////////////////////////

// Detail settings for pool allocator
#ifdef SASS_CUSTOM_ALLOCATOR

  // How many buckets should we have for the free-list
  // We have a bucket for every `SASS_MEM_ALIGN` * `SassAllocatorBuckets`
  // When something requests x amount of memory, we will pad the request
  // to be a multiple of `SASS_MEM_ALIGN` and then assign it either to
  // an existing bucket or directly use to malloc/free. Otherwise we will
  // chunk out a slice of the arena to store it in that memory.
  #define SassAllocatorBuckets 960

  // The size of the memory pool arenas in bytes.
  // This determines the minimum allocated memory chunk.
  // Whenever we need more memory, we malloc that much.
  #define SassAllocatorArenaSize (1024 * 1024)

#endif
// EO SASS_CUSTOM_ALLOCATOR

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#ifdef DEBUG_SHARED_PTR
#define DEBUG_MSVC_CRT_MEM
#endif
#endif

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif

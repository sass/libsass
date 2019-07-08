/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_MEMORY_CONFIG_H
#define SASS_MEMORY_CONFIG_H

#include "../settings.hpp"

/////////////////////////////////////////////////////////////////////////
// Memory allocator configurations
/////////////////////////////////////////////////////////////////////////

// Define memory alignment requirements
#define SASS_MEM_ALIGN sizeof(unsigned int)

// Minimal alignment for memory fragments. Must be a multiple
// of `SASS_MEM_ALIGN` and should not be too big (maybe 1 or 2)
#define SassAllocatorHeadSize sizeof(unsigned int)

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
  // Determines when allocations go directly to malloc/free
  // For maximum size of managed items multiply by alignment
  #define SassAllocatorBuckets 640

  // The size of the memory pool arenas in bytes.
  #define SassAllocatorArenaSize (1024 * 640)

#endif
// EO SASS_CUSTOM_ALLOCATOR

#endif

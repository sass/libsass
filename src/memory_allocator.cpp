/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "memory_allocator.hpp"

#ifdef SASS_CUSTOM_ALLOCATOR
#include "memory_pool.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // You must only use PODs for thread_local.
  // Objects get very unpredictable init order.
  static thread_local MemoryPool* pool;
  static thread_local size_t allocations;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Allocate memory from the memory pool.
  // Memory pool is allocated on first call.
  void* allocateMem(size_t size)
  {
    if (pool == nullptr) {
      pool = new MemoryPool();
    }
    ++allocations;
    return pool->allocate(size);
  }

  // Release the memory from the pool.
  // Destroys the pool when it is emptied.
  void deallocateMem(void* ptr, size_t size)
  {

    // It seems thread_local variable might be discharged!?
    // But the destructors of e.g. static strings is still
    // called, although their memory was discharged too.
    // Fine with me as long as address sanitizer is happy.
    if (pool == nullptr || allocations == 0) { return; }

    pool->deallocate(ptr);
    if (--allocations == 0) {
      delete pool;
      pool = nullptr;
    }

  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

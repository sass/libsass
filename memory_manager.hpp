#ifndef SASS_MEMORY_MANAGER_H
#define SASS_MEMORY_MANAGER_H

#include <vector>
#include <memory>
#include <typeinfo>
#include <iostream>
#include <stdio.h>
#include <string.h>

namespace Sass {
  using namespace std;
  /////////////////////////////////////////////////////////////////////////////
  // A class for tracking allocations of AST_Node objects. The intended usage
  // is something like: Some_Node* n = new (mem_mgr) Some_Node(...);
  // Then, at the end of the program, the memory manager will delete all of the
  // allocated nodes that have been passed to it.
  // In the future, this class may implement a custom allocator.
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class Memory_Manager {
    vector<T*> nodes;

  public:
    Memory_Manager(size_t size = 0);
    ~Memory_Manager();

    bool has(T* np);
    void remove(T* np);
    void destroy(T* np);
    T* operator()(T* np);

  };
}

template <typename T>
inline void* operator new(size_t size, Sass::Memory_Manager<T>& mem)
{
  T* val = static_cast<T*>(operator new(size));
  memset( (void*) val, 0, size );
  return mem(val);
}

template <typename T>
inline void operator delete(void *np, Sass::Memory_Manager<T>& mem)
{
  mem.remove(reinterpret_cast<T*>(np));
  operator delete(np);
}

#endif

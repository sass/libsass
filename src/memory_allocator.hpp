/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ALLOCATOR_HPP
#define SASS_ALLOCATOR_HPP

#include "memory_config.hpp"
#include "settings.hpp"
#include "MurmurHash2.hpp"
#include "randomize.hpp"

#include <deque>
#include <vector>
#include <limits>
#include <iostream>
#include <algorithm>
#include <functional>

namespace Sass {

  // Fallback to standard allocator
  #ifndef SASS_CUSTOM_ALLOCATOR
  template <typename T> using Allocator = std::allocator<T>;
  #else

  // Allocate memory from the memory pool.
  // Memory pool is allocated on first call.
  void* allocateMem(size_t size);

  // Release the memory from the pool.
  // Destroys the pool when it is emptied.
  void deallocateMem(void* ptr, size_t size = 1);

  template<typename T>
  class Allocator
  {
  public:

    // Allocator traits
    typedef T                 type;            
    typedef type              value_type;      
    typedef value_type*       pointer;         
    typedef value_type const* const_pointer;   
    typedef value_type&       reference;       
    typedef value_type const& const_reference; 
    typedef std::size_t       size_type;       
    typedef std::ptrdiff_t    difference_type; 

    // Not really sure what this does!?
    template<typename U>
    struct rebind
    {
      typedef Allocator<U> other;
    };

    // Default constructor
    Allocator(void) {}

    // Copy constructor
    template<typename U>
    Allocator(Allocator<U> const&)
    {}

    // allocate but don't initialize count of elements of type T
    pointer allocate(size_type count, const_pointer /* hint */ = 0)
    {
      return (pointer)(Sass::allocateMem(count * sizeof(T)));
    }

    // deallocate storage ptr of deleted elements
    void deallocate(pointer ptr, size_type count)
    {
      Sass::deallocateMem(ptr, count);
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw()
    {
      return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // Address of object
    type* address(type& obj) const { return &obj; }
    type const* address(type const& obj) const { return &obj; }

    // Construct object
    void construct(type* ptr, type const& ref) const
    {
      // In-place copy construct
      new(ptr) type(ref);
    }

    // Destroy object
    void destroy(type* ptr) const
    {
      // Call destructor
      ptr->~type();
    }

  };

  // Allocators are equal, don't care for type!
  template<typename T, typename U>
    bool operator==(Allocator<T> const& left,
      Allocator<U> const& right)
  {
    return true;
  }

  // Allocators are equal, don't care for type!
  template<typename T, typename U>
    bool operator!=(Allocator<T> const& left,
      Allocator<U> const& right)
  {
    return !(left == right);
  }

  // EO custom allocator
  #endif

}

// Make them available on the global scope
// Easier for global structs needed for C linkage
namespace sass { // Note the lower-case notation
#ifndef SASS_CUSTOM_ALLOCATOR
  template <typename T> using deque = std::deque<T>;
  template <typename T> using vector = std::vector<T>;
  using string = std::string;
  using wstring = std::wstring;
  using sstream = std::stringstream;
  using ostream = std::ostringstream;
  using istream = std::istringstream;
#else
  template <typename T> using deque = std::deque<T, Sass::Allocator<T>>;
  template <typename T> using vector = std::vector<T, Sass::Allocator<T>>;
  using string = std::basic_string<char, std::char_traits<char>, Sass::Allocator<char>>;
  using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, Sass::Allocator<wchar_t>>;
  using sstream = std::basic_stringstream<char, std::char_traits<char>, Sass::Allocator<char>>;
  using ostream = std::basic_ostringstream<char, std::char_traits<char>, Sass::Allocator<char>>;
  using istream = std::basic_istringstream<char, std::char_traits<char>, Sass::Allocator<char>>;
#endif
}

#ifdef SASS_CUSTOM_ALLOCATOR

namespace std {
  // Only GCC seems to need this specialization!?
  template <> struct hash<sass::string> {
  public:
    inline size_t operator()(
      const sass::string& name) const
    {
      return MurmurHash2(
        (void*)name.c_str(),
        (int)name.size(),
        Sass::getHashSeed());
    }
  };
}

#endif

#endif

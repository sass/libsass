/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_HASHING_HPP
#define SASS_HASHING_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "randomize.hpp"

//////////////////////////////////////////////////////////
// `hash_combine` comes from boost (functional/hash):
// http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html
// Boost Software License - Version 1.0
// http://www.boost.org/users/license.html
//////////////////////////////////////////////////////////

namespace Sass {

  template <typename T>
  inline void hash_combine(std::size_t& hash, const T& val)
  {
    hash ^= std::hash<T>()(val) + getHashSeed()
      + (hash << 6) + (hash >> 2);
  }
  // EO hash_combine

  template <typename T>
  inline void hash_start(std::size_t& hash, const T& val)
  {
    hash = std::hash<T>()(val) + getHashSeed();
  }
  // EO hash_start

  // Not sure if calling std::hash<size_t> has any overhead!?
  inline void hash_combine(std::size_t& hash, std::size_t val)
  {
    hash ^= val + getHashSeed()
      + (hash << 6) + (hash >> 2);
  }
  // EO hash_combine

  // Not sure if calling std::hash<size_t> has any overhead!?
  inline void hash_start(std::size_t& hash, std::size_t val)
  {
    hash = val + getHashSeed();
  }
  // EO hash_start

}

#endif

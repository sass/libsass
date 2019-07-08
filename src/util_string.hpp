#ifndef SASS_UTIL_STRING_HPP
#define SASS_UTIL_STRING_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "character.hpp"
#include <algorithm>
#include <vector>
#include <bitset>
#include <iterator>
#include <sstream>
#include <iostream>

namespace Sass {

  //////////////////////////////////////////////////////////
  // `hash_combine` comes from boost (functional/hash):
  // http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html
  // Boost Software License - Version 1.0
  // http://www.boost.org/users/license.html
  template <typename T>
  void hash_combine(std::size_t& hash, const T& val)
  {
    hash ^= std::hash<T>()(val) + getHashSeed()
      + (hash << 6) + (hash >> 2);
  }
  //////////////////////////////////////////////////////////

  template <typename T>
  // Micro-optimization to avoid one call to `hash_combine`
  void hash_start(std::size_t& hash, const T& val)
  {
    hash = std::hash<T>()(val);
  }

  // Not sure if calling std::hash<size_t> has any overhead!?
  inline void hash_combine(std::size_t& hash, std::size_t val)
  {
    hash ^= val + getHashSeed()
      + (hash << 6) + (hash >> 2);
  }

  // Not sure if calling std::hash<size_t> has any overhead!?
  inline void hash_start(std::size_t& hash, std::size_t val)
  {
    hash = val;
  }

  namespace Util {

    /////////////////////////////////////////////////////////////////////////#
    // Returns [name] without a vendor prefix.
    // If [name] has no vendor prefix, it's returned as-is.
    /////////////////////////////////////////////////////////////////////////#
    sass::string unvendor(const sass::string& name);

  }
  // EO namespace Sass

}
// EO namespace Util

#endif  // SASS_UTIL_STRING_H

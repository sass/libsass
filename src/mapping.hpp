/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_MAPPING_H
#define SASS_MAPPING_H

#include <vector>
#include "memory.hpp"
#include "offset.hpp"

namespace Sass {

  class Mapping {

  public:
    // Source Index for the origin
    size_t srcidx; // uint32_t
    // Position of original occurrence
    Offset origin;
    // Position where it was rendered.
    Offset target;

    // Base copy constructor
    Mapping(size_t srcidx, const Offset& origin, const Offset& target)
    : srcidx(srcidx), origin(origin), target(target) { }

  };

  typedef sass::vector<Mapping> Mappings;

}

#endif

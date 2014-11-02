#define SASS_MAPPING

#ifndef SASS_POSITION
#include "position.hpp"
#endif

namespace Sass {

  struct Mapping {
    Position original_position;
    Position generated_position;
    size_t type;

    Mapping(const Position& original_position, const Position& generated_position, size_t type = 6)
    : original_position(original_position), generated_position(generated_position), type(type) { }
  };

}

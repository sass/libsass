#include "../sass.hpp"
#include "values.hpp"
#include "../ast.hpp"
#include "../context.hpp"
#include "../node.hpp"
#include "../extend.hpp"
#include "../emitter.hpp"
#include "../color_maps.hpp"
#include <set>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace Sass {

  bool Ruleset::is_invisible() const {
    Selector_List* sl = static_cast<Selector_List*>(selector());
    for (size_t i = 0, L = sl->length(); i < L; ++i)
      if (!(*sl)[i]->has_placeholder()) return false;
    return true;
  }

  bool Media_Block::is_invisible() const {
    for (size_t i = 0, L = block()->length(); i < L; ++i) {
      if (!(*block())[i]->is_invisible()) return false;
    }
    return true;
  }

}
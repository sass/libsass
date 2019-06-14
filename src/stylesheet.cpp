#include "stylesheet.hpp"

namespace Sass {

  // Constructor
  Sass::StyleSheet::StyleSheet(const Resource& res, Block_Obj root) :
    Resource(res),
    root(root)
  {
  }

  StyleSheet::StyleSheet(const StyleSheet& sheet) :
    Resource(sheet),
    root(sheet.root)
  {
  }

}

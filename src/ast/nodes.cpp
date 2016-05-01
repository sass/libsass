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

  void AST_Node::update_pstate(const ParserState& pstate)
  {
    pstate_.offset += pstate - pstate_ + pstate.offset;
  }

  std::string AST_Node::to_string(Sass_Inspect_Options opt) const
  {
    Sass_Output_Options out(opt);
    Emitter emitter(out);
    Inspect i(emitter);
    i.in_declaration = true;
    // ToDo: inspect should be const
    const_cast<AST_Node*>(this)->perform(&i);
    return i.get_buffer();
  }

  std::string AST_Node::to_string() const
  {
    return to_string({ NESTED, 5 });
  }

}
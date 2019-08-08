#include "../sass.hpp"
#include <iostream>
#include <typeinfo>

#include "SharedPtr.hpp"
#include "../ast_fwd_decl.hpp"

#ifdef DEBUG_SHARED_PTR
#include "../debugger.hpp"
#endif

namespace Sass {

  #ifdef DEBUG_SHARED_PTR
  void SharedObj::dumpMemLeaks() {
    if (!all.empty()) {
      std::clog << "###################################" << std::endl;
      std::clog << "# REPORTING MISSING DEALLOCATIONS #" << std::endl;
      std::clog << "###################################" << std::endl;
      for (SharedObj* var : all) {
        if (AST_Node* ast = dynamic_cast<AST_Node*>(var)) {
          debug_ast(ast);
        } else {
          std::clog << "LEAKED " << var << std::endl;
        }
      }
    }
  }
  std::vector<SharedObj*> SharedObj::all;
  #endif

  bool SharedObj::taint = false;
}

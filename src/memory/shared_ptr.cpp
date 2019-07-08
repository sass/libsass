#include "shared_ptr.hpp"

#ifdef DEBUG_SHARED_PTR
#include "../debugger.hpp"
#include <iostream>
#include <typeinfo>
#endif

#include "../source.hpp"

namespace Sass {

  #ifdef DEBUG_SHARED_PTR
  void SharedObj::dumpMemLeaks() {
    if (!all.empty()) {
      std::cerr << "###################################\n";
      std::cerr << "# REPORTING MISSING DEALLOCATIONS #\n";
      std::cerr << "###################################\n";
      for (SharedObj* var : all) {
        if (AstNode* ast = dynamic_cast<AstNode*>(var)) {
          std::cerr << "LEAKED AST " << ast->getDbgFile() << ":" << ast->getDbgLine() << "\n";
          debug_ast(ast);
        }
        else if (SourceData* ast = dynamic_cast<SourceData*>(var)) {
          std::cerr << "LEAKED SOURCE " << ast->getDbgFile() << ":" << ast->getDbgLine() << "\n";
        }
        else {
          std::cerr << "LEAKED " << var << "\n";
        }
      }
      all.clear();
      deleted.clear();
      objCount = 0;
    }
  }
  size_t SharedObj::objCount = 0;
  sass::vector<SharedObj*> SharedObj::all;
  std::unordered_set<size_t> SharedObj::deleted;
  size_t SharedObj::maxRefCount = 0;
#endif

  bool SharedObj::taint = false;
  // size_t SharedObj::moves = 0;
  // size_t SharedObj::copies = 0;
}

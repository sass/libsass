#include "shared_ptr.hpp"

#ifdef DEBUG_SHARED_PTR
#include "debugger.hpp"
#include <iostream>
#include <typeinfo>
#endif

#include "source.hpp"

namespace Sass {

  #ifdef DEBUG_SHARED_PTR
  void RefCounted::dumpMemLeaks() {
    if (!all.empty()) {
      std::cerr << "###################################\n";
      std::cerr << "# REPORTING MISSING DEALLOCATIONS #\n";
      std::cerr << "###################################\n";
      for (RefCounted* var : all) {
        if (AstNode* ast = dynamic_cast<AstNode*>(var)) {
          std::cerr << "LEAKED AST " << ast->getDbgFile() << ":" << ast->getDbgLine() << "\n";
          debug_ast(ast);
        }
        else if (SourceData* ast = dynamic_cast<SourceData*>(var)) {
          std::cerr << "LEAKED SOURCE " << ast->getDbgFile() << ":" << ast->getDbgLine() << "[" << ast->getAbsPath() << "]\n";
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
  size_t RefCounted::objCount = 0;
  sass::vector<RefCounted*> RefCounted::all;
  std::unordered_set<size_t> RefCounted::deleted;
  size_t RefCounted::maxRefCount = 0;
#endif

  bool RefCounted::taint = false;
  // size_t RefCounted::moves = 0;
  // size_t RefCounted::copies = 0;
}

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "preloader.hpp"

#include "eval.hpp"
#include "compiler.hpp"
#include "exceptions.hpp"
#include "environment.hpp"
#include "ast_imports.hpp"

namespace Sass {

  Preloader::Preloader(Eval& eval, Root* root) :
    eval(eval),
    root(root),
    compiler(eval.compiler),
    modctx(eval.modctx42),
    wconfig(eval.wconfig),
    idxs(root->idxs)
  {}

  // During the whole parsing we should keep a big map of
  // Variable name to EnvRef vector with all alternatives

  void Preloader::process()
  {
    acceptRoot(root);
  }

  void Preloader::acceptRoot(Root* sheet)
  {
    if (sheet && !sheet->empty()) {
      RAII_MODULE(modules, root);
      RAII_PTR(Root, modctx, sheet);
      RAII_PTR(EnvRefs, idxs, sheet->idxs);
      ImportStackFrame isf(compiler, sheet->import);
      compiler.varRoot.stack.push_back(sheet->idxs);
      for (auto& it : sheet->elements()) it->accept(this);
      compiler.varRoot.stack.pop_back();
    }
  }

  void Preloader::visitParentStatement(ParentStatement* rule)
  {
    if (rule->empty()) return;
    RAII_PTR(EnvRefs, idxs, rule->idxs);
    compiler.varRoot.stack.push_back(rule->idxs);
    for (auto& it : rule->elements()) it->accept(this);
    compiler.varRoot.stack.pop_back();
  }

  void Preloader::visitUseRule(UseRule* rule)
  {
    callStackFrame frame(compiler, {
      rule->pstate(), Strings::useRule });
    acceptRoot(eval.loadModRule(rule));
    eval.exposeUseRule(rule);
  }

  void Preloader::visitForwardRule(ForwardRule* rule)
  {
    callStackFrame frame(compiler, {
      rule->pstate(), Strings::forwardRule });
    acceptRoot(eval.loadModRule(rule));
    eval.exposeFwdRule(rule);
  }

  void Preloader::visitIncludeImport(IncludeImport* rule)
  {
    callStackFrame frame(compiler, {
      rule->pstate(), Strings::importRule });
    // We could demux glob-stars here
    acceptRoot(eval.resolveIncludeImport(rule));
    eval.exposeImpRule(rule);
  }

  void Preloader::visitAssignRule(AssignRule* rule)
  {
    if (!rule->ns().empty()) return;
  }

  void Preloader::visitFunctionRule(FunctionRule* rule)
  {
    RAII_PTR(EnvRefs, idxs, rule->idxs);
    compiler.varRoot.stack.push_back(rule->idxs);
    for (auto& it : rule->elements()) it->accept(this);
    compiler.varRoot.stack.pop_back();
  }

  void Preloader::visitMixinRule(MixinRule* rule)
  {
    RAII_PTR(EnvRefs, idxs, rule->idxs);
    compiler.varRoot.stack.push_back(rule->idxs);
    for (auto& it : rule->elements()) it->accept(this);
    compiler.varRoot.stack.pop_back();
  }

  void Preloader::visitImportRule(ImportRule* rule)
  {
    for (const ImportBaseObj& import : rule->elements()) {
      if (IncludeImport* include = import->isaIncludeImport()) {
        visitIncludeImport(include);
      }
    }
  }

  void Preloader::visitAtRootRule(AtRootRule* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitAtRule(AtRule* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitContentBlock(ContentBlock* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitContentRule(ContentRule* rule)
  {
  }

  void Preloader::visitDebugRule(DebugRule* rule)
  {
  }

  void Preloader::visitDeclaration(Declaration* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitErrorRule(ErrorRule* rule)
  {
  }

  void Preloader::visitExtendRule(ExtendRule* rule)
  {
  }

  void Preloader::visitIfRule(IfRule* rule)
  {
    visitParentStatement(rule);
    if (rule->alternative() == nullptr) return;
    visitIfRule(rule->alternative());
  }


  void Preloader::visitIncludeRule(IncludeRule* rule)
  {
    if (ContentBlock* content = rule->content()) {
      RAII_PTR(EnvRefs, idxs, content->idxs);
      compiler.varRoot.stack.push_back(content->idxs);
      for (auto& it : content->elements()) it->accept(this);
      compiler.varRoot.stack.pop_back();
    }
  }

  void Preloader::visitLoudComment(LoudComment* rule)
  {
  }

  void Preloader::visitMediaRule(MediaRule* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitEachRule(EachRule* rule)
  {
    RAII_PTR(EnvRefs, idxs, rule->idxs);
    auto& vars(rule->variables());
    for (size_t i = 0; i < vars.size(); i += 1) {
      idxs->varIdxs.insert({ vars[i], (uint32_t)i });
    }
    compiler.varRoot.stack.push_back(rule->idxs);
    for (auto& it : rule->elements()) it->accept(this);
    compiler.varRoot.stack.pop_back();
  }

  void Preloader::visitForRule(ForRule* rule)
  {
    RAII_PTR(EnvRefs, idxs, rule->idxs);
    idxs->varIdxs.insert({ rule->varname(), 0 });
    compiler.varRoot.stack.push_back(rule->idxs);
    for (auto& it : rule->elements()) it->accept(this);
    compiler.varRoot.stack.pop_back();
  }

  void Preloader::visitReturnRule(ReturnRule* rule)
  {
  }

  void Preloader::visitSilentComment(SilentComment* rule)
  {
  }

  void Preloader::visitStyleRule(StyleRule* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitSupportsRule(SupportsRule* rule)
  {
    visitParentStatement(rule);
  }

  void Preloader::visitWarnRule(WarnRule* rule)
  {
  }

  void Preloader::visitWhileRule(WhileRule* rule)
  {
    visitParentStatement(rule);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

};

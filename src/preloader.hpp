/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PRELOADER_HPP
#define SASS_PRELOADER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "environment_key.hpp"
#include "environment_cnt.hpp"
#include "visitor_statement.hpp"

namespace Sass {

  /*#####################################################################*/
  /*#####################################################################*/

  class Preloader :
    public StatementVisitor<void>
  {

  public:

    Eval& eval;
    Root* root;
    Compiler& compiler;

    // Alias into context
    Root*& modctx;

    sass::vector<Root*> modules;

    // Alias into context
    WithConfig*& wconfig;

    // Current lexical scope
    EnvRefs* idxs;

    void visitParentStatement(ParentStatement* rule);
    void visitIncludeImport(IncludeImport* include);

  public:

    Preloader(
      Eval& eval,
      Root* root);

    void process();

    void acceptRoot(Root* root);


    void visitAtRootRule(AtRootRule* rule) override final;
    void visitAtRule(AtRule* rule) override final;
    void visitContentBlock(ContentBlock* rule) override final;
    void visitContentRule(ContentRule* rule) override final;
    void visitDebugRule(DebugRule* rule) override final;
    void visitDeclaration(Declaration* rule) override final;
    void visitEachRule(EachRule* rule) override final;
    void visitErrorRule(ErrorRule* rule) override final;
    void visitExtendRule(ExtendRule* rule) override final;
    void visitForRule(ForRule* rule) override final;
    void visitForwardRule(ForwardRule* rule) override final;
    void visitFunctionRule(FunctionRule* rule) override final;
    void visitIfRule(IfRule* rule) override final;
    void visitImportRule(ImportRule* rule) override final;
    void visitIncludeRule(IncludeRule* rule) override final;
    void visitLoudComment(LoudComment* rule) override final;
    void visitMediaRule(MediaRule* rule) override final;
    void visitMixinRule(MixinRule* rule) override final;
    void visitReturnRule(ReturnRule* rule) override final;
    void visitSilentComment(SilentComment* rule) override final;
    void visitStyleRule(StyleRule* rule) override final;
    void visitSupportsRule(SupportsRule* rule) override final;
    void visitUseRule(UseRule* rule) override final;
    void visitAssignRule(AssignRule* rule) override final;
    void visitWarnRule(WarnRule* rule) override final;
    void visitWhileRule(WhileRule* rule) override final;

  };

}

#endif

/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VISITOR_STATEMENT_HPP
#define SASS_VISITOR_STATEMENT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

namespace Sass {

  // An interface for [visitors][] that traverse Sass statements.
  // [visitors]: https://en.wikipedia.org/wiki/Visitor_pattern

  template <typename T>
  class StatementVisitor {
  public:

    bool inImport81 = false;

    virtual T visitAtRootRule(AtRootRule*) = 0;
    virtual T visitAtRule(AtRule*) = 0;
    virtual T visitContentBlock(ContentBlock*) = 0;
    virtual T visitContentRule(ContentRule*) = 0;
    virtual T visitDebugRule(DebugRule*) = 0;
    virtual T visitDeclaration(Declaration*) = 0;
    virtual T visitEachRule(EachRule*) = 0;
    virtual T visitErrorRule(ErrorRule*) = 0;
    virtual T visitExtendRule(ExtendRule*) = 0;
    virtual T visitForRule(ForRule*) = 0;
    virtual T visitForwardRule(ForwardRule*) = 0;
    virtual T visitFunctionRule(FunctionRule*) = 0;
    virtual T visitIfRule(IfRule*) = 0;
    virtual T visitImportRule(ImportRule*) = 0;
    virtual T visitIncludeRule(IncludeRule*) = 0;
    virtual T visitLoudComment(LoudComment*) = 0;
    virtual T visitMediaRule(MediaRule*) = 0;
    virtual T visitMixinRule(MixinRule*) = 0;
    virtual T visitReturnRule(ReturnRule*) = 0;
    virtual T visitSilentComment(SilentComment*) = 0;
    virtual T visitStyleRule(StyleRule*) = 0;
    //virtual T visitStylesheet(Stylesheet*) = 0;
    virtual T visitSupportsRule(SupportsRule*) = 0;
    virtual T visitUseRule(UseRule*) = 0;
    // Renamed from visitVariableDeclaration
    virtual T visitAssignRule(AssignRule*) = 0;
    virtual T visitWarnRule(WarnRule*) = 0;
    virtual T visitWhileRule(WhileRule*) = 0;

  };

  template <typename T>
  class StatementVisitable {
  public:
    virtual T accept(StatementVisitor<T>* visitor) = 0;
  };

}

#define DECLARE_STATEMENT_ACCEPT(T, name)\
  T accept(StatementVisitor<T>* visitor) override final {\
    return visitor->visit##name(this);\
  }\

#endif

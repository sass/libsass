#include "ast_fwd_decl.hpp"

#include "ast_css.hpp"
#include "ast_nodes.hpp"
#include "ast_values.hpp"
#include "ast_statements.hpp"
#include "ast_supports.hpp"
#include "ast_selectors.hpp"

namespace Sass {

  #define IMPLEMENT_BASE_CAST(T) \
  template<> \
  T* Cast(AstNode* ptr) { \
    return dynamic_cast<T*>(ptr); \
  }; \
  \
  template<> \
  const T* Cast(const AstNode* ptr) { \
    return dynamic_cast<const T*>(ptr); \
  }; \

  IMPLEMENT_BASE_CAST(Expression);
  IMPLEMENT_BASE_CAST(Statement);
  IMPLEMENT_BASE_CAST(ParentStatement);
  IMPLEMENT_BASE_CAST(CssParentNode);
  IMPLEMENT_BASE_CAST(CallableInvocation);
  IMPLEMENT_BASE_CAST(CallableDeclaration);
  IMPLEMENT_BASE_CAST(Value);
  IMPLEMENT_BASE_CAST(Color);
  IMPLEMENT_BASE_CAST(List);
  IMPLEMENT_BASE_CAST(Callable);
  IMPLEMENT_BASE_CAST(String);
  IMPLEMENT_BASE_CAST(SupportsCondition);
  IMPLEMENT_BASE_CAST(Selector);
  IMPLEMENT_BASE_CAST(SelectorComponent);
  IMPLEMENT_BASE_CAST(SimpleSelector);
  IMPLEMENT_BASE_CAST(NameSpaceSelector);
  IMPLEMENT_BASE_CAST(CssNode);


}

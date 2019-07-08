/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_statements.hpp"

#include "ast_supports.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ParentStatement::ParentStatement(
    SourceSpan&& pstate,
    StatementVector&& children,
    VarRefs* idxs) :
    Statement(std::move(pstate)),
    Vectorized<Statement>(std::move(children)),
    idxs_(idxs)
  {}

  // Returns whether we have a child content block
  bool ParentStatement::hasContent() const
  {
    if (Statement::hasContent()) return true;
    for (const StatementObj& child : elements_) {
      if (child->hasContent()) return true;
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  StyleRule::StyleRule(
    SourceSpan&& pstate,
    Interpolation* interpolation,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    interpolation_(interpolation)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Declaration::Declaration(
    SourceSpan&& pstate,
    Interpolation* name,
    Expression* value,
    bool is_custom_property,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children)),
    name_(name),
    value_(value),
    is_custom_property_(is_custom_property)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ForRule::ForRule(
    SourceSpan&& pstate,
    const EnvKey& varname,
    Expression* lower_bound,
    Expression* upper_bound,
    bool is_inclusive,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    varname_(varname),
    lower_bound_(lower_bound),
    upper_bound_(upper_bound),
    is_inclusive_(is_inclusive)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  EachRule::EachRule(
    SourceSpan&& pstate,
    const EnvKeys& variables,
    Expression* expressions,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    variables_(variables),
    expressions_(expressions)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  WhileRule::WhileRule(
    SourceSpan&& pstate,
    Expression* condition,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    condition_(condition)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  MediaRule::MediaRule(
    SourceSpan&& pstate,
    Interpolation* query,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children)),
    query_(query)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AtRule::AtRule(
    SourceSpan&& pstate,
    Interpolation* name,
    Interpolation* value,
    bool isChildless,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children)),
    name_(name),
    value_(value),
    isChildless_(isChildless)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AtRootRule::AtRootRule(
    SourceSpan&& pstate,
    Interpolation* query,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    query_(query)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  IfRule::IfRule(
    SourceSpan&& pstate,
    VarRefs* idxs,
    StatementVector&& children,
    Expression* predicate,
    IfRule* alternative) :
    ParentStatement(
      std::move(pstate),
      std::move(children)),
    idxs_(idxs),
    predicate_(predicate),
    alternative_(alternative)
  {}

  // Also check alternative for content block
  bool IfRule::hasContent() const
  {
    if (ParentStatement::hasContent()) return true;
    return alternative_ && alternative_->hasContent();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SupportsRule::SupportsRule(
    SourceSpan&& pstate,
    SupportsCondition* condition,
    VarRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    condition_(condition)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CallableDeclaration::CallableDeclaration(
    SourceSpan&& pstate,
    const EnvKey& name,
    ArgumentDeclaration* arguments,
    StatementVector&& children,
    SilentComment* comment,
    VarRefs* idxs) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    name_(name),
    comment_(comment),
    arguments_(arguments)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  IncludeRule::IncludeRule(
    SourceSpan&& pstate,
    const EnvKey& name,
    ArgumentInvocation* arguments,
    const sass::string& ns,
    ContentBlock* content) :
    Statement(std::move(pstate)),
    CallableInvocation(arguments),
    ns_(ns),
    name_(name),
    content_(content)
  {}

  bool IncludeRule::hasContent() const
  {
    return !content_.isNull();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ContentBlock::ContentBlock(
    SourceSpan&& pstate,
    ArgumentDeclaration* arguments,
    VarRefs* idxs,
    StatementVector&& children,
    SilentComment* comment) :
    CallableDeclaration(
      std::move(pstate),
      Keys::contentRule,
      arguments,
      std::move(children),
      comment, idxs)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  FunctionRule::FunctionRule(
    SourceSpan&& pstate,
    const EnvKey& name,
    ArgumentDeclaration* arguments,
    VarRefs* idxs,
    StatementVector&& children,
    SilentComment* comment) :
    CallableDeclaration(
      std::move(pstate),
      name, arguments,
      std::move(children),
      comment, idxs)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  MixinRule::MixinRule(
    SourceSpan&& pstate,
    const sass::string& name,
    ArgumentDeclaration* arguments,
    VarRefs* idxs,
    StatementVector&& children,
    SilentComment* comment) :
    CallableDeclaration(
      std::move(pstate),
      name, arguments,
      std::move(children),
      comment, idxs)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  WarnRule::WarnRule(
    SourceSpan&& pstate,
    Expression* expression) :
    Statement(std::move(pstate)),
    expression_(expression)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ErrorRule::ErrorRule(
    SourceSpan&& pstate,
    Expression* expression) :
    Statement(std::move(pstate)),
    expression_(expression)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  DebugRule::DebugRule(
    SourceSpan&& pstate,
    Expression* expression) :
    Statement(std::move(pstate)),
    expression_(expression)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ReturnRule::ReturnRule(
    SourceSpan&& pstate,
    Expression* value) :
    Statement(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ContentRule::ContentRule(
    SourceSpan&& pstate,
    ArgumentInvocation* arguments) :
    Statement(std::move(pstate)),
    arguments_(arguments)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ExtendRule::ExtendRule(
    SourceSpan&& pstate,
    Interpolation* selector,
    bool is_optional) :
    Statement(std::move(pstate)),
    selector_(selector),
    is_optional_(is_optional)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  LoudComment::LoudComment(
    SourceSpan&& pstate,
    Interpolation* text) :
    Statement(std::move(pstate)),
    text_(text)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SilentComment::SilentComment(
    SourceSpan&& pstate,
    sass::string&& text) :
    Statement(pstate),
    text_(text)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ImportRule::ImportRule(
    const SourceSpan& pstate) :
    Statement(pstate)
  {}


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AssignRule::AssignRule(
    const SourceSpan& pstate,
    const EnvKey& variable,
    VarRef vidx,
    Expression* value,
    bool is_default,
    bool is_global) :
    Statement(pstate),
    variable_(variable),
    value_(value),
    vidxs_({ vidx }),
    is_default_(is_default),
    is_global_(is_global)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}


/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_statements.hpp"

#include "ast_supports.hpp"
#include "exceptions.hpp"
#include "compiler.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  WithConfig::WithConfig(
    WithConfig* pwconfig,
    sass::vector<WithConfigVar> configs,
    bool hasConfig,
    bool hasShowFilter,
    bool hasHideFilter,
    std::set<EnvKey> varFilters,
    std::set<EnvKey> callFilters,
    const sass::string& prefix) :
    parent(pwconfig),
    hasConfig(hasConfig),
    hasShowFilter(hasShowFilter),
    hasHideFilter(hasHideFilter),
    varFilters(varFilters),
    callFilters(callFilters),
    prefix(prefix)
  {
    // Populate config map from vector
    // Duplicate entries are overwritten
    for (auto cfgvar : configs) {
      config[cfgvar.name] = cfgvar;
    }
  }

  void WithConfig::finalize(Logger& logger)
  {
    // Check if everything was consumed
    for (auto cfgvar : config) {
      if (cfgvar.second.wasAssigned == false) {
        if (cfgvar.second.isGuarded41 == false) {
          callStackFrame csf(logger, cfgvar.second.pstate);
          throw Exception::RuntimeException(logger, "$" +
            cfgvar.second.name + " was not declared "
            "with !default in the @used module.");
        }
      }
    }
  }

  WithConfigVar* WithConfig::getCfgVar(const EnvKey& name)
  {

    EnvKey key(name);
    WithConfig* withcfg = this;
    while (withcfg) {
      // Check if we should apply any filtering first
      if (withcfg->hasHideFilter) {
        if (withcfg->varFilters.count(key.norm())) {
          break;
        }
      }
      if (withcfg->hasShowFilter) {
        if (!withcfg->varFilters.count(key.norm())) {
          break;
        }
      }
      // Then try to find the named item
      auto varcfg = withcfg->config.find(key);
      if (varcfg != withcfg->config.end()) {
        // Found an unguarded value
        if (!varcfg->second.isGuarded41) {
          if (!varcfg->second.isNull()) {
            varcfg->second.wasAssigned = true;
            return &varcfg->second;
          }
        }
      }
      // Should we apply some prefixes
      if (!withcfg->prefix.empty()) {
        sass::string prefix = withcfg->prefix;
        key = EnvKey(prefix + key.orig());
      }
      withcfg = withcfg->parent;
    }
    // Since we found no unguarded value we can assume
    // the full stack only contains guarded values.
    // Therefore they overwrite all each other.
    withcfg = this; key = name;
    WithConfigVar* guarded = nullptr;
    while (withcfg) {
      // Check if we should apply any filtering first
      if (withcfg->hasHideFilter) {
        if (withcfg->varFilters.count(key.norm())) {
          break;
        }
      }
      if (withcfg->hasShowFilter) {
        if (!withcfg->varFilters.count(key.norm())) {
          break;
        }
      }
      // Then try to find the named item
      auto varcfg = withcfg->config.find(key);
      if (varcfg != withcfg->config.end()) {
        varcfg->second.wasAssigned = true;
        if (!guarded) guarded = &varcfg->second;
      }
      // Should we apply some prefixes
      if (!withcfg->prefix.empty()) {
        sass::string prefix = withcfg->prefix;
        key = EnvKey(prefix + key.orig());
      }
      withcfg = withcfg->parent;
    }
    return guarded;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ParentStatement::ParentStatement(
    SourceSpan&& pstate,
    StatementVector&& children,
    EnvRefs* idxs) :
    Statement(std::move(pstate)),
    Vectorized<Statement>(std::move(children)),
    Env(idxs)
  {}

  ParentStatement::ParentStatement(
    const SourceSpan& pstate,
    StatementVector&& children,
    EnvRefs* idxs) :
    Statement(std::move(pstate)),
    Vectorized<Statement>(std::move(children)),
    Env(idxs)
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
    EnvRefs* idxs,
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
      std::move(children),
      nullptr),
    name_(name),
    value_(value),
    is_custom_property_(is_custom_property)
  {}

  bool Declaration::isCustomProperty() const
  {
    if (name_.isNull()) return false;
    const sass::string& plain = name_->getInitialPlain();
    return StringUtils::startsWith(plain, "--", 2);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ForRule::ForRule(
    SourceSpan&& pstate,
    const EnvKey& varname,
    Expression* lower_bound,
    Expression* upper_bound,
    bool is_inclusive,
    EnvRefs* idxs,
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
    EnvRefs* idxs,
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
    EnvRefs* idxs,
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
    EnvRefs* idxs,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    query_(query)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AtRule::AtRule(
    SourceSpan&& pstate,
    Interpolation* name,
    Interpolation* value,
    EnvRefs* idxs,
    bool isChildless,
    StatementVector&& children) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    name_(name),
    value_(value),
    isChildless_(isChildless)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AtRootRule::AtRootRule(
    SourceSpan&& pstate,
    Interpolation* query,
    EnvRefs* idxs,
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
    EnvRefs* idxs,
    StatementVector&& children,
    Expression* predicate,
    IfRule* alternative) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
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
    EnvRefs* idxs,
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
    CallableSignature* arguments,
    StatementVector&& children,
    SilentComment* comment,
    EnvRefs* idxs) :
    ParentStatement(
      std::move(pstate),
      std::move(children),
      idxs),
    name_(name),
    comment_(comment),
    arguments_(arguments)
  {}

  CallableDeclaration::CallableDeclaration(
    const SourceSpan& pstate,
    const EnvKey& name,
    CallableSignature* arguments,
    StatementVector&& children,
    SilentComment* comment,
    EnvRefs* idxs) :
    ParentStatement(
      pstate,
      std::move(children),
      idxs),
    name_(name),
    comment_(comment),
    arguments_(arguments)
  {}

  bool CallableDeclaration::operator==(const CallableDeclaration & rhs) const
  {
    return name_ == rhs.name_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  IncludeRule::IncludeRule(
    SourceSpan&& pstate,
    const EnvKey& name,
    const sass::string& ns,
    CallableArguments* arguments,
    ContentBlock* content) :
    Statement(std::move(pstate)),
    arguments_(arguments),
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
    CallableSignature* arguments,
    EnvRefs* idxs,
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
    CallableSignature* arguments,
    EnvRefs* idxs,
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
    CallableSignature* arguments,
    EnvRefs* idxs,
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
    CallableArguments* arguments) :
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

  ModRule::ModRule(
    const sass::string& prev,
    const sass::string& url,
    WithConfig* pwconfig,
    sass::vector<WithConfigVar>&& config,
    bool hasLocalWith) :
    WithConfig(pwconfig,
      std::move(config),
      hasLocalWith),
    prev51_(prev),
    url_(url),
    module32_(nullptr),
    wasExposed_(false)
  {}

  ModRule::ModRule(
    const sass::string& prev,
    const sass::string& url,
    const sass::string& prefix,
    WithConfig* pwconfig,
    std::set<EnvKey>&& varFilters,
    std::set<EnvKey>&& callFilters,
    sass::vector<WithConfigVar>&& config,
    bool isShown,
    bool isHidden,
    bool hasWith) :
    WithConfig(pwconfig,
      std::move(config),
      hasWith, isShown, isHidden,
      std::move(varFilters),
      std::move(callFilters),
      prefix),
    prev51_(prev),
    url_(url),
    module32_(nullptr),
    wasExposed_(false)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  UseRule::UseRule(
    const SourceSpan& pstate,
    const sass::string& prev,
    const sass::string& url,
    Import* import,
    WithConfig* pwconfig,
    sass::vector<WithConfigVar>&& config,
    bool hasLocalWith) :
    Statement(pstate),
    ModRule(
      prev, url, pwconfig,
      std::move(config),
      hasLocalWith)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ForwardRule::ForwardRule(
    const SourceSpan& pstate,
    const sass::string& prev,
    const sass::string& url,
    Import* import,
    const sass::string& prefix,
    WithConfig* pwconfig,
    std::set<EnvKey>&& varFilters,
    std::set<EnvKey>&& callFilters,
    sass::vector<WithConfigVar>&& config,
    bool isShown,
    bool isHidden,
    bool hasWith) :
    Statement(pstate),
    ModRule(
      prev, url,
      prefix, pwconfig,
      std::move(varFilters),
      std::move(callFilters),
      std::move(config),
      isShown, isHidden, hasWith)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AssignRule::AssignRule(
    const SourceSpan& pstate,
    const EnvKey& variable,
    const sass::string ns,
    sass::vector<EnvRef> vidxs,
    Expression* value,
    bool is_default,
    bool is_global) :
    Statement(pstate),
    variable_(variable),
    ns_(ns),
    value_(value),
    is_default_(is_default),
    is_global_(is_global)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}


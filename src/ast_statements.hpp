/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_STATEMENTS_HPP
#define SASS_AST_STATEMENTS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <set>
#include "ast_nodes.hpp"
#include "ast_callable.hpp"
#include "ast_supports.hpp"
#include "ast_statements.hpp"
#include "ast_css.hpp"
#include "environment_cnt.hpp"
#include "environment_stack.hpp"
#include "file.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  struct WithConfigVar {
    // SourceSpan where this config var was defined
    SourceSpan pstate;
    // Name of this config variable (without dollar sign)
    sass::string name;
    // The assigned value or expressions
    ValueObj value33;
    ExpressionObj expression44;
    bool isGuarded41 = false;
    bool wasAssigned = false;
    bool isNull() const {
      return (value33 && value33->isaNull()) ||
        (expression44 && expression44->isaNullExpression());
    }
  };


  /////////////////////////////////////////////////////////////////////////
  // Helper class to manage nested `with` config for @use, @forward and
  // @include meta.load-css rules.
  /////////////////////////////////////////////////////////////////////////

  class WithConfig
  {
  public:

    WithConfig* parent = nullptr;

    // Flag if we do RAI
    bool hasConfig = false;
    bool hasShowFilter = false;
    bool hasHideFilter = false;

    std::set<EnvKey> varFilters;
    std::set<EnvKey> callFilters;

    sass::string prefix;

    // Our configuration
    EnvKeyMap<WithConfigVar> config;

    // Value constructor
    WithConfig(
      WithConfig* pwconfig,
      sass::vector<WithConfigVar> config,
      bool hasConfig = true,
      bool hasShowFilter = false,
      bool hasHideFilter = false,
      std::set<EnvKey> varFilters = {},
      std::set<EnvKey> callFilters = {},
      const sass::string& prefix = "");

    void finalize(Logger& logger);

    // Get value and mark it as used
    WithConfigVar* getCfgVar(const EnvKey& name);

  };

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements that contain blocks of statements.
  /////////////////////////////////////////////////////////////////////////

  class ParentStatement : public Statement,
    public Vectorized<Statement>, public Env
  {
  public:
    // Value constructor
    ParentStatement(
      SourceSpan&& pstate,
      StatementVector&& children,
      EnvRefs* idxs = nullptr);
    // Value constructor
    ParentStatement(
      const SourceSpan& pstate,
      StatementVector&& children,
      EnvRefs* idxs = nullptr);
    // Returns whether we have a child content block
    virtual bool hasContent() const override;
  };

  /////////////////////////////////////////////////////////////////////////
  // A style rule. This applies style declarations to elements 
  // that match a given selector. Formerly known as `Ruleset`.
  /////////////////////////////////////////////////////////////////////////

  class StyleRule final : public ParentStatement
  {

    // Interpolation forming this style rule
    ADD_CONSTREF(InterpolationObj, interpolation);

  public:

    // Value constructor
    StyleRule(SourceSpan&& pstate,
      Interpolation* interpolation,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitStyleRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitStyleRule(this);
    }

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(StyleRule);
  };

  /////////////////////////////////////////////////////////////////////////
  // Declarations -- style rules consisting of a property name and values.
  /////////////////////////////////////////////////////////////////////////

  class Declaration final : public ParentStatement
  {

    ADD_CONSTREF(InterpolationObj, name);
    ADD_CONSTREF(ExpressionObj, value);
    ADD_CONSTREF(bool, is_custom_property);

  public:

    // Value constructor
    Declaration(SourceSpan&& pstate,
      Interpolation* name,
      Expression* value = nullptr,
      bool is_custom_property = false,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitDeclaration(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitDeclaration(this);
    }

    bool isCustomProperty() const;

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@for` control directive.
  /////////////////////////////////////////////////////////////////////////

  class ForRule final : public ParentStatement
  {

    // Name of loop variable
    ADD_CONSTREF(EnvKey, varname);
    // Lower boundary to iterator from
    ADD_CONSTREF(ExpressionObj, lower_bound);
    // Upper boundary to iterator to
    ADD_CONSTREF(ExpressionObj, upper_bound);
    // Is upper boundary inclusive?
    ADD_CONSTREF(bool, is_inclusive);

  public:

    // Value constructor
    ForRule(
      SourceSpan&& pstate,
      const EnvKey& varname,
      Expression* lower_bound,
      Expression* upper_bound,
      bool is_inclusive = false,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitForRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitForRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@each` control directive.
  /////////////////////////////////////////////////////////////////////////

  class EachRule final : public ParentStatement
  {

    // Names of loop variables for key and value
    ADD_CONSTREF(EnvKeys, variables);

    // Expression object to loop over
    ADD_CONSTREF(ExpressionObj, expressions);

  public:

    // Value constructor
    EachRule(
      SourceSpan&& pstate,
      const EnvKeys& variables,
      Expression* expressions,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitEachRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitEachRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@while` control directive.
  /////////////////////////////////////////////////////////////////////////

  class WhileRule final : public ParentStatement
  {

    // Condition to evaluate every loop
    ADD_CONSTREF(ExpressionObj, condition);

  public:

    // Value constructor
    WhileRule(
      SourceSpan&& pstate,
      Expression* condition,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitWhileRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitWhileRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The query that determines on which platforms the styles will be in
  // effect. This is only parsed after the interpolation has been resolved.
  /////////////////////////////////////////////////////////////////////////

  class MediaRule final : public ParentStatement
  {

    // Interpolation forming this media rule
    ADD_CONSTREF(InterpolationObj, query)

  public:

    // Value constructor
    MediaRule(
      SourceSpan&& pstate,
      Interpolation* query,
      EnvRefs* idxs,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitMediaRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitMediaRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // At-rules -- arbitrary directives beginning with "@" 
  // The rules may have more optional statement blocks.
  /////////////////////////////////////////////////////////////////////////

  class AtRule final : public ParentStatement
  {

    // Interpolation naming this at-rule
    ADD_CONSTREF(InterpolationObj, name);
    // Interpolations with additional options
    ADD_CONSTREF(InterpolationObj, value);
    // Flag if the at-rule was parsed childless
    ADD_CONSTREF(bool, isChildless);

  public:

    // Value constructor
    AtRule(
      SourceSpan&& pstate,
      Interpolation* name,
      Interpolation* value,
      EnvRefs* idxs,
      bool is_childless = true,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitAtRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitAtRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // An `@at-root` rule
  /////////////////////////////////////////////////////////////////////////

  class AtRootRule final : public ParentStatement
  {

    // Interpolation forming this at-root rule
    ADD_CONSTREF(InterpolationObj, query);

  public:

    // Value constructor
    AtRootRule(
      SourceSpan&& pstate,
      Interpolation* query = nullptr,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitAtRootRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitAtRootRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@if` control directive.
  /////////////////////////////////////////////////////////////////////////

  class IfRule final : public ParentStatement
  {
    // Predicate is optional, which indicates an else block.
    // In this case further `alternatives` are simply ignored.
    ADD_CONSTREF(ExpressionObj, predicate);
    // The else or else-if block
    ADD_CONSTREF(IfRuleObj, alternative);

  public:

    // Value constructor
    IfRule(SourceSpan&& pstate,
      EnvRefs* idxs,
      StatementVector&& children = {},
      Expression* predicate = nullptr,
      IfRule* alternative = {});

    // Also check alternative for content block
    bool hasContent() const override final;

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitIfRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitIfRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // `@supports` rule.
  /////////////////////////////////////////////////////////////////////////

  class SupportsRule final : public ParentStatement
  {

    // The condition forming this supports rule
    ADD_CONSTREF(SupportsConditionObj, condition);

  public:

    SupportsRule(
      SourceSpan&& pstate,
      SupportsCondition* condition,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {});

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitSupportsRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitSupportsRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Base class for specialized callable declarations.
  /////////////////////////////////////////////////////////////////////////

  class CallableDeclaration : public ParentStatement
  {

    // The name of this callable.
    ADD_CONSTREF(EnvKey, name);
    // The comment immediately preceding this declaration.
    ADD_CONSTREF(SilentCommentObj, comment);
    // The declared arguments this callable accepts.
    ADD_CONSTREF(CallableSignatureObj, arguments);

  public:

    // Value constructor
    CallableDeclaration(
      SourceSpan&& pstate,
      const EnvKey& name,
      CallableSignature* arguments,
      StatementVector&& children = {},
      SilentComment* comment = nullptr,
      EnvRefs* idxs = nullptr);

    // Value constructor
    CallableDeclaration(
      const SourceSpan& pstate,
      const EnvKey& name,
      CallableSignature* arguments,
      StatementVector&& children = {},
      SilentComment* comment = nullptr,
      EnvRefs* idxs = nullptr);

    // Equality comparator (needed for `get-function` value)
    bool operator==(const CallableDeclaration& rhs) const;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(MixinRule);
  };

  /////////////////////////////////////////////////////////////////////////
  // ToDo: not exactly sure anymore
  /////////////////////////////////////////////////////////////////////////

  class ContentBlock final :
    public CallableDeclaration
  {

    // Content function reference
    ADD_CONSTREF(EnvRef, cidx);

  public:

    // Value constructor
    ContentBlock(
      SourceSpan&& pstate,
      CallableSignature* arguments = nullptr,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {},
      SilentComment* comment = nullptr);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitContentBlock(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitContentBlock(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Specialization for `@function` callables
  /////////////////////////////////////////////////////////////////////////

  class FunctionRule final :
    public CallableDeclaration
  {

    // Function reference
    ADD_CONSTREF(EnvRef, fidx);

  public:

    // Value constructor
    FunctionRule(
      SourceSpan&& pstate,
      const EnvKey& name,
      CallableSignature* arguments,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {},
      SilentComment* comment = nullptr);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitFunctionRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitFunctionRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Specialization for `@mixin` callables
  /////////////////////////////////////////////////////////////////////////

  class MixinRule final :
    public CallableDeclaration
  {

    // Mixin function reference
    ADD_CONSTREF(EnvRef, midx);

  public:

    // Value constructor
    MixinRule(
      SourceSpan&& pstate,
      const sass::string& name,
      CallableSignature* arguments,
      EnvRefs* idxs = nullptr,
      StatementVector&& children = {},
      SilentComment* comment = nullptr);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitMixinRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitMixinRule(this);
    }

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(MixinRule);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@warn` directive.
  /////////////////////////////////////////////////////////////////////////

  class WarnRule final : public Statement
  {

    // Expression forming this warning rule
    ADD_CONSTREF(ExpressionObj, expression);

  public:

    // Value constructor
    WarnRule(SourceSpan&& pstate,
      Expression* expression);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitWarnRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitWarnRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@error` directive.
  /////////////////////////////////////////////////////////////////////////
  class ErrorRule final : public Statement
  {

    // Expression forming this error rule
    ADD_CONSTREF(ExpressionObj, expression);

  public:

    // Value constructor
    ErrorRule(SourceSpan&& pstate,
      Expression* expression);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitErrorRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitErrorRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@debug` directive.
  /////////////////////////////////////////////////////////////////////////

  class DebugRule final : public Statement
  {

    // Expression forming this debug rule
    ADD_CONSTREF(ExpressionObj, expression);

  public:

    // Value constructor
    DebugRule(SourceSpan&& pstate,
      Expression* expression);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitDebugRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitDebugRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The @return directive for use inside SassScript functions.
  /////////////////////////////////////////////////////////////////////////

  class ReturnRule final : public Statement
  {

    // Expression forming this return rule
    ADD_CONSTREF(ExpressionObj, value);

  public:

    // Value constructor
    ReturnRule(SourceSpan&& pstate,
      Expression* value = nullptr);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitReturnRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitReturnRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The @content directive for mixin content blocks.
  /////////////////////////////////////////////////////////////////////////

  class ContentRule final : public Statement
  {

    // Expression forming this content rule
    ADD_CONSTREF(CallableArgumentsObj, arguments);

  public:

    // Value constructor
    ContentRule(SourceSpan&& pstate,
      CallableArguments* arguments);

    // Specialize method to indicate we have content
    bool hasContent() const override final { return true; }

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitContentRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitContentRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // The Sass `@extend` directive.
  /////////////////////////////////////////////////////////////////////////

  class ExtendRule final : public Statement
  {

    // This should be a simple selector only!
    // ToDo: change when we remove old code!
    ADD_CONSTREF(InterpolationObj, selector);
    // Flag if extend had optional flag
    ADD_CONSTREF(bool, is_optional);

  public:

    // Value constructor
    ExtendRule(
      SourceSpan&& pstate,
      Interpolation* selector,
      bool is_optional = false);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitExtendRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitExtendRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // CSS comments. These may be interpolated.
  /////////////////////////////////////////////////////////////////////////

  class LoudComment final : public Statement
  {
    // The interpolated text of this comment, including comment characters.
    ADD_CONSTREF(InterpolationObj, text)

  public:

    // Value constructor
    LoudComment(
      SourceSpan&& pstate,
      Interpolation* text);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitLoudComment(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitLoudComment(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Silent comment, starting with `//`.
  /////////////////////////////////////////////////////////////////////////

  class SilentComment final : public Statement
  {

    // The text of this comment, including comment characters.
    ADD_CONSTREF(sass::string, text)

  public:

    // Value constructor
    SilentComment(
      SourceSpan&& pstate,
      sass::string&& text);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitSilentComment(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitSilentComment(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // `@import` rule.
  /////////////////////////////////////////////////////////////////////////

  class ImportRule final : public Statement,
    public Vectorized<ImportBase>
  {

  public:

    // Value constructor
    ImportRule(const SourceSpan& pstate);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitImportRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitImportRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Base class for @use and @forward rules
  /////////////////////////////////////////////////////////////////////////

  class ModRule : public WithConfig
  {
  private:

    ADD_CONSTREF(sass::string, prev51);
    ADD_CONSTREF(sass::string, url);
    ADD_CONSTREF(sass::string, ns);

    // The associated module (required)
    ADD_PROPERTY(Module*, module32);

    // Optional root object, indicating that we have
    // a connected environment. Must be the same instance
    // as root if set (as Root implements Module).
    ADD_CONSTREF(RootObj, root47);

    // Flag to see if rule was already exposed
    // Normally modules are exposed as singletons
    ADD_CONSTREF(bool, wasExposed);

  public:

    ModRule(
      const sass::string& prev,
      const sass::string& url,
      WithConfig* pwconfig = nullptr,
      sass::vector<WithConfigVar>&& config = {},
      bool hasLocalWith = false);

    ModRule(
      const sass::string& prev,
      const sass::string& url,
      const sass::string& prefix,
      WithConfig* pwconfig,
      std::set<EnvKey>&& varFilters,
      std::set<EnvKey>&& callFilters,
      sass::vector<WithConfigVar>&& config,
      bool isShown,
      bool isHidden,
      bool hasWith);
  };


  /////////////////////////////////////////////////////////////////////////
  // `@use` rule.
  /////////////////////////////////////////////////////////////////////////

  class UseRule final : public Statement, public ModRule
  {
  public:

    // Value constructor
    UseRule(
      const SourceSpan& pstate,
      const sass::string& prev,
      const sass::string& url,
      Import* import,
      WithConfig* pwconfig,
      sass::vector<WithConfigVar>&& config,
      bool hasLocalWith);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitUseRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitUseRule(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // `@forward` rule.
  /////////////////////////////////////////////////////////////////////////

  class ForwardRule final : public Statement, public ModRule
  {
  public:

    // Value constructor
    ForwardRule(
      const SourceSpan& pstate,
      const sass::string& prev,
      const sass::string& url,
      Import* import,
      const sass::string& prefix,
      WithConfig* pwconfig,
      std::set<EnvKey>&& varFilters,
      std::set<EnvKey>&& callFilters,
      sass::vector<WithConfigVar>&& config,
      bool isShown, bool isHidden, bool hasWith);

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitForwardRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitForwardRule(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // Assignment rule to assign (evaluated) expression to variable
  /////////////////////////////////////////////////////////////////////////

  class AssignRule final : public Statement
  {

  private:

    ADD_CONSTREF(EnvRef, vidx);
    ADD_CONSTREF(EnvKey, variable);
    ADD_CONSTREF(sass::string, ns);
    ADD_CONSTREF(ExpressionObj, value);
    ADD_CONSTREF(bool, is_default); // ToDO rename
    ADD_CONSTREF(bool, is_global); // ToDO rename

  public:
    // Value constructor
    AssignRule(
      const SourceSpan& pstate,
      const EnvKey& variable,
      const sass::string ns,
      sass::vector<EnvRef> vidxs,
      Expression* value,
      bool is_default = false,
      bool is_global = false);
    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitAssignRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitAssignRule(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // The @include mixin invocation rule
  /////////////////////////////////////////////////////////////////////////

  class IncludeRule final : public Statement
  {

    // The arguments passed to the callable.
    ADD_CONSTREF(CallableArgumentsObj, arguments);

  private:

    // The namespace of the mixin being invoked, or
    // `null` if it's invoked without a namespace.
    ADD_CONSTREF(sass::string, ns);

    // The name of the mixin being invoked.
    ADD_CONSTREF(EnvKey, name);

    ADD_CONSTREF(EnvRef, midx);

    // The block that will be invoked for [ContentRule]s in the mixin
    // being invoked, or `null` if this doesn't pass a content block.
    ADD_CONSTREF(ContentBlockObj, content);

  public:

    // Value constructor
    IncludeRule(
      SourceSpan&& pstate,
      const EnvKey& name,
      const sass::string& ns,
      CallableArguments* arguments,
      ContentBlock* content = nullptr);

    // Check if we have a valid content block
    bool hasContent() const override final;

    // Statement visitor to sass values entry function
    Value* accept(StatementVisitor<Value*>* visitor) override final {
      return visitor->visitIncludeRule(this);
    }
    void accept(StatementVisitor<void>* visitor) override final {
      return visitor->visitIncludeRule(this);
    }
  };

}

#include "modules.hpp"
#include "stylesheet.hpp"

#endif

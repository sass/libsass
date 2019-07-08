#ifndef SASS_EVAL_HPP
#define SASS_EVAL_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// #include "context.hpp"
#include "extender.hpp"
#include "ast_supports.hpp"
#include "ast_callables.hpp"

namespace Sass {

  /*#####################################################################*/
  /*#####################################################################*/

  class Eval :
    public StatementVisitor<Value*>,
    public ExpressionVisitor<Value*> {

  private:

    // Some references
    Logger& logger456;
    Compiler& compiler;
    BackTraces& traces;

    // The extend handler
    Extender extender;

    CssParentNode* current;
    CssMediaVector mediaStack;
    SelectorLists originalStack;
    SelectorLists selectorStack;

    // The name of the current declaration parent. Used for BEM-
    // declaration blocks as in `div { prefix: { suffix: val; } }`;
    sass::string declarationName;

    CssMediaQueryVector mediaQueries;

    // The style rule that defines the current parent selector, if any.
    CssStyleRule* readStyleRule = nullptr;

    // Current content block
    UserDefinedCallable* content88 = nullptr;

    // Whether we're working with plain css.
    bool plainCss = false;

    // Whether we're currently executing a mixing.
    bool inMixin = false;

    // Whether we're currently executing a function.
    bool inFunction = false;

    // Whether we're currently building the output of an unknown at rule.
    bool inUnknownAtRule = false;

    // Whether we're directly within an `@at-root` rule excluding style rules.
    bool atRootExcludingStyleRule = false;

    // Whether we're currently building the output of a `@keyframes` rule.
    bool inKeyframes = false;




    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // ToDo: maybe create on demand for better pstate?
    // ToDo: do some benchmarks to check implications!
    BooleanObj bool_true;
    BooleanObj bool_false;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  public:

    // Value constructor
    Eval(Compiler& compiler,
      Logger& logger,
      bool isCss = false);

    // Query if we are in a mixin
    bool isInMixin() const {
      return inMixin;
    }

    // Query if use plain css
    bool isPlainCss() const {
      return plainCss;
    }

    // Query if we have a content block
    bool hasContentBlock() const {
      return content88 != nullptr;
    }

    // Check if there are any unsatisfied extends (will throw)
    bool checkForUnsatisfiedExtends(Extension& unsatisfied) const {
      return extender.checkForUnsatisfiedExtends(unsatisfied);
    }

    // Main entry point to evaluation
    CssRoot* acceptRoot(Root* b);

    // Another entry point for the `call` sass-function
    Value* acceptFunctionExpression(FunctionExpression* expression) {
      return visitFunctionExpression(expression);
    }

    // Converts the expression to css representation
    sass::string toCss(Expression* expression, bool quote = true);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Call built-in function with no overloads
    Value* execute(
      BuiltInCallable* callable,
      ArgumentInvocation* arguments,
      const SourceSpan& pstate,
      bool selfAssign);

    // Call built-in function with overloads
    Value* execute(
      BuiltInCallables* callable,
      ArgumentInvocation* arguments,
      const SourceSpan& pstate,
      bool selfAssign);

    // Used for user functions and also by
    // mixin includes and content includes.
    Value* execute(
      UserDefinedCallable* callable,
      ArgumentInvocation* arguments,
      const SourceSpan& pstate,
      bool selfAssign);

    // Call external C-API function
    Value* execute(
      ExternalCallable* callable,
      ArgumentInvocation* arguments,
      const SourceSpan& pstate,
      bool selfAssign);

    // Return plain css call as string
    // Used when function is not known
    Value* execute(
      PlainCssCallable* callable,
      ArgumentInvocation* arguments,
      const SourceSpan& pstate,
      bool selfAssign);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  private:

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    SelectorListObj& selector() { return selectorStack.back(); }
    SelectorListObj& original() { return originalStack.back(); }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Return root of current child
    CssParentNode* getRoot()
    {
      auto parent = current;
      while (parent->parent()) {
        parent = parent->parent();
      }
      return parent;
    }

    // Check if we currently build
    // the output of a style rule.
    bool isInStyleRule() const {
      return readStyleRule != nullptr &&
        !atRootExcludingStyleRule;
    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  private:

    // Fetch unevaluated positional argument (optionally by name)
    // Will error if argument is missing or available both ways
    // Note: only needed for lazy evaluation in if expressions
    Expression* getArgument(
      ExpressionVector& positional,
      ExpressionFlatMap& named,
      size_t idx, const EnvKey& name);

    // Fetch evaluated positional argument (optionally by name)
    // Will error if argument is missing or available both ways
    // Named arguments are consumed and removed from the hash
    Value* getParameter(
      ArgumentResults& evaled,
      size_t idx, const Argument* arg);

    // Call built-in function with no overloads
    Value* _runBuiltInCallable(
      ArgumentInvocation* arguments,
      BuiltInCallable* callable,
      const SourceSpan& pstate,
      bool selfAssign);

    // Call built-in function with overloads
    Value* _runBuiltInCallables(
      ArgumentInvocation* arguments,
      BuiltInCallables* callable,
      const SourceSpan& pstate,
      bool selfAssign);

    // Helper for _runBuiltInCallable(s)
    Value* _callBuiltInCallable(
      ArgumentResults& evaluated,
      const SassFnPair& function,
      const SourceSpan& pstate,
      bool selfAssign);

    // Used for user functions and also by
    // mixin includes and content includes.
    Value* _runUserDefinedCallable(
      ArgumentInvocation* evaled,
      UserDefinedCallable* callable,
      const SourceSpan& pstate);

    // Call external C-API function
    Value* _runExternalCallable(
      ArgumentInvocation* arguments,
      ExternalCallable* callable,
      const SourceSpan& pstate);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////
  private:
    ArgumentResults _evaluateArguments(ArgumentInvocation* arguments);
    void _addRestValueMap(ValueFlatMap& values, Map* map, const SourceSpan& nodeForSpan);
    void _addRestExpressionMap(ExpressionFlatMap& values, Map* map, const SourceSpan& pstate);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    sass::string acceptInterpolation(InterpolationObj interpolation, bool warnForColor, bool trim = false);
    SourceData* interpolationToSource(InterpolationObj interpolation, bool warnForColor, bool trim = false);
    CssString* interpolationToCssString(InterpolationObj interpolation, bool warnForColor, bool trim = false);
    SelectorListObj interpolationToSelector(Interpolation* interpolation, bool plainCss, bool allowParent = true);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    void _evaluateMacroArguments(CallableInvocation& invocation,
      ExpressionVector& positional,
      ExpressionFlatMap& named);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    void acceptStaticImport(StaticImport* import);
    void acceptIncludeImport(IncludeImport* import);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Evaluate all children at the currently existing block context
    Value* acceptChildren(const Vectorized<Statement>& children);

    // Evaluate all children at a newly established current block context
    Value* acceptChildrenAt(CssParentNode* parent, const Vectorized<Statement>& children);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    Value* visitBinaryOpExpression(BinaryOpExpression*);
    Value* visitBooleanExpression(BooleanExpression*);
    Value* visitColorExpression(ColorExpression*);
    Value* visitFunctionExpression(FunctionExpression*);
    Value* visitIfExpression(IfExpression*);
    Value* visitListExpression(ListExpression*);
    Value* visitMapExpression(MapExpression*);
    Value* visitNullExpression(NullExpression*);
    Value* visitNumberExpression(NumberExpression*);
    Value* visitParenthesizedExpression(ParenthesizedExpression*);
    Value* visitParentExpression(ParentExpression*);
    Value* visitStringExpression(StringExpression*);
    Value* visitUnaryOpExpression(UnaryOpExpression*);
    Value* visitValueExpression(ValueExpression*);
    Value* visitVariableExpression(VariableExpression*);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    Value* visitAtRootRule(AtRootRule* rule);
    Value* visitAtRule(AtRule* rule);
    Value* visitContentBlock(ContentBlock* rule);
    Value* visitContentRule(ContentRule* rule);
    Value* visitDebugRule(DebugRule* rule);
    Value* visitDeclaration(Declaration* rule);
    Value* visitEachRule(EachRule* rule);
    Value* visitErrorRule(ErrorRule* rule);
    Value* visitExtendRule(ExtendRule* rule);
    Value* visitForRule(ForRule* rule);
    // Value* visitForwardRule(ForwardRule* rule);
    Value* visitFunctionRule(FunctionRule* rule);
    Value* visitIfRule(IfRule* rule);
    Value* visitImportRule(ImportRule* rule);
    Value* visitIncludeRule(IncludeRule* rule);
    Value* visitLoudComment(LoudComment* rule);
    Value* visitMediaRule(MediaRule* rule);
    Value* visitMixinRule(MixinRule* rule);
    Value* visitReturnRule(ReturnRule* rule);
    Value* visitSilentComment(SilentComment* rule);
    Value* visitStyleRule(StyleRule* rule);
    // visitStylesheet
    Value* visitSupportsRule(SupportsRule* rule);
    // Value* visitUseRule(UseRule* rule);
    Value* visitAssignRule(AssignRule* rule);
    Value* visitWarnRule(WarnRule* rule);
    Value* visitWhileRule(WhileRule* rule);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    void callExternalMessageOverloadFunction(Callable* fn, Value* message);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    sass::string _visitSupportsCondition(SupportsCondition* condition);
    sass::string _parenthesize(SupportsCondition* condition);
    sass::string _parenthesize(SupportsCondition* condition, SupportsOperation::Operand operand);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    CssParentNode* _trimIncluded(CssParentVector& nodes);

    CssParentNode* hoistStyleRule(CssParentNode* node);

    CssMediaQueryVector mergeMediaQueries(const CssMediaQueryVector& lhs, const CssMediaQueryVector& rhs);













    CssMediaQueryVector evalMediaQueries(Interpolation* itpl);



  };

}

#endif

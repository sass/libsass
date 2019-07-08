#ifndef SASS_AST_FWD_DECL_HPP
#define SASS_AST_FWD_DECL_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <stack>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "sass/values.h"
#include "sass/functions.h"
#include "memory/shared_ptr.hpp"

/////////////////////////////////////////////
// Forward declarations for the AST visitors.
/////////////////////////////////////////////
namespace Sass {

  class Logger;
  class Compiler;
  class EnvFrame;

  class StyleSheet;

  class SourceData;
  class Import;
  class SourceFile;
  class SourceString;
  class SourceWithPath;
  class SourceItpl;

  class AstNode;
  class Root;

  class Callable;
  class UserDefinedCallable;
  class PlainCssCallable;
  class ExternalCallable;
  class BuiltInCallables;
  class BuiltInCallable;

  class ArgumentResults;
  class CallableInvocation;
  class ArgumentInvocation;
  class ArgumentDeclaration;
  class CallableDeclaration;
  class FunctionRule;
  class IncludeRule;
  class ContentBlock;
  class MixinRule;

  class ParentStatement;
  class CssParentNode;

  class SimpleSelector;
  class NameSpaceSelector;

  class ParentExpression;
  class BooleanExpression;
  class ColorExpression;
  class NumberExpression;
  class NullExpression;

  class Interpolant;
  class Expression;
  class Statement;
  class Value;
  class Declaration;
  class StyleRule;

  class MapExpression;
  class ListExpression;
  class ValueExpression;

  class MediaRule;

  class CssRoot;
  class CssNode;
  class CssString;
  class CssStringList;
  // class CssSelectors;
  class CssMediaRule;
  class CssMediaQuery;
  class CssAtRule;
  class CssComment;
  class CssDeclaration;
  class CssImport;
  class CssKeyframeBlock;
  class CssStyleRule;
  class CssSupportsRule;

  class SupportsRule;
  class AtRule;

  class AtRootRule;
  class AssignRule;

  class WarnRule;

  class ImportRule;
  class ImportBase;
  class StaticImport;
  class IncludeImport;

  class ErrorRule;
  class DebugRule;
  class LoudComment;
  class SilentComment;

  class IfRule;
  class ForRule;
  class EachRule;
  class WhileRule;
  class ReturnRule;
  class ContentRule;
  class ExtendRule;

  class List;
  class ArgumentList;
  class Map;
  class Function;

  class ParenthesizedExpression;
  class BinaryOpExpression;
  class UnaryOpExpression;
  class FunctionExpression;
  class IfExpression;
  class CustomWarning;
  class CustomError;

  class VariableExpression;
  class Number;
  class Color;
  class ColorRgba;
  class ColorHsla;
  class ColorHwba;
  class Boolean;
  class Null;

  class Interpolation;
  class ItplString;
  class StringExpression;

  class String;

  class SupportsCondition;
  class SupportsOperation;
  class SupportsNegation;
  class SupportsDeclaration;
  class SupportsInterpolation;
  
  class AtRootQuery;
  class Argument;
  class Selector;


  class PlaceholderSelector;
  class TypeSelector;
  class ClassSelector;
  class IDSelector;
  class AttributeSelector;

  class PseudoSelector;
  
  class SelectorComponent;
  class SelectorCombinator;
  class CompoundSelector;
  class ComplexSelector;
  class SelectorList;

  // common classes
  class Context;
  class Eval;

  class Extension;

  // declare classes that are instances of memory nodes
  #define IMPL_MEM_OBJ(type) \
    typedef SharedImpl<type> type##Obj;

  IMPL_MEM_OBJ(StyleSheet);

  IMPL_MEM_OBJ(SourceData);
  IMPL_MEM_OBJ(Import);
  IMPL_MEM_OBJ(SourceFile);
  IMPL_MEM_OBJ(SourceString);
  IMPL_MEM_OBJ(SourceWithPath);
  IMPL_MEM_OBJ(SourceItpl);

  IMPL_MEM_OBJ(AstNode);
  IMPL_MEM_OBJ(Statement);
  IMPL_MEM_OBJ(Root);
  IMPL_MEM_OBJ(StyleRule);
  IMPL_MEM_OBJ(MediaRule);

  IMPL_MEM_OBJ(MapExpression);
  IMPL_MEM_OBJ(ListExpression);
  IMPL_MEM_OBJ(ValueExpression);

  IMPL_MEM_OBJ(CssRoot);
  IMPL_MEM_OBJ(CssNode);
  IMPL_MEM_OBJ(CssStringList);
  IMPL_MEM_OBJ(CssString);
  // IMPL_MEM_OBJ(CssSelectors);
  IMPL_MEM_OBJ(CssMediaRule);
  IMPL_MEM_OBJ(CssMediaQuery);
  // IMPLEMENT_AST_OPERATORS(CssNode);
  IMPL_MEM_OBJ(CssAtRule);
  IMPL_MEM_OBJ(CssComment);
  IMPL_MEM_OBJ(CssDeclaration);
  IMPL_MEM_OBJ(CssImport);
  IMPL_MEM_OBJ(CssKeyframeBlock);
  IMPL_MEM_OBJ(CssStyleRule);
  IMPL_MEM_OBJ(CssSupportsRule);
  IMPL_MEM_OBJ(Callable);
  IMPL_MEM_OBJ(UserDefinedCallable);
  IMPL_MEM_OBJ(PlainCssCallable);
  IMPL_MEM_OBJ(ExternalCallable);
  IMPL_MEM_OBJ(BuiltInCallable);
  IMPL_MEM_OBJ(BuiltInCallables);
  IMPL_MEM_OBJ(SupportsRule);
  IMPL_MEM_OBJ(CallableDeclaration);
  IMPL_MEM_OBJ(FunctionRule);
  IMPL_MEM_OBJ(IncludeRule);
  IMPL_MEM_OBJ(ContentBlock); 
  IMPL_MEM_OBJ(MixinRule);
  IMPL_MEM_OBJ(AtRule);
  IMPL_MEM_OBJ(AtRootRule);
  IMPL_MEM_OBJ(Declaration);
  IMPL_MEM_OBJ(AssignRule);
  IMPL_MEM_OBJ(ImportRule);
  IMPL_MEM_OBJ(ImportBase);
  IMPL_MEM_OBJ(StaticImport);
  IMPL_MEM_OBJ(IncludeImport);
  IMPL_MEM_OBJ(WarnRule);
  IMPL_MEM_OBJ(ErrorRule);
  IMPL_MEM_OBJ(DebugRule);
  IMPL_MEM_OBJ(LoudComment);
  IMPL_MEM_OBJ(SilentComment);
  IMPL_MEM_OBJ(ParentStatement);
  IMPL_MEM_OBJ(CssParentNode);
  IMPL_MEM_OBJ(CallableInvocation);
  IMPL_MEM_OBJ(ArgumentInvocation);
  IMPL_MEM_OBJ(ArgumentDeclaration);
  IMPL_MEM_OBJ(IfRule);
  IMPL_MEM_OBJ(ForRule);
  IMPL_MEM_OBJ(EachRule);
  IMPL_MEM_OBJ(WhileRule);
  IMPL_MEM_OBJ(ReturnRule);
  IMPL_MEM_OBJ(ContentRule);
  IMPL_MEM_OBJ(ExtendRule);
  IMPL_MEM_OBJ(Value);
  IMPL_MEM_OBJ(Interpolant);
  IMPL_MEM_OBJ(Expression);
  IMPL_MEM_OBJ(List);
  IMPL_MEM_OBJ(ArgumentList);
  IMPL_MEM_OBJ(Map);
  IMPL_MEM_OBJ(Function);
  IMPL_MEM_OBJ(ParenthesizedExpression);
  IMPL_MEM_OBJ(BinaryOpExpression);
  IMPL_MEM_OBJ(UnaryOpExpression);
  IMPL_MEM_OBJ(FunctionExpression);
  IMPL_MEM_OBJ(IfExpression);
  IMPL_MEM_OBJ(CustomWarning);
  IMPL_MEM_OBJ(CustomError);
  IMPL_MEM_OBJ(VariableExpression);
  IMPL_MEM_OBJ(Number);
  IMPL_MEM_OBJ(Color);
  IMPL_MEM_OBJ(ColorRgba);
  IMPL_MEM_OBJ(ColorHsla);
  IMPL_MEM_OBJ(ColorHwba);
  IMPL_MEM_OBJ(Boolean);
  IMPL_MEM_OBJ(String);
  IMPL_MEM_OBJ(Interpolation);
  IMPL_MEM_OBJ(ItplString);
  IMPL_MEM_OBJ(StringExpression);
  IMPL_MEM_OBJ(SupportsCondition);
  IMPL_MEM_OBJ(SupportsOperation);
  IMPL_MEM_OBJ(SupportsNegation);
  IMPL_MEM_OBJ(SupportsDeclaration);
  IMPL_MEM_OBJ(SupportsInterpolation);
  IMPL_MEM_OBJ(AtRootQuery);
  IMPL_MEM_OBJ(Null);

  IMPL_MEM_OBJ(ParentExpression);
  IMPL_MEM_OBJ(BooleanExpression);
  IMPL_MEM_OBJ(ColorExpression);
  IMPL_MEM_OBJ(NumberExpression);
  IMPL_MEM_OBJ(NullExpression);

  IMPL_MEM_OBJ(Argument);
  IMPL_MEM_OBJ(Selector);
  IMPL_MEM_OBJ(SimpleSelector);
  IMPL_MEM_OBJ(NameSpaceSelector);
  IMPL_MEM_OBJ(PlaceholderSelector);
  IMPL_MEM_OBJ(TypeSelector);
  IMPL_MEM_OBJ(ClassSelector);
  IMPL_MEM_OBJ(IDSelector);
  IMPL_MEM_OBJ(AttributeSelector);
  IMPL_MEM_OBJ(PseudoSelector);

  IMPL_MEM_OBJ(SelectorComponent);
  IMPL_MEM_OBJ(SelectorCombinator);
  IMPL_MEM_OBJ(CompoundSelector);
  IMPL_MEM_OBJ(ComplexSelector);
  IMPL_MEM_OBJ(SelectorList);

  /////////////////////////////////////////////////////////////////////////#
  // some often used typedefs
  /////////////////////////////////////////////////////////////////////////#

  typedef sass::vector<sass::string> StringVector;
  typedef sass::vector<SelectorComponentObj> SelectorComponentVector;
  typedef sass::vector<ValueObj> ValueVector;
  typedef sass::vector<CssNodeObj> CssNodeVector;
  typedef sass::vector<CssParentNodeObj> CssParentVector;
  typedef sass::vector<CssMediaQueryObj> CssMediaQueryVector;
  typedef sass::vector<CssMediaRuleObj> CssMediaVector;
  typedef sass::vector<SelectorListObj> SelectorLists;
  typedef sass::vector<StatementObj> StatementVector;
  typedef sass::vector<ExpressionObj> ExpressionVector;
  typedef std::unordered_set<sass::string> StringSet;

  /////////////////////////////////////////////////////////////////////////#
  // explicit type conversion functions
  /////////////////////////////////////////////////////////////////////////#

  template<class T>
  T* Cast(AstNode* ptr);

  template<class T>
  const T* Cast(const AstNode* ptr);

  // sometimes you know the class you want to cast to is final
  // in this case a simple typeid check is faster and safe to use

  #define DECLARE_BASE_CAST(T) \
  template<> T* Cast(AstNode* ptr); \
  template<> const T* Cast(const AstNode* ptr); \

  /////////////////////////////////////////////////////////////////////////#
  // implement specialization for final classes
  /////////////////////////////////////////////////////////////////////////#

  DECLARE_BASE_CAST(AstNode)
  DECLARE_BASE_CAST(Expression)
  DECLARE_BASE_CAST(Statement)
  DECLARE_BASE_CAST(ParentStatement)
  DECLARE_BASE_CAST(CssParentNode)
  DECLARE_BASE_CAST(CallableInvocation)
  DECLARE_BASE_CAST(Value)
  DECLARE_BASE_CAST(Callable)
  DECLARE_BASE_CAST(Color)
  DECLARE_BASE_CAST(List)
  DECLARE_BASE_CAST(String)
  DECLARE_BASE_CAST(SupportsCondition)
  DECLARE_BASE_CAST(Selector)
  DECLARE_BASE_CAST(SimpleSelector)
  DECLARE_BASE_CAST(NameSpaceSelector);
  DECLARE_BASE_CAST(SelectorComponent)
  DECLARE_BASE_CAST(ImportBase);
  DECLARE_BASE_CAST(CssNode);

  class Eval;
  class Logger;
  class Compiler;
  class SourceSpan;

  #define FN_PROTOTYPE2 \
    const SourceSpan& pstate, \
    const ValueVector& arguments, \
    Compiler& compiler, \
    Eval& eval, \
    bool selfAssign \

}

#endif

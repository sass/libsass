#ifndef SASS_DEBUGGER_HPP
#define SASS_DEBUGGER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <queue>
#include <vector>
#include <string>
#include <sstream>
#include "ast.hpp"
#include "ast_fwd_decl.hpp"
#include "extender.hpp"
#include "extension.hpp"

#include "ordered_map.hpp"

using namespace Sass;

inline void debug_ast(AstNode* node, std::string ind = "");

inline sass::string ns_name(NameSpaceSelector* s)
{
  if (!s->hasNs()) return s->name();
  else return s->ns() + "|" + s->name();
}

inline std::string debug_pstate(SourceSpan pstate) {
  std::stringstream str;
  str << pstate.getLine() << ":";
  str << pstate.getColumn() << " - ";
  return str.str();
}

inline sass::string dbgValStr(CssString* str) {
  return str ? str->text() : "{nullptr}";
}

// inline sass::string debug_vec(const AstNode* node) {
//   if (node == NULL) return "null";
//   else return node->inspect();
// }

inline std::string debug_dude(sass::vector<sass::vector<int>> vec) {
  std::stringstream out;
  out << "{";
  bool joinOut = false;
  for (auto ct : vec) {
    if (joinOut) out << ", ";
    joinOut = true;
    out << "{";
    bool joinIn = false;
    for (auto nr : ct) {
      if (joinIn) out << ", ";
      joinIn = true;
      out << nr;
    }
    out << "}";
  }
  out << "}";
  return out.str();
}

inline std::string debug_vec(std::string& str) {
  return str;
}


inline std::string debug_vec(EnvKey key) {
  return key.norm().c_str();
}

inline std::string debug_vec(sass::vector<ComplexSelectorObj> vec) {
  std::stringstream out;
  out << "[";
  SelectorListObj slist = SASS_MEMORY_NEW(SelectorList, SourceSpan::tmp("DBG"), std::move(vec));
  out << slist->inspect();
  out << "]";
  return out.str();
}

template <class T>
inline std::string debug_vec(const sass::vector<T>& vec) {
  std::stringstream out;
  out << "[";
  for (size_t i = 0; i < vec.size(); i += 1) {
    if (i > 0) out << ", ";
    out << debug_vec(vec[i]);
  }
  out << "]";
  return out.str();
}

template <class T>
inline std::string debug_vec(std::queue<T> vec) {
  std::stringstream out;
  out << "{";
  for (size_t i = 0; i < vec.size(); i += 1) {
    if (i > 0) out << ", ";
    out << debug_vec(vec[i]);
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O>
inline std::string debug_vec(std::map<T, U, O> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(it->first) // string (key)
      << ": "
      << debug_vec(it->second); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_vec(const ordered_map<T, U, O, V>& vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(*it); // string (key)
    // << debug_vec(it->second); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_vec(std::unordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(it->first) // string (key)
      << ": "
      << debug_vec(it->second); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_keys(std::unordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(it->first); // string (key)
    joinit = true;
  }
  out << "}";
  return out.str();
}

inline std::string debug_vec(ExtListSelSet& vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    // out << debug_vec(*it); // string (key)
    joinit = true;
  }
  out << "}";
  return out.str();
}

/*
template <class T, class U, class O, class V>
inline std::string debug_values(tsl::ordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(const_cast<U&>(it->second)); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_vec(tsl::ordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(it->first) // string (key)
      << ": "
      << debug_vec(const_cast<U&>(it->second)); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_vals(tsl::ordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(const_cast<U&>(it->second)); // string's value
    joinit = true;
  }
  out << "}";
  return out.str();
}

template <class T, class U, class O, class V>
inline std::string debug_keys(tsl::ordered_map<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto it = vec.begin(); it != vec.end(); it++)
  {
    if (joinit) out << ", ";
    out << debug_vec(it->first);
    joinit = true;
  }
  out << "}";
  return out.str();
}
*/

template <class T, class U>
inline std::string debug_vec(std::set<T, U> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto item : vec) {
    if (joinit) out << ", ";
    out << debug_vec(item);
    joinit = true;
  }
  out << "}";
  return out.str();
}

/*
template <class T, class U, class O, class V>
inline std::string debug_vec(tsl::ordered_set<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto item : vec) {
    if (joinit) out << ", ";
    out << debug_vec(item);
    joinit = true;
  }
  out << "}";
  return out.str();
}
*/

template <class T, class U, class O, class V>
inline std::string debug_vec(std::unordered_set<T, U, O, V> vec) {
  std::stringstream out;
  out << "{";
  bool joinit = false;
  for (auto item : vec) {
    if (joinit) out << ", ";
    out << debug_vec(item);
    joinit = true;
  }
  out << "}";
  return out.str();
}

inline std::string debug_bool(bool val) {
  return val ? "true" : "false";
}
inline std::string debug_vec(ExtSmplSelSet* node) {
  // if (node == NULL) return "null";
  // else return debug_vec(*node);
  return "";
}

inline void debug_ast(const AstNode* node, std::string ind = "") {
  debug_ast(const_cast<AstNode*>(node), ind);
}

inline sass::string str_replace(sass::string str, const sass::string& oldStr, const sass::string& newStr)
{
  size_t pos = 0;
  while ((pos = str.find(oldStr, pos)) != std::string::npos)
  {
    str.replace(pos, oldStr.length(), newStr);
    pos += newStr.length();
  }
  return str;
}

inline sass::string prettyprint(const sass::string& str) {
  sass::string clean = str_replace(str, "\n", "\\n");
  clean = str_replace(clean, "	", "\\t");
  clean = str_replace(clean, "\r", "\\r");
  return clean;
}

inline std::string longToHex(long long t) {
  std::stringstream is;
  is << std::hex << t;
  return is.str();
}

inline std::string parent_block(VarRefs* node)
{
  std::stringstream str;
  if (!node) return "";
  str << "SG: " << (node->permeable ? "true" : "false");
  return str.str();
}

inline std::string pstate_source_position(AstNode* node)
{
  std::stringstream str;
  SourceSpan pstate(node->pstate());
  if (pstate.getSource()) {
    str << (pstate.getSrcIdx() == std::string::npos ? 9999999 : pstate.getSrcIdx())
      << "@[" << (pstate.position.line) << ":" << (pstate.position.column) << "]"
      << "+[" << (pstate.span.line) << ":" << (pstate.span.column) << "]"
      << "X" << node->refcount;
  }
  else {
    str << "[NOSRC]";
  }
#ifdef DEBUG_SHARED_PTR
  str << "x" << node->getRefCount() << ""
    << " {#" << node->objId << "}"
    << " " << node->getDbgFile()
    << "@" << node->getDbgLine();
#endif
  return str.str();
}
inline void debug_block(ParentStatement* node, std::string ind)
{
  for (auto item : node->elements()) {
    debug_ast(item, ind);
  }
}

inline void debug_block(CssParentNode* node, std::string ind)
{
  for (auto item : node->elements()) {
    debug_ast(item, ind);
  }
}

inline void debug_block(Root* node, std::string ind)
{
  for (auto item : node->elements()) {
    debug_ast(item, ind);
  }
}

inline void debug_css_parent_node(CssParentNode* rule, std::string ind = "")
{
  std::cerr << ind << "CssParentNode " << rule;
  std::cerr << " (" << pstate_source_position(rule) << ")";
  std::cerr << std::endl;
  debug_block(rule, ind + " ");
}

inline void debug_ast(AstNode* node, std::string ind)
{
  if (node == 0) return;
  if (ind == "") std::cerr << "####################################################################\n";
  if (Cast<Root>(node)) {
    Root* root = Cast<Root>(node);
    std::cerr << ind << "Root " << root;
    std::cerr << " (" << pstate_source_position(root) << ")";
    std::cerr << std::endl;
    debug_block(root, ind + " ");
  }
  else if (Cast<CssRoot>(node)) {
    CssRoot* root = Cast<CssRoot>(node);
    std::cerr << ind << "CssRoot " << root;
    std::cerr << " (" << pstate_source_position(root) << ")";
    std::cerr << std::endl;
    debug_block(root, ind + " ");
  }
  else if (Cast<CssComment>(node)) {
    CssComment* comment = Cast<CssComment>(node);
    std::cerr << ind << "CssComment " << comment;
    std::cerr << " (" << pstate_source_position(comment) << ")";
    std::cerr << std::endl;
  }
  else if (Cast<AtRootRule>(node)) {
    AtRootRule* root_block = Cast<AtRootRule>(node);
    std::cerr << ind << "AtRootRule " << root_block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << root_block->tabs();
    std::cerr << std::endl;
    debug_ast(root_block->query(), ind + ":");
    debug_block(root_block, ind + " ");
  }
  else if (Cast<AtRootQuery>(node)) {
    AtRootQuery* query = Cast<AtRootQuery>(node);
    std::cerr << ind << "AtRootQuery " << query;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " <" << query->inspect() << ">";
    std::cerr << std::endl;
  }
  else if (Cast<SelectorList>(node)) {
    SelectorList* selector = Cast<SelectorList>(node);
    std::cerr << ind << "SelectorList " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << (selector->hasInvisible() ? " [hasInvisible]" : " -");
    // std::cerr << (selector->has_real_parent_ref() ? " [real-parent]" : " -");
    std::cerr << std::endl;

    for (const ComplexSelectorObj& i : selector->elements()) { debug_ast(i, ind + " "); }

  }
  else if (Cast<ComplexSelector>(node)) {
    ComplexSelector* selector = Cast<ComplexSelector>(node);
    std::cerr << ind << "ComplexSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << (selector->chroots() ? "CHROOT" : "CONNECT") << "]";

      // << " [" << (selector->hasInvisible() ? "hasInvisible" : "") << "]"

    std::cerr << " [length:" << longToHex(selector->size()) << "]";
    std::cerr << " [weight:" << longToHex(selector->specificity()) << "]";
      // << (selector->hasInvisible() ? " [hasInvisible]" : " -")
    std::cerr << (selector->hasPreLineFeed() ? " [hasPreLineFeed]" : " -");

      // << (selector->has_placeholder() ? " [PLACEHOLDER]": " -")
      // << (selector->is_optional() ? " [is_optional]": " -")
      // << (selector->has_real_parent_ref() ? " [real parent]" : " -")
      // << (selector->has_line_feed() ? " [line-feed]": " -")
      // << (selector->has_line_break() ? " [line-break]": " -")
    std::cerr << " -- \n";

    for (const SelectorComponentObj& i : selector->elements()) { debug_ast(i, ind + " "); }

  }
  else if (Cast<SelectorCombinator>(node)) {
    SelectorCombinator* selector = Cast<SelectorCombinator>(node);
    std::cerr << ind << "SelectorCombinator " << selector
      << " (" << pstate_source_position(node) << ")"
      << " [weight:" << longToHex(selector->specificity()) << "]"
      // << (selector->has_real_parent_ref() ? " [real parent]" : " -")
      << " -- ";

    std::string del;
    switch (selector->combinator()) {
    case SelectorCombinator::CHILD:    del = ">"; break;
    case SelectorCombinator::GENERAL:  del = "~"; break;
    case SelectorCombinator::ADJACENT: del = "+"; break;
    }

    std::cerr << "[" << del << "]" << "\n";

  }
  else if (Cast<CompoundSelector>(node)) {
    CompoundSelector* selector = Cast<CompoundSelector>(node);
    std::cerr << ind << "CompoundSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << (selector->withExplicitParent() ? " [EXPLICIT PARENT]" : "") << ">";
    std::cerr << " [weight:" << longToHex(selector->specificity()) << "]";
    std::cerr << (selector->hasPostLineBreak() ? " [hasPostLineBreak]" : " -");
    // std::cerr << (selector->hasInvisible() ? " [hasInvisible]" : " -");
    std::cerr << "\n";
    for (const SimpleSelectorObj& i : selector->elements()) { debug_ast(i, ind + " "); }

  }
  else if (Cast<ParentExpression>(node)) {
    ParentExpression* selector = Cast<ParentExpression>(node);
    std::cerr << ind << "ParentExpression " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
  }

  else if (Cast<BooleanExpression>(node)) {
    BooleanExpression* selector = Cast<BooleanExpression>(node);
    std::cerr << ind << "ParentExpression " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
  }

  else if (Cast<ColorExpression>(node)) {
    ColorExpression* selector = Cast<ColorExpression>(node);
    std::cerr << ind << "ParentExpression " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
  }

  else if (Cast<NumberExpression>(node)) {
    NumberExpression* selector = Cast<NumberExpression>(node);
    std::cerr << ind << "NumberExpression " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
  }

  else if (Cast<NullExpression>(node)) {
    NullExpression* selector = Cast<NullExpression>(node);
    std::cerr << ind << "NullExpression " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
  }
  else if (Cast<PseudoSelector>(node)) {
    PseudoSelector* selector = Cast<PseudoSelector>(node);
    std::cerr << ind << "PseudoSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << selector->name() << ">>";
    std::cerr << (selector->isClass() ? " [isClass]" : " -");
    std::cerr << (selector->isPseudoElement() ? " [isPseudoElement]" : " -");
    std::cerr << (selector->isSyntacticClass() ? " [isSyntacticClass]" : " -");
    //  std::cerr << (selector->hasInvisible() ? " [hasInvisible]" : " -");
    std::cerr << std::endl;
    std::cerr << ind << " <= " << selector->argument() << std::endl;
    debug_ast(selector->selector(), ind + " || ");
  }
  else if (Cast<AttributeSelector>(node)) {
    AttributeSelector* selector = Cast<AttributeSelector>(node);
    std::cerr << ind << "AttributeSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << ns_name(selector) << ">>";
    std::cerr << std::endl;
    std::cerr << ind << "[" << selector->op() << "] ";
    std::cerr << selector->value() << std::endl;
  }
  else if (Cast<ClassSelector>(node)) {
    ClassSelector* selector = Cast<ClassSelector>(node);
    std::cerr << ind << "ClassSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << selector->name() << ">>";
    std::cerr << std::endl;
  }
  else if (Cast<IDSelector>(node)) {
    IDSelector* selector = Cast<IDSelector>(node);
    std::cerr << ind << "IDSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << selector->name() << ">>";
    std::cerr << std::endl;
  }
  else if (Cast<TypeSelector>(node)) {
    TypeSelector* selector = Cast<TypeSelector>(node);
    std::cerr << ind << "TypeSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << ns_name(selector) << ">>";
    // std::cerr << " <" << prettyprint(selector->pstate().token.ws_before()) << ">";
    std::cerr << std::endl;
  }
  else if (Cast<PlaceholderSelector>(node)) {

    PlaceholderSelector* selector = Cast<PlaceholderSelector>(node);
    std::cerr << ind << "PlaceholderSelector [" << selector->name() << "] " << selector;
    std::cerr << " (" << pstate_source_position(selector) << ")"
      //  << (selector->hasInvisible() ? " [hasInvisible]" : " -")
      << std::endl;

  }
  else if (Cast<SimpleSelector>(node)) {
    SimpleSelector* selector = Cast<SimpleSelector>(node);
    std::cerr << ind << "SimpleSelector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")";

  }
  else if (Cast<Selector>(node)) {
    Selector* selector = Cast<Selector>(node);
    std::cerr << ind << "Selector " << selector;
    std::cerr << " (" << pstate_source_position(node) << ")"
      << std::endl;
  }
  else if (Cast<ContentRule>(node)) {
    ContentRule* rule = Cast<ContentRule>(node);
    std::cerr << ind << "ContentRule " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << std::endl;
    debug_ast(rule->arguments(), ind + " =@ ");
  }
  else if (Cast<UserDefinedCallable>(node)) {
    UserDefinedCallable* rule = Cast<UserDefinedCallable>(node);
    std::cerr << ind << "UserDefinedCallable " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << std::endl;
    debug_ast(rule->declaration(), ind + " =@ ");
  }
  else if (Cast<ValueExpression>(node)) {
    ValueExpression* rule = Cast<ValueExpression>(node);
    std::cerr << ind << "ValueExpression " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << std::endl;
    debug_ast(rule->value(), ind + " =@ ");
  }

  else if (Cast<MediaRule>(node)) {
    MediaRule* rule = Cast<MediaRule>(node);
    std::cerr << ind << "MediaRule " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << " " << rule->tabs() << std::endl;
    debug_ast(rule->query(), ind + " =@ ");
    debug_block(rule, ind + " ");
  }
  else if (Cast<CssMediaRule>(node)) {
    CssMediaRule* rule = Cast<CssMediaRule>(node);
    std::cerr << ind << "CssMediaRule " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    for (auto item : rule->queries()) {
      debug_ast(item, ind + "() ");
    }
    for (auto item : rule->elements()) {
      debug_ast(item, ind + " !! ");
    }
    debug_css_parent_node(rule, ind + " :: ");
  }
  else if (Cast<CssDeclaration>(node)) {
    CssDeclaration* rule = Cast<CssDeclaration>(node);
    std::cerr << ind << "CssDeclaration " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << "\n";
    debug_ast(rule->name(), ind + " name: ");
    debug_ast(rule->value(), ind + " prop: ");
  }
  else if (Cast<CssString>(node)) {
    CssString* rule = Cast<CssString>(node);
    std::cerr << ind << "CssString " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << " <" << rule->text() << ">";
    std::cerr << std::endl;
  }
  else if (Cast<CssMediaQuery>(node)) {
    CssMediaQuery* query = Cast<CssMediaQuery>(node);
    std::cerr << ind << "CssMediaQuery " << query;
    std::cerr << " (" << pstate_source_position(query) << ")";
    std::cerr << " [" << (query->modifier()) << "] ";
    std::cerr << " [" << (query->type()) << "] ";
    std::cerr << " " << debug_vec(query->features());
    std::cerr << std::endl;
  }
  else if (Cast<SupportsRule>(node)) {
    SupportsRule* block = Cast<SupportsRule>(node);
    std::cerr << ind << "SupportsRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->condition(), ind + " =@ ");
    debug_block(block, ind + " <>");
    std::cerr << std::endl;
  }
  else if (Cast<CssSupportsRule>(node))
  {
    CssSupportsRule* block = Cast<CssSupportsRule>(node);
    std::cerr << ind << "CssSupportsRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    debug_ast(block->condition(), ind + " =@ ");
    for (auto stmt : block->elements()) {
      debug_ast(stmt, ind + " <>");
    }
  }
  else if (Cast<SupportsRule>(node)) {
    SupportsRule* block = Cast<SupportsRule>(node);
    std::cerr << ind << "SupportsRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->condition(), ind + " =@ ");
    debug_block(block, ind + " <>");
  }
  else if (Cast<SupportsOperation>(node)) {
    SupportsOperation* block = Cast<SupportsOperation>(node);
    std::cerr << ind << "SupportsOperation " << block;
    std::cerr << " (" << pstate_source_position(node) << ")"
      << std::endl;
    debug_ast(block->left(), ind + " left) ");
    debug_ast(block->right(), ind + " right) ");
  }
  else if (Cast<SupportsNegation>(node)) {
    SupportsNegation* block = Cast<SupportsNegation>(node);
    std::cerr << ind << "SupportsNegation " << block;
    std::cerr << " (" << pstate_source_position(node) << ")"
      << std::endl;
    debug_ast(block->condition(), ind + " condition) ");
  }
  else if (Cast<SupportsDeclaration>(node)) {
    SupportsDeclaration* block = Cast<SupportsDeclaration>(node);
    std::cerr << ind << "SupportsDeclaration " << block;
    std::cerr << " (" << pstate_source_position(node) << ")"
      << std::endl;
    debug_ast(block->feature(), ind + " feature) ");
    debug_ast(block->value(), ind + " value) ");
  }
  else if (Cast< SupportsCondition>(node)) {
    SupportsCondition* block = Cast<SupportsCondition>(node);
    std::cerr << ind << "SupportsDeclaration " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;

  }
  else if (Cast<WarnRule>(node)) {
    WarnRule* block = Cast<WarnRule>(node);
    std::cerr << ind << "WarnRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->expression(), ind + " : ");
  }
  else if (Cast<ErrorRule>(node)) {
    ErrorRule* block = Cast<ErrorRule>(node);
    std::cerr << ind << "ErrorRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->expression(), ind + " ");
  }
  else if (Cast<DebugRule>(node)) {
    DebugRule* block = Cast<DebugRule>(node);
    std::cerr << ind << "DebugRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->expression(), ind + " ");
  }
  else if (Cast<LoudComment>(node)) {
    LoudComment* block = Cast<LoudComment>(node);
    std::cerr << ind << "LoudComment " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->text(), ind + "// ");
  }
  else if (Cast<SilentComment>(node)) {
    SilentComment* block = Cast<SilentComment>(node);
    std::cerr << ind << "SilentComment " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << " [" << block->text() << "]" << std::endl;
    // debug_ast(block->text(), ind + "// ");
  }
  else if (Cast<IfRule>(node)) {
    IfRule* block = Cast<IfRule>(node);
    std::cerr << ind << "IfRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << parent_block(block->idxs()) << "]";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->predicate(), ind + " = ");
    debug_block(block, ind + " <>");
    debug_ast(block->alternative(), ind + " ><");
  }
  else if (Cast<ReturnRule>(node)) {
  ReturnRule* block = Cast<ReturnRule>(node);
    std::cerr << ind << "ReturnRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs();
    std::cerr << std::endl;
    debug_ast(block->value(), ind + " => ");
    // std::cerr << " [" << block->value()->inspect() << "]";
  }
  else if (Cast<ExtendRule>(node)) {
    ExtendRule* block = Cast<ExtendRule>(node);
    std::cerr << ind << "ExtendRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->selector(), ind + "-> ");
  }
  else if (Cast<ContentRule>(node)) {
    ContentRule* block = Cast<ContentRule>(node);
    std::cerr << ind << "ContentRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->arguments(), ind + " args: ");
  }
  else if (Cast<StaticImport>(node)) {
    StaticImport* block = Cast<StaticImport>(node);
    std::cerr << ind << "StaticImport " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << block->imp_path() << "] ";
    // std::cerr << " " << block->tabs();
    std::cerr << std::endl;
    debug_ast(block->url(), "url: ");
    debug_ast(block->supports(), "supports: ");
    debug_ast(block->media(), "media: ");
  }
  else if (Cast<CssImport>(node)) {
    CssImport* block = Cast<CssImport>(node);
    std::cerr << ind << "CssImport " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << block->url()->text() << "] ";
    std::cerr << std::endl;
  }
  else if (Cast<IncludeImport>(node)) {
    IncludeImport* block = Cast<IncludeImport>(node);
    std::cerr << ind << "IncludeImport " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << block->sheet() << "] ";
    std::cerr << std::endl;
  }
  else if (Cast<ImportRule>(node)) {
    ImportRule* block = Cast<ImportRule>(node);
    std::cerr << ind << "ImportRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << block->imp_path() << "] ";
    // std::cerr << " " << block->tabs();
    std::cerr << std::endl;
    for (auto imp : block->elements()) debug_ast(imp, ind + "@: ");
  }
  else if (Cast<AssignRule>(node)) {
    AssignRule* block = Cast<AssignRule>(node);
    std::cerr << ind << "AssignRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " <<" << block->variable().orig() << ">> " << block->tabs();
    std::cerr << " vidx(";
    bool join = false;
    for (auto pidx : block->vidxs()) {
      if (join) std::cerr << ", ";
      std::cerr << pidx.toString();
      join = true;
    }
    std::cerr << ")" << std::endl; 

    debug_ast(block->value(), ind + "=");
  }
  else if (Cast<Declaration>(node)) {
    Declaration* block = Cast<Declaration>(node);
    std::cerr << ind << "Declaration " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [is_custom_property: " << block->is_custom_property() << "] ";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->name(), ind + " name: ");
    debug_ast(block->value(), ind + " value: ");
    debug_block(block, ind + " ");
  }
  else if (Cast<AtRule>(node)) {
    AtRule* block = Cast<AtRule>(node);
    std::cerr << ind << "AtRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << block->keyword() << "] " << block->tabs();
    std::cerr << std::endl;
    // debug_ast(block->interpolation(), ind + "#");
    // debug_ast(block->value(), ind + "+");

    debug_ast(block->name(), ind + "#");
    debug_ast(block->value(), ind + "=");

    for (const StatementObj& i : block->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<EachRule>(node)) {
    EachRule* block = Cast<EachRule>(node);
    std::cerr << ind << "EachRule [" << debug_vec(block->variables()) << "]" << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    for (const StatementObj& i : block->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<ForRule>(node)) {
    ForRule* block = Cast<ForRule>(node);
    std::cerr << ind << "ForRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    for (const StatementObj& i : block->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<WhileRule>(node)) {
    WhileRule* block = Cast<WhileRule>(node);
    std::cerr << ind << "WhileRule " << block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << block->tabs() << std::endl;
    debug_ast(block->condition(), ind + " ?? ");
    for (const StatementObj& i : block->elements()) { debug_ast(i, ind + " "); }
  }


  else if (PlainCssCallable* ruleset = Cast<PlainCssCallable>(node)) {
    std::cerr << ind << "PlainCssCallable " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [indent: " << ruleset->tabs() << "]";
    // std::cerr << " [" << ruleset->name() << "]";
    std::cerr << std::endl;
    // debug_ast(ruleset->content(), ind + " @ ");
  }
  else if (IncludeRule* ruleset = Cast<IncludeRule>(node)) {
    std::cerr << ind << "IncludeRule " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [indent: " << ruleset->tabs() << "]";
    // std::cerr << " [" << ruleset->name() << "]";
    std::cerr << std::endl;
    debug_ast(ruleset->content(), ind + " @ ");
  }
  else if (ContentBlock* ruleset = Cast<ContentBlock>(node)) {
    std::cerr << ind << "ContentBlock " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [indent: " << ruleset->tabs() << "]";
    std::cerr << " [" << ruleset->name().orig() << "]";
    std::cerr << std::endl;
    debug_block(ruleset, ind + " ");
  }

  else if (MixinRule* ruleset = Cast<MixinRule>(node)) {
    std::cerr << ind << "MixinRule " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [indent: " << ruleset->tabs() << "]";
    std::cerr << " [" << ruleset->name().orig() << "]";
    std::cerr << std::endl;
    debug_ast(ruleset->arguments(), ind + "$");
    debug_block(ruleset, ind + " ");
  }
  else if (ArgumentDeclaration* args = Cast<ArgumentDeclaration>(node)) {
    std::cerr << ind << "MixinRArgumentDeclarationule " << args;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [indent: " << ruleset->tabs() << "]";
    // std::cerr << " [" << ruleset->name() << "]";
    std::cerr << std::endl;
    // debug_ast(ruleset->arguments(), ind + "$");
    // debug_ast(ruleset, ind + " ");
  }

  else if (FunctionRule* ruleset = Cast<FunctionRule>(node)) {
    std::cerr << ind << "FunctionRule " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [indent: " << ruleset->tabs() << "]";
    std::cerr << " [" << ruleset->name().orig() << "]";
    std::cerr << std::endl;
    debug_block(ruleset, ind + " ");
  }
  else if (StyleRule* ruleset = Cast<StyleRule>(node)) {
    std::cerr << ind << "StyleRule " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << parent_block(ruleset->idxs()) << "]";
    std::cerr << " [indent: " << ruleset->tabs() << "]";
    std::cerr << std::endl;
    debug_ast(ruleset->interpolation(), ind + "#");
    debug_block(ruleset, ind + " ");
  }
  else if (CssStyleRule* ruleset = Cast<CssStyleRule>(node)) {
    std::cerr << ind << "CssStyleRule " << ruleset;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
    debug_ast(ruleset->selector(), ind + ">");
    // debug_ast(ruleset->interpolation(), ind + "#");
    for (auto stmt : ruleset->elements()) {
      debug_ast(stmt, ind + " !! ");
    }
    // debug_ast(ruleset, ind + " :: ");
  }
  else if (Cast<VariableExpression>(node)) {
    VariableExpression* expression = Cast<VariableExpression>(node);
    std::cerr << ind << "VariableExpression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << expression->name().orig() << "]";
    std::cerr << " vidx(";
    bool join = false;
    for (auto pidx : expression->vidxs()) {
      if (join) std::cerr << ", ";
      std::cerr << pidx.toString();
      join = true;
    }
    std::cerr << ")" << std::endl;

    // sass::string name(expression->name());
    // if (env && env->has(name)) debug_ast(Cast<Expression>((*env)[name]), ind + " -> ");
  }
  else if (Cast<ArgumentInvocation>(node)) {
    ArgumentInvocation* arguments = Cast<ArgumentInvocation>(node);
    std::cerr << ind << "ArgumentInvocation " << arguments;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
    // std::cerr << ind << " positional: " << debug_vec(arguments->positional()) << "\n";
    std::cerr << ind << " named: " << arguments->named().size() << "\n";
    // std::cerr << ind << " restArg: " << debug_vec(arguments->restArg()) << "\n";
    // std::cerr << ind << " kwdRest: " << debug_vec(arguments->kwdRest()) << "\n";

  }
  else if (Cast<FunctionExpression>(node)) {
    FunctionExpression* expression = Cast<FunctionExpression>(node);
    std::cerr << ind << "FunctionExpression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [selfAssign: " << expression->selfAssign() << "]";
    // if (expression->is_css()) std::cerr << " [css]";
    std::cerr << std::endl;
    debug_ast(expression->name(), ind + " name: ");
    debug_ast(expression->arguments(), ind + " args: ");
    // debug_ast(expression->func(), ind + " func: ");
  }
  else if (Cast<Argument>(node)) {
    Argument* expression = Cast<Argument>(node);
    std::cerr << ind << "Argument " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << expression->defval().ptr() << "]";
    std::cerr << " [name: " << expression->name().orig() << "] ";
    std::cerr << " [rest: " << expression->is_rest_argument() << "] ";
    std::cerr << " [keyword: " << expression->is_keyword_argument() << "] " << std::endl;
    debug_ast(expression->defval(), ind + " value: ");
  }
  else if (Cast<UnaryOpExpression>(node)) {
    UnaryOpExpression* expression = Cast<UnaryOpExpression>(node);
    std::cerr << ind << "UnaryOpExpression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << expression->type() << "]" << std::endl;
    debug_ast(expression->operand(), ind + " operand: ");
  }
  else if (Cast<ParenthesizedExpression>(node)) {
    ParenthesizedExpression* expression = Cast<ParenthesizedExpression>(node);
    std::cerr << ind << "ParenthesizedExpression " << expression;
    std::cerr << " (" << pstate_source_position(expression) << ")";
    std::cerr << std::endl;
    debug_ast(expression->expression(), ind + "() ");

  }
  else if (Cast<BinaryOpExpression>(node)) {
    BinaryOpExpression* expression = Cast<BinaryOpExpression>(node);
    std::cerr << ind << "BinaryOpExpression " << expression;
    // std::cerr << " [ws_before: " << expression->operand().ws_before << "] ";
    // std::cerr << " [ws_after: " << expression->operand().ws_after << "] ";
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
    debug_ast(expression->left(), ind + " left:  ");
    debug_ast(expression->right(), ind + " right: ");
  }
  else if (Cast<Map>(node)) {
    Map* expression = Cast<Map>(node);
    std::cerr << ind << "Map " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [Hashed]" << std::endl;
    for (const auto& i : expression->elements()) {
      debug_ast(i.first, ind + " key: ");
      debug_ast(i.second, ind + " val: ");
    }
  }
  else if (Cast<ListExpression>(node)) {
    ListExpression* expression = Cast<ListExpression>(node);
    std::cerr << ind << "ListExpression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " (" << expression->size() << ") " <<
      (expression->separator() == SASS_COMMA ? "Comma " : expression->separator() == SASS_UNDEF ? "Unkonwn" : "Space ") <<
      " [bracketed: " << expression->hasBrackets() << "] " <<
      std::endl;
    for (size_t i = 0; i < expression->size(); i++) {
      debug_ast(expression->get(i), ind + " ");
    }
  }
  else if (Cast<MapExpression>(node)) {
    MapExpression* expression = Cast<MapExpression>(node);
    std::cerr << ind << "MapExpression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " (" << expression->size() << ") " <<
      std::endl;
    for (size_t i = 0; i < expression->size(); i++) {
      debug_ast(expression->get(i), ind + " ");
    }
  }
  else if (Cast<ArgumentList>(node)) {

    ArgumentList* expression = Cast<ArgumentList>(node);
    std::cerr << ind << "ArgumentList " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " (" << expression->size() << ") " <<
      (expression->separator() == SASS_COMMA ? "Comma " : expression->separator() == SASS_UNDEF ? "Unkonwn" : "Space ") <<
      " [bracketed: " << expression->hasBrackets() << "] " <<
      " [hash: " << expression->hash() << "] " <<
      std::endl;
    ValueFlatMap keywords = expression->keywords();
    for (const auto& i : expression->elements()) { debug_ast(i, ind + " [] "); }
    for (const auto& kv : keywords) { debug_ast(kv.second, ind + " " + kv.first.orig().c_str() + " "); }
  }
  else if (Cast<List>(node)) {
    List* expression = Cast<List>(node);
    std::cerr << ind << "List " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " (" << expression->size() << ") " <<
      (expression->separator() == SASS_COMMA ? "Comma " : expression->separator() == SASS_UNDEF ? "Unkonwn" : "Space ") <<
      " [bracketed: " << expression->hasBrackets() << "] " <<
      " [hash: " << expression->hash() << "] " <<
      std::endl;
    for (const auto& i : expression->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<Boolean>(node)) {
    Boolean* expression = Cast<Boolean>(node);
    std::cerr << ind << "Boolean " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << expression->value() << "]" << std::endl;
  }
  else if (Cast<ColorRgba>(node)) {
    ColorRgba* expression = Cast<ColorRgba>(node);
    std::cerr << ind << "Color " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [name: " << expression->disp() << "] ";
    std::cerr << " rgba[" << expression->r() << ":" << expression->g() << ":" << expression->b() << "@" << expression->a() << "]" << std::endl;
  }
  else if (Cast<ColorHsla>(node)) {
    ColorHsla* expression = Cast<ColorHsla>(node);
    std::cerr << ind << "Color " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [name: " << expression->disp() << "] ";
    std::cerr << " hsla[" << expression->h() << ":" << expression->s() << ":" << expression->l() << "@" << expression->a() << "]" << std::endl;
  }
  else if (Cast<Number>(node)) {
    Number* expression = Cast<Number>(node);
    std::cerr << ind << "Number " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << expression->value() << expression->unit() << "]" <<
      " [hash: " << expression->hash() << "] " <<
      std::endl;
    debug_ast(expression->lhsAsSlash(), ind + "[lhs] ");
    debug_ast(expression->rhsAsSlash(), ind + "[rhs] ");
  }
  else if (Cast<Null>(node)) {
    Null* expression = Cast<Null>(node);
    std::cerr << ind << "Null " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")"
      // " [hash: " << expression->hash() << "] "
      << std::endl;
  }
  else if (Cast<StringExpression>(node)) {
    StringExpression* expression = Cast<StringExpression>(node);
    // std::cerr << ind << "StringExpression " << expression;
    // std::cerr << " [" << prettyprint(expression->text()) << "]";
    std::cerr << ind << "StringExpression";
    std::cerr << " (" << pstate_source_position(node) << ")";
    if (expression->hasQuotes()) std::cerr << " {quoted}";
    std::cerr << std::endl;
    debug_ast(expression->text(), ind + "  ");
  }
  else if (Cast<ItplString>(node)) {
    ItplString* expression = Cast<ItplString>(node);
    std::cerr << ind << "ItplString " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << prettyprint(expression->text()) << "]";

    std::cerr << std::endl;
  }
  else if (Cast<String>(node)) {
    String* expression = Cast<String>(node);
    std::cerr << ind << "String ";
    if (expression->hasQuotes()) {
      std::cerr << "[QUOTED] ";
    }
    std::cerr << expression;
    std::cerr << " " << expression->type();
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " [" << prettyprint(expression->value()) << "]";
    // std::cerr << " <" << prettyprint(expression->pstate().token.ws_before()) << ">"
    std::cerr << std::endl;
  }
  else if (Cast<Interpolation>(node)) {
    Interpolation* expression = Cast<Interpolation>(node);
    std::cerr << ind << "Interpolation";
    std::cerr << " (" << pstate_source_position(expression) << ")";

    // std::cerr << ind << "Interpolation " << expression;
    // std::cerr << " " << expression->concrete_type();
    // std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << std::endl;
    for (const auto& i : expression->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<Expression>(node)) {
    Expression* expression = Cast<Expression>(node);
    std::cerr << ind << "Expression " << expression;
    std::cerr << " (" << pstate_source_position(node) << ")";
    // std::cerr << " [" << expression->type() << "]";
    std::cerr << std::endl;
  }
  else if (Cast<ParentStatement>(node)) {
    ParentStatement* has_block = Cast<ParentStatement>(node);
    std::cerr << ind << "Has_Block " << has_block;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << has_block->tabs() << std::endl;
    for (const StatementObj& i : has_block->elements()) { debug_ast(i, ind + " "); }
  }
  else if (Cast<Statement>(node)) {
    Statement* statement = Cast<Statement>(node);
    std::cerr << ind << "Statement " << statement;
    std::cerr << " (" << pstate_source_position(node) << ")";
    std::cerr << " " << statement->tabs() << std::endl;
  }
  else if (Cast<CssAtRule>(node)) {
  CssAtRule* rule = Cast<CssAtRule>(node);
    std::cerr << ind << "CssAtRule " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << " [name: " << dbgValStr(rule->name()) << "] ";
    std::cerr << " [value: " << dbgValStr(rule->value()) << "] ";
    std::cerr << std::endl;
    debug_block(rule, ind + " ");
  }
  else if (Cast<CssParentNode>(node)) {
    CssParentNode* rule = Cast<CssParentNode>(node);
    std::cerr << ind << "CssParentNode " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << std::endl;
    debug_block(rule, ind + " ");
  }
  else if (Cast<CssNode>(node)) {
    CssNode* rule = Cast<CssNode>(node);
    std::cerr << ind << "CssNode " << rule;
    std::cerr << " (" << pstate_source_position(rule) << ")";
    std::cerr << std::endl;
  }
  else {
    std::cerr << ind << "Undetected" << std::endl;
  }

  if (ind == "") std::cerr << "####################################################################\n";
}


/*
inline void debug_ast(const AstNode* node, std::string ind = ""* env = 0)
{
  debug_ast(const_cast<AstNode*>(node), ind);
}
*/

#endif // SASS_DEBUGGER

#ifndef SASS_DEBUGGER_H
#define SASS_DEBUGGER_H

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include <queue>
#include <vector>
#include <string>
#include <sstream>
#include "ast.hpp"
#include "ast_fwd_decl.hpp"
#include "extension.hpp"

#include "ordered_map.hpp"

using namespace Sass;

inline void debug_ast(AST_Node* node, std::string ind = "", Env* env = 0);

inline std::string debug_vec(const AST_Node* node) {
  if (node == NULL) return "null";
  else return node->to_string();
}

inline std::string debug_dude(std::vector<std::vector<int>> vec) {
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

inline std::string debug_vec(Extension& ext) {
  std::stringstream out;
  out << debug_vec(ext.extender);
  out << " {@extend ";
  out << debug_vec(ext.target);
  if (ext.isOptional) {
    out << " !optional";
  }
  out << "}";
  return out.str();
}

template <class T>
inline std::string debug_vec(std::vector<T> vec) {
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
    out << debug_vec(*it); // string (key)
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
  if (node == NULL) return "null";
  else return debug_vec(*node);
}

inline void debug_ast(const AST_Node* node, std::string ind = "", Env* env = 0) {
  debug_ast(const_cast<AST_Node*>(node), ind, env);
}

inline std::string str_replace(std::string str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
  return str;
}

inline std::string prettyprint(const std::string& str) {
  std::string clean = str_replace(str, "\n", "\\n");
  clean = str_replace(clean, "	", "\\t");
  clean = str_replace(clean, "\r", "\\r");
  return clean;
}

inline std::string longToHex(long long t) {
  std::stringstream is;
  is << std::hex << t;
  return is.str();
}

inline std::string pstate_source_position(AST_Node* node)
{
  std::stringstream str;
  Position start(node->pstate());
  Position end(start + node->pstate().offset);
  str << (start.file == std::string::npos ? 99999999 : start.file)
    << "@[" << start.line << ":" << start.column << "]"
    << "-[" << end.line << ":" << end.column << "]";
#ifdef DEBUG_SHARED_PTR
      str << "x" << node->getRefCount() << ""
      << " " << node->getDbgFile()
      << "@" << node->getDbgLine();
#endif
  return str.str();
}

inline void debug_ast(AST_Node* node, std::string ind, Env* env)
{
  if (node == 0) return;
  if (ind == "") std::clog << "####################################################################" << std::endl;
  if (Cast<Bubble>(node)) {
    Bubble* bubble = Cast<Bubble>(node);
    std::clog << ind << "Bubble " << bubble;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << bubble->tabs();
    std::clog << std::endl;
    debug_ast(bubble->node(), ind + " ", env);
  } else if (Cast<Trace>(node)) {
    Trace* trace = Cast<Trace>(node);
    std::clog << ind << "Trace " << trace;
    std::clog << " (" << pstate_source_position(node) << ")"
    << " [name:" << trace->name() << ", type: " << trace->type() << "]"
    << std::endl;
    debug_ast(trace->block(), ind + " ", env);
  } else if (Cast<At_Root_Block>(node)) {
    At_Root_Block* root_block = Cast<At_Root_Block>(node);
    std::clog << ind << "At_Root_Block " << root_block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << root_block->tabs();
    std::clog << std::endl;
    debug_ast(root_block->expression(), ind + ":", env);
    debug_ast(root_block->block(), ind + " ", env);
  } else if (Cast<SelectorList>(node)) {
    SelectorList* selector = Cast<SelectorList>(node);
    std::clog << ind << "SelectorList " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << (selector->is_invisible() ? " [is_invisible]" : " -");
    std::clog << (selector->isInvisible() ? " [isInvisible]" : " -");
    std::clog << (selector->has_real_parent_ref() ? " [real-parent]": " -");
    std::clog << std::endl;

    for(const ComplexSelector_Obj& i : selector->elements()) { debug_ast(i, ind + " ", env); }

  } else if (Cast<ComplexSelector>(node)) {
    ComplexSelector* selector = Cast<ComplexSelector>(node);
    std::clog << ind << "ComplexSelector " << selector
      << " (" << pstate_source_position(node) << ")"
      << " <" << selector->hash() << ">"
      << " [" << (selector->chroots() ? "CHROOT" : "CONNECT") << "]"
      << " [length:" << longToHex(selector->length()) << "]"
      << " [weight:" << longToHex(selector->specificity()) << "]"
      << (selector->is_invisible() ? " [is_invisible]" : " -")
      << (selector->isInvisible() ? " [isInvisible]" : " -")
      << (selector->hasPreLineFeed() ? " [hasPreLineFeed]" : " -")

      // << (selector->is_invisible() ? " [INVISIBLE]": " -")
      // << (selector->has_placeholder() ? " [PLACEHOLDER]": " -")
      // << (selector->is_optional() ? " [is_optional]": " -")
      << (selector->has_real_parent_ref() ? " [real parent]": " -")
      // << (selector->has_line_feed() ? " [line-feed]": " -")
      // << (selector->has_line_break() ? " [line-break]": " -")
      << " -- \n";

    for(const SelectorComponentObj& i : selector->elements()) { debug_ast(i, ind + " ", env); }

  } else if (Cast<SelectorCombinator>(node)) {
    SelectorCombinator* selector = Cast<SelectorCombinator>(node);
    std::clog << ind << "SelectorCombinator " << selector
      << " (" << pstate_source_position(node) << ")"
      << " <" << selector->hash() << ">"
      << " [weight:" << longToHex(selector->specificity()) << "]"
      << (selector->has_real_parent_ref() ? " [real parent]": " -")
      << " -- ";

      std::string del;
      switch (selector->combinator()) {
        case SelectorCombinator::CHILD:    del = ">"; break;
        case SelectorCombinator::GENERAL:  del = "~"; break;
        case SelectorCombinator::ADJACENT: del = "+"; break;
      }

      std::clog << "[" << del << "]" << std::endl;

  } else if (Cast<CompoundSelector>(node)) {
    CompoundSelector* selector = Cast<CompoundSelector>(node);
    std::clog << ind << "CompoundSelector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << (selector->hasRealParent() ? " [REAL PARENT]" : "") << ">";
    std::clog << " [weight:" << longToHex(selector->specificity()) << "]";
    std::clog << (selector->hasPostLineBreak() ? " [hasPostLineBreak]" : " -");
    std::clog << (selector->is_invisible() ? " [is_invisible]" : " -");
    std::clog << (selector->isInvisible() ? " [isInvisible]" : " -");
    std::clog << std::endl;
    for(const SimpleSelector_Obj& i : selector->elements()) { debug_ast(i, ind + " ", env); }

  } else if (Cast<Parent_Reference>(node)) {
    Parent_Reference* selector = Cast<Parent_Reference>(node);
    std::clog << ind << "Parent_Reference " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <" << prettyprint(selector->pstate().token.ws_before()) << ">" << std::endl;

  } else if (Cast<Pseudo_Selector>(node)) {
    Pseudo_Selector* selector = Cast<Pseudo_Selector>(node);
    std::clog << ind << "Pseudo_Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <<" << selector->ns_name() << ">>";
    std::clog << (selector->isClass() ? " [isClass]": " -");
    std::clog << (selector->isSyntacticClass() ? " [isSyntacticClass]": " -");
    std::clog << std::endl;
    debug_ast(selector->argument(), ind + " <= ", env);
    debug_ast(selector->selector(), ind + " || ", env);
  } else if (Cast<Attribute_Selector>(node)) {
    Attribute_Selector* selector = Cast<Attribute_Selector>(node);
    std::clog << ind << "Attribute_Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <<" << selector->ns_name() << ">>";
    std::clog << std::endl;
    debug_ast(selector->value(), ind + "[" + selector->matcher() + "] ", env);
  } else if (Cast<Class_Selector>(node)) {
    Class_Selector* selector = Cast<Class_Selector>(node);
    std::clog << ind << "Class_Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <<" << selector->ns_name() << ">>";
    std::clog << std::endl;
  } else if (Cast<Id_Selector>(node)) {
    Id_Selector* selector = Cast<Id_Selector>(node);
    std::clog << ind << "Id_Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <<" << selector->ns_name() << ">>";
    std::clog << std::endl;
  } else if (Cast<Type_Selector>(node)) {
    Type_Selector* selector = Cast<Type_Selector>(node);
    std::clog << ind << "Type_Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <" << selector->hash() << ">";
    std::clog << " <<" << selector->ns_name() << ">>";
    std::clog << " <" << prettyprint(selector->pstate().token.ws_before()) << ">";
    std::clog << std::endl;
  } else if (Cast<Placeholder_Selector>(node)) {

    Placeholder_Selector* selector = Cast<Placeholder_Selector>(node);
    std::clog << ind << "Placeholder_Selector [" << selector->ns_name() << "] " << selector;
    std::clog << " (" << pstate_source_position(selector) << ")"
      << " <" << selector->hash() << ">"
      << (selector->isInvisible() ? " [isInvisible]" : " -")
    << std::endl;

  } else if (Cast<SimpleSelector>(node)) {
    SimpleSelector* selector = Cast<SimpleSelector>(node);
    std::clog << ind << "SimpleSelector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")";

  } else if (Cast<Selector_Schema>(node)) {
    Selector_Schema* selector = Cast<Selector_Schema>(node);
    std::clog << ind << "Selector_Schema " << selector;
    std::clog << " (" << pstate_source_position(node) << ")"
      << (selector->connect_parent() ? " [connect-parent]": " -")
    << std::endl;

    debug_ast(selector->contents(), ind + " ");
    // for(auto i : selector->elements()) { debug_ast(i, ind + " ", env); }

  } else if (Cast<Selector>(node)) {
    Selector* selector = Cast<Selector>(node);
    std::clog << ind << "Selector " << selector;
    std::clog << " (" << pstate_source_position(node) << ")"
    << std::endl;

  } else if (Cast<Media_Query_Expression>(node)) {
    Media_Query_Expression* block = Cast<Media_Query_Expression>(node);
    std::clog << ind << "Media_Query_Expression " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << (block->is_interpolated() ? " [is_interpolated]": " -")
    << std::endl;
    debug_ast(block->feature(), ind + " feature) ");
    debug_ast(block->value(), ind + " value) ");

  } else if (Cast<Media_Query>(node)) {
    Media_Query* block = Cast<Media_Query>(node);
    std::clog << ind << "Media_Query " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << (block->is_negated() ? " [is_negated]": " -")
      << (block->is_restricted() ? " [is_restricted]": " -")
    << std::endl;
    debug_ast(block->media_type(), ind + " ");
    for(const auto& i : block->elements()) { debug_ast(i, ind + " ", env); }
  }
  else if (Cast<MediaRule>(node)) {
    MediaRule* rule = Cast<MediaRule>(node);
    std::clog << ind << "MediaRule " << rule;
    std::clog << " (" << pstate_source_position(rule) << ")";
    std::clog << " " << rule->tabs() << std::endl;
    debug_ast(rule->schema(), ind + " =@ ");
    debug_ast(rule->block(), ind + " ");
  }
  else if (Cast<CssMediaRule>(node)) {
    CssMediaRule* rule = Cast<CssMediaRule>(node);
    std::clog << ind << "CssMediaRule " << rule;
    std::clog << " (" << pstate_source_position(rule) << ")";
    std::clog << " " << rule->tabs() << std::endl;
    for (auto item : rule->elements()) {
      debug_ast(item, ind + " == ");
    }
    debug_ast(rule->block(), ind + " ");
  }
  else if (Cast<CssMediaQuery>(node)) {
    CssMediaQuery* query = Cast<CssMediaQuery>(node);
    std::clog << ind << "CssMediaQuery " << query;
    std::clog << " (" << pstate_source_position(query) << ")";
    std::clog << " [" << (query->modifier()) << "] ";
    std::clog << " [" << (query->type()) << "] ";
    std::clog << " " << debug_vec(query->features());
    std::clog << std::endl;
  } else if (Cast<Supports_Block>(node)) {
    Supports_Block* block = Cast<Supports_Block>(node);
    std::clog << ind << "Supports_Block " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->condition(), ind + " =@ ");
    debug_ast(block->block(), ind + " <>");
  } else if (Cast<Supports_Operator>(node)) {
    Supports_Operator* block = Cast<Supports_Operator>(node);
    std::clog << ind << "Supports_Operator " << block;
    std::clog << " (" << pstate_source_position(node) << ")"
    << std::endl;
    debug_ast(block->left(), ind + " left) ");
    debug_ast(block->right(), ind + " right) ");
  } else if (Cast<Supports_Negation>(node)) {
    Supports_Negation* block = Cast<Supports_Negation>(node);
    std::clog << ind << "Supports_Negation " << block;
    std::clog << " (" << pstate_source_position(node) << ")"
    << std::endl;
    debug_ast(block->condition(), ind + " condition) ");
  } else if (Cast<At_Root_Query>(node)) {
    At_Root_Query* block = Cast<At_Root_Query>(node);
    std::clog << ind << "At_Root_Query " << block;
    std::clog << " (" << pstate_source_position(node) << ")"
    << std::endl;
    debug_ast(block->feature(), ind + " feature) ");
    debug_ast(block->value(), ind + " value) ");
  } else if (Cast<Supports_Declaration>(node)) {
    Supports_Declaration* block = Cast<Supports_Declaration>(node);
    std::clog << ind << "Supports_Declaration " << block;
    std::clog << " (" << pstate_source_position(node) << ")"
    << std::endl;
    debug_ast(block->feature(), ind + " feature) ");
    debug_ast(block->value(), ind + " value) ");
  } else if (Cast<Block>(node)) {
    Block* root_block = Cast<Block>(node);
    std::clog << ind << "Block " << root_block;
    std::clog << " (" << pstate_source_position(node) << ")";
    if (root_block->is_root()) std::clog << " [root]";
    if (root_block->isInvisible()) std::clog << " [isInvisible]";
    std::clog << " " << root_block->tabs() << std::endl;
    for(const Statement_Obj& i : root_block->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Warning>(node)) {
    Warning* block = Cast<Warning>(node);
    std::clog << ind << "Warning " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->message(), ind + " : ");
  } else if (Cast<Error>(node)) {
    Error* block = Cast<Error>(node);
    std::clog << ind << "Error " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
  } else if (Cast<Debug>(node)) {
    Debug* block = Cast<Debug>(node);
    std::clog << ind << "Debug " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->value(), ind + " ");
  } else if (Cast<Comment>(node)) {
    Comment* block = Cast<Comment>(node);
    std::clog << ind << "Comment " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() <<
      " <" << prettyprint(block->pstate().token.ws_before()) << ">" << std::endl;
    debug_ast(block->text(), ind + "// ", env);
  } else if (Cast<If>(node)) {
    If* block = Cast<If>(node);
    std::clog << ind << "If " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->predicate(), ind + " = ");
    debug_ast(block->block(), ind + " <>");
    debug_ast(block->alternative(), ind + " ><");
  } else if (Cast<Return>(node)) {
    Return* block = Cast<Return>(node);
    std::clog << ind << "Return " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs();
    std::clog << " [" << block->value()->to_string() << "]" << std::endl;
  } else if (Cast<ExtendRule>(node)) {
    ExtendRule* block = Cast<ExtendRule>(node);
    std::clog << ind << "ExtendRule " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->selector(), ind + "-> ", env);
  } else if (Cast<Content>(node)) {
    Content* block = Cast<Content>(node);
    std::clog << ind << "Content " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->arguments(), ind + " args: ", env);
  } else if (Cast<Import_Stub>(node)) {
    Import_Stub* block = Cast<Import_Stub>(node);
    std::clog << ind << "Import_Stub " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << block->imp_path() << "] ";
    std::clog << " " << block->tabs() << std::endl;
  } else if (Cast<Import>(node)) {
    Import* block = Cast<Import>(node);
    std::clog << ind << "Import " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    // std::vector<std::string>         files_;
    for (auto imp : block->urls()) debug_ast(imp, ind + "@: ", env);
    debug_ast(block->import_queries(), ind + "@@ ");
  } else if (Cast<Assignment>(node)) {
    Assignment* block = Cast<Assignment>(node);
    std::clog << ind << "Assignment " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " <<" << block->variable() << ">> " << block->tabs() << std::endl;
    debug_ast(block->value(), ind + "=", env);
  } else if (Cast<Declaration>(node)) {
    Declaration* block = Cast<Declaration>(node);
    std::clog << ind << "Declaration " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [is_custom_property: " << block->is_custom_property() << "] ";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->property(), ind + " prop: ", env);
    debug_ast(block->value(), ind + " value: ", env);
    debug_ast(block->block(), ind + " ", env);
  } else if (Cast<Keyframe_Rule>(node)) {
    Keyframe_Rule* has_block = Cast<Keyframe_Rule>(node);
    std::clog << ind << "Keyframe_Rule " << has_block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << has_block->tabs() << std::endl;
    if (has_block->name()) debug_ast(has_block->name(), ind + "@");
    if (has_block->block()) for(const Statement_Obj& i : has_block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Directive>(node)) {
    Directive* block = Cast<Directive>(node);
    std::clog << ind << "Directive " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << block->keyword() << "] " << block->tabs() << std::endl;
    debug_ast(block->selector(), ind + "~", env);
    debug_ast(block->value(), ind + "+", env);
    if (block->block()) for(const Statement_Obj& i : block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Each>(node)) {
    Each* block = Cast<Each>(node);
    std::clog << ind << "Each " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    if (block->block()) for(const Statement_Obj& i : block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<For>(node)) {
    For* block = Cast<For>(node);
    std::clog << ind << "For " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    if (block->block()) for(const Statement_Obj& i : block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<While>(node)) {
    While* block = Cast<While>(node);
    std::clog << ind << "While " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << block->tabs() << std::endl;
    if (block->block()) for(const Statement_Obj& i : block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Definition>(node)) {
    Definition* block = Cast<Definition>(node);
    std::clog << ind << "Definition " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [name: " << block->name() << "] ";
    std::clog << " [type: " << (block->type() == Sass::Definition::Type::MIXIN ? "Mixin " : "Function ") << "] ";
    // this seems to lead to segfaults some times?
    // std::clog << " [signature: " << block->signature() << "] ";
    std::clog << " [native: " << block->native_function() << "] ";
    std::clog << " " << block->tabs() << std::endl;
    debug_ast(block->parameters(), ind + " params: ", env);
    if (block->block()) debug_ast(block->block(), ind + " ", env);
  } else if (Cast<Mixin_Call>(node)) {
    Mixin_Call* block = Cast<Mixin_Call>(node);
    std::clog << ind << "Mixin_Call " << block << " " << block->tabs();
    std::clog << " (" << pstate_source_position(block) << ")";
    std::clog << " [" <<  block->name() << "]";
    std::clog << " [has_content: " << block->has_content() << "] " << std::endl;
    debug_ast(block->arguments(), ind + " args: ", env);
    debug_ast(block->block_parameters(), ind + " block_params: ", env);
    if (block->block()) debug_ast(block->block(), ind + " ", env);
  } else if (Ruleset* ruleset = Cast<Ruleset>(node)) {
    std::clog << ind << "Ruleset " << ruleset;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [indent: " << ruleset->tabs() << "]";
    std::clog << (ruleset->is_invisible() ? " [INVISIBLE]" : "");
    std::clog << (ruleset->is_root() ? " [root]" : "");
    std::clog << std::endl;
    debug_ast(ruleset->selector(), ind + ">");
    debug_ast(ruleset->block(), ind + " ");
  } else if (Cast<Block>(node)) {
    Block* block = Cast<Block>(node);
    std::clog << ind << "Block " << block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << (block->is_invisible() ? " [INVISIBLE]" : "");
    std::clog << " [indent: " << block->tabs() << "]" << std::endl;
    for(const Statement_Obj& i : block->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Variable>(node)) {
    Variable* expression = Cast<Variable>(node);
    std::clog << ind << "Variable " << expression;
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << expression->name() << "]" << std::endl;
    std::string name(expression->name());
    if (env && env->has(name)) debug_ast(Cast<Expression>((*env)[name]), ind + " -> ", env);
  } else if (Cast<Function_Call>(node)) {
    Function_Call* expression = Cast<Function_Call>(node);
    std::clog << ind << "Function_Call " << expression;
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << expression->name() << "]";
    if (expression->is_delayed()) std::clog << " [delayed]";
    if (expression->is_interpolant()) std::clog << " [interpolant]";
    if (expression->is_css()) std::clog << " [css]";
    std::clog << std::endl;
    debug_ast(expression->arguments(), ind + " args: ", env);
    debug_ast(expression->func(), ind + " func: ", env);
  } else if (Cast<Function>(node)) {
    Function* expression = Cast<Function>(node);
    std::clog << ind << "Function " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    if (expression->is_css()) std::clog << " [css]";
    std::clog << std::endl;
    debug_ast(expression->definition(), ind + " definition: ", env);
  } else if (Cast<Arguments>(node)) {
    Arguments* expression = Cast<Arguments>(node);
    std::clog << ind << "Arguments " << expression;
    if (expression->is_delayed()) std::clog << " [delayed]";
    std::clog << " (" << pstate_source_position(node) << ")";
    if (expression->has_named_arguments()) std::clog << " [has_named_arguments]";
    if (expression->has_rest_argument()) std::clog << " [has_rest_argument]";
    if (expression->has_keyword_argument()) std::clog << " [has_keyword_argument]";
    std::clog << std::endl;
    for(const Argument_Obj& i : expression->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Argument>(node)) {
    Argument* expression = Cast<Argument>(node);
    std::clog << ind << "Argument " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << expression->value().ptr() << "]";
    std::clog << " [name: " << expression->name() << "] ";
    std::clog << " [rest: " << expression->is_rest_argument() << "] ";
    std::clog << " [keyword: " << expression->is_keyword_argument() << "] " << std::endl;
    debug_ast(expression->value(), ind + " value: ", env);
  } else if (Cast<Parameters>(node)) {
    Parameters* expression = Cast<Parameters>(node);
    std::clog << ind << "Parameters " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [has_optional: " << expression->has_optional_parameters() << "] ";
    std::clog << " [has_rest: " << expression->has_rest_parameter() << "] ";
    std::clog << std::endl;
    for(const Parameter_Obj& i : expression->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Parameter>(node)) {
    Parameter* expression = Cast<Parameter>(node);
    std::clog << ind << "Parameter " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [name: " << expression->name() << "] ";
    std::clog << " [default: " << expression->default_value().ptr() << "] ";
    std::clog << " [rest: " << expression->is_rest_parameter() << "] " << std::endl;
  } else if (Cast<Unary_Expression>(node)) {
    Unary_Expression* expression = Cast<Unary_Expression>(node);
    std::clog << ind << "Unary_Expression " << expression;
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " [delayed: " << expression->is_delayed() << "] ";
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << expression->type() << "]" << std::endl;
    debug_ast(expression->operand(), ind + " operand: ", env);
  } else if (Cast<Binary_Expression>(node)) {
    Binary_Expression* expression = Cast<Binary_Expression>(node);
    std::clog << ind << "Binary_Expression " << expression;
    if (expression->is_interpolant()) std::clog << " [is interpolant] ";
    if (expression->is_left_interpolant()) std::clog << " [left interpolant] ";
    if (expression->is_right_interpolant()) std::clog << " [right interpolant] ";
    std::clog << " [delayed: " << expression->is_delayed() << "] ";
    std::clog << " [ws_before: " << expression->op().ws_before << "] ";
    std::clog << " [ws_after: " << expression->op().ws_after << "] ";
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << expression->type_name() << "]" << std::endl;
    debug_ast(expression->left(), ind + " left:  ", env);
    debug_ast(expression->right(), ind + " right: ", env);
  } else if (Cast<Map>(node)) {
    Map* expression = Cast<Map>(node);
    std::clog << ind << "Map " << expression;
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [Hashed]" << std::endl;
    for (const auto& i : expression->elements()) {
      debug_ast(i.first, ind + " key: ");
      debug_ast(i.second, ind + " val: ");
    }
  } else if (Cast<List>(node)) {
    List* expression = Cast<List>(node);
    std::clog << ind << "List " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " (" << expression->length() << ") " <<
      (expression->separator() == SASS_COMMA ? "Comma " : expression->separator() == SASS_HASH ? "Map " : "Space ") <<
      " [delayed: " << expression->is_delayed() << "] " <<
      " [interpolant: " << expression->is_interpolant() << "] " <<
      " [listized: " << expression->from_selector() << "] " <<
      " [arglist: " << expression->is_arglist() << "] " <<
      " [bracketed: " << expression->is_bracketed() << "] " <<
      " [expanded: " << expression->is_expanded() << "] " <<
      " [hash: " << expression->hash() << "] " <<
      std::endl;
    for(const auto& i : expression->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Boolean>(node)) {
    Boolean* expression = Cast<Boolean>(node);
    std::clog << ind << "Boolean " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " [" << expression->value() << "]" << std::endl;
  } else if (Cast<Color_RGBA>(node)) {
    Color_RGBA* expression = Cast<Color_RGBA>(node);
    std::clog << ind << "Color " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [name: " << expression->disp() << "] ";
    std::clog << " [delayed: " << expression->is_delayed() << "] ";
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " rgba[" << expression->r() << ":"  << expression->g() << ":" << expression->b() << "@" << expression->a() << "]" << std::endl;
  } else if (Cast<Color_HSLA>(node)) {
    Color_HSLA* expression = Cast<Color_HSLA>(node);
    std::clog << ind << "Color " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [name: " << expression->disp() << "] ";
    std::clog << " [delayed: " << expression->is_delayed() << "] ";
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " hsla[" << expression->h() << ":"  << expression->s() << ":" << expression->l() << "@" << expression->a() << "]" << std::endl;
  } else if (Cast<Number>(node)) {
    Number* expression = Cast<Number>(node);
    std::clog << ind << "Number " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [delayed: " << expression->is_delayed() << "] ";
    std::clog << " [interpolant: " << expression->is_interpolant() << "] ";
    std::clog << " [" << expression->value() << expression->unit() << "]" <<
      " [hash: " << expression->hash() << "] " <<
      std::endl;
  } else if (Cast<Null>(node)) {
    Null* expression = Cast<Null>(node);
    std::clog << ind << "Null " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [interpolant: " << expression->is_interpolant() << "] "
      // " [hash: " << expression->hash() << "] "
      << std::endl;
  } else if (Cast<String_Quoted>(node)) {
    String_Quoted* expression = Cast<String_Quoted>(node);
    std::clog << ind << "String_Quoted " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << prettyprint(expression->value()) << "]";
    if (expression->is_delayed()) std::clog << " [delayed]";
    if (expression->is_interpolant()) std::clog << " [interpolant]";
    if (expression->quote_mark()) std::clog << " [quote_mark: " << expression->quote_mark() << "]";
    std::clog << " <" << prettyprint(expression->pstate().token.ws_before()) << ">" << std::endl;
  } else if (Cast<String_Constant>(node)) {
    String_Constant* expression = Cast<String_Constant>(node);
    std::clog << ind << "String_Constant " << expression;
    if (expression->concrete_type()) {
      std::clog << " " << expression->concrete_type();
    }
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " [" << prettyprint(expression->value()) << "]";
    if (expression->is_delayed()) std::clog << " [delayed]";
    if (expression->is_interpolant()) std::clog << " [interpolant]";
    std::clog << " <" << prettyprint(expression->pstate().token.ws_before()) << ">" << std::endl;
  } else if (Cast<String_Schema>(node)) {
    String_Schema* expression = Cast<String_Schema>(node);
    std::clog << ind << "String_Schema " << expression;
    std::clog << " (" << pstate_source_position(expression) << ")";
    std::clog << " " << expression->concrete_type();
    std::clog << " (" << pstate_source_position(node) << ")";
    if (expression->css()) std::clog << " [css]";
    if (expression->is_delayed()) std::clog << " [delayed]";
    if (expression->is_interpolant()) std::clog << " [is interpolant]";
    if (expression->has_interpolant()) std::clog << " [has interpolant]";
    if (expression->is_left_interpolant()) std::clog << " [left interpolant] ";
    if (expression->is_right_interpolant()) std::clog << " [right interpolant] ";
    std::clog << " <" << prettyprint(expression->pstate().token.ws_before()) << ">" << std::endl;
    for(const auto& i : expression->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<String>(node)) {
    String* expression = Cast<String>(node);
    std::clog << ind << "String " << expression;
    std::clog << " " << expression->concrete_type();
    std::clog << " (" << pstate_source_position(node) << ")";
    if (expression->is_interpolant()) std::clog << " [interpolant]";
    std::clog << " <" << prettyprint(expression->pstate().token.ws_before()) << ">" << std::endl;
  } else if (Cast<Expression>(node)) {
    Expression* expression = Cast<Expression>(node);
    std::clog << ind << "Expression " << expression;
    std::clog << " (" << pstate_source_position(node) << ")";
    switch (expression->concrete_type()) {
      case Expression::Type::NONE: std::clog << " [NONE]"; break;
      case Expression::Type::BOOLEAN: std::clog << " [BOOLEAN]"; break;
      case Expression::Type::NUMBER: std::clog << " [NUMBER]"; break;
      case Expression::Type::COLOR: std::clog << " [COLOR]"; break;
      case Expression::Type::STRING: std::clog << " [STRING]"; break;
      case Expression::Type::LIST: std::clog << " [LIST]"; break;
      case Expression::Type::MAP: std::clog << " [MAP]"; break;
      case Expression::Type::SELECTOR: std::clog << " [SELECTOR]"; break;
      case Expression::Type::NULL_VAL: std::clog << " [NULL_VAL]"; break;
      case Expression::Type::C_WARNING: std::clog << " [C_WARNING]"; break;
      case Expression::Type::C_ERROR: std::clog << " [C_ERROR]"; break;
      case Expression::Type::FUNCTION: std::clog << " [FUNCTION]"; break;
      case Expression::Type::NUM_TYPES: std::clog << " [NUM_TYPES]"; break;
      case Expression::Type::VARIABLE: std::clog << " [VARIABLE]"; break;
      case Expression::Type::FUNCTION_VAL: std::clog << " [FUNCTION_VAL]"; break;
      case Expression::Type::PARENT: std::clog << " [PARENT]"; break;
    }
    std::clog << std::endl;
  } else if (Cast<Has_Block>(node)) {
    Has_Block* has_block = Cast<Has_Block>(node);
    std::clog << ind << "Has_Block " << has_block;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << has_block->tabs() << std::endl;
    if (has_block->block()) for(const Statement_Obj& i : has_block->block()->elements()) { debug_ast(i, ind + " ", env); }
  } else if (Cast<Statement>(node)) {
    Statement* statement = Cast<Statement>(node);
    std::clog << ind << "Statement " << statement;
    std::clog << " (" << pstate_source_position(node) << ")";
    std::clog << " " << statement->tabs() << std::endl;
  }

  if (ind == "") std::clog << "####################################################################" << std::endl;
}


/*
inline void debug_ast(const AST_Node* node, std::string ind = "", Env* env = 0)
{
  debug_ast(const_cast<AST_Node*>(node), ind, env);
}
*/

#endif // SASS_DEBUGGER

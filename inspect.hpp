#ifndef SASS_INSPECT_H
#define SASS_INSPECT_H

#include <string>

#include "context.hpp"
#include "position.hpp"
#include "operation.hpp"
#include "emitter.hpp"

namespace Sass {
  using namespace std;
  struct Context;

  class Inspect : public Operation_CRTP<void, Inspect>, public Emitter {
  protected:
    // import all the class-specific methods and override as desired
    using Operation_CRTP<void, Inspect>::operator();

    void fallback_impl(AST_Node* n);
    bool in_argument;
    bool in_wrapped;
    bool is_output;

  public:
    bool disable_quotes;

    Inspect(Emitter emi, bool output = false);
    virtual ~Inspect();

    // statements
    virtual void operator()(Block*);
    virtual void operator()(Ruleset*);
    virtual void operator()(Propset*);
    virtual void operator()(Bubble*);
    virtual void operator()(Feature_Block*);
    virtual void operator()(Media_Block*);
    virtual void operator()(At_Root_Block*);
    virtual void operator()(At_Rule*);
    virtual void operator()(Keyframe_Rule*);
    virtual void operator()(Declaration*);
    virtual void operator()(Assignment*);
    virtual void operator()(Import*);
    virtual void operator()(Import_Stub*);
    virtual void operator()(Warning*);
    virtual void operator()(Error*);
    virtual void operator()(Debug*);
    virtual void operator()(Comment*);
    virtual void operator()(If*);
    virtual void operator()(For*);
    virtual void operator()(Each*);
    virtual void operator()(While*);
    virtual void operator()(Return*);
    virtual void operator()(Extension*);
    virtual void operator()(Definition*);
    virtual void operator()(Mixin_Call*);
    virtual void operator()(Content*);
    // expressions
    virtual void operator()(Map*);
    virtual void operator()(List*);
    virtual void operator()(Binary_Expression*);
    virtual void operator()(Unary_Expression*);
    virtual void operator()(Function_Call*);
    virtual void operator()(Function_Call_Schema*);
    virtual void operator()(Variable*);
    virtual void operator()(Textual*);
    virtual void operator()(Number*);
    virtual void operator()(Color*);
    virtual void operator()(Boolean*);
    virtual void operator()(String_Schema*);
    virtual void operator()(String_Constant*);
    virtual void operator()(String_Quoted*);
    virtual void operator()(Feature_Query*);
    virtual void operator()(Feature_Query_Condition*);
    virtual void operator()(Media_Query*);
    virtual void operator()(Media_Query_Expression*);
    virtual void operator()(At_Root_Expression*);
    virtual void operator()(Null*);
    // parameters and arguments
    virtual void operator()(Parameter*);
    virtual void operator()(Parameters*);
    virtual void operator()(Argument*);
    virtual void operator()(Arguments*);
    // selectors
    virtual void operator()(Selector_Schema*);
    virtual void operator()(Selector_Reference*);
    virtual void operator()(Selector_Placeholder*);
    virtual void operator()(Type_Selector*);
    virtual void operator()(Selector_Qualifier*);
    virtual void operator()(Attribute_Selector*);
    virtual void operator()(Pseudo_Selector*);
    virtual void operator()(Wrapped_Selector*);
    virtual void operator()(Compound_Selector*);
    virtual void operator()(Complex_Selector*);
    virtual void operator()(Selector_List*);

    template <typename U>
    void fallback(U x) { fallback_impl(reinterpret_cast<AST_Node*>(x)); }
  };

  string unquote(const string&, char* q = 0);
  string quote(const string&, char);

}
#endif

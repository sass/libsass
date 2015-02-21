#ifndef SASS_OUTPUT_H
#define SASS_OUTPUT_H

#include <string>
#include <vector>

#include "util.hpp"
#include "inspect.hpp"
#include "context.hpp"
#include "operation.hpp"

namespace Sass {
  using namespace std;
  using namespace Sass;
  struct Context;

  inline bool ends_with(std::string const & value, std::string const & ending)
  {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  class Output : public Inspect {
  protected:
    using Inspect::operator();

  public:
    Output(OutputBuffer& buf, Context* ctx = 0);
    virtual ~Output();

  protected:
    Context* ctx;
    vector<Import*> top_imports;
    vector<Comment*> top_comments;
    bool source_comments;
  protected:
    // indentation level
    // size_t indentation;
    // internal state
    bool in_directive;
    bool in_keyframes;

  public:
    virtual void operator()(Import* imp);
    virtual void operator()(Comment* c);
    void fallback_impl(AST_Node* n);
    string get_buffer(void);

    virtual void operator()(Ruleset*);
    // virtual void operator()(Propset*);
    virtual void operator()(Feature_Block*);
    virtual void operator()(Media_Block*);
    virtual void operator()(At_Rule*);
    virtual void operator()(Keyframe_Rule*);
    virtual void operator()(String_Quoted*);
    virtual void operator()(String_Constant*);


  };

}

#endif

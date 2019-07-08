/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CSSIZE_HPP
#define SASS_CSSIZE_HPP

#include "inspect.hpp"

namespace Sass {

  class Cssize : public Inspect {
  public:

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    Cssize(
      SassOutputOptionsCpp& opt,
      bool srcmap_enabled) :
      Inspect(opt, srcmap_enabled)
    {}

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    virtual void visitFunction(Function*) override;
    virtual void visitNumber(Number*) override;
    virtual void visitList(List*) override;
    virtual void visitMap(Map*) override;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  };

}

#endif

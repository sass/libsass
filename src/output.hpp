/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_OUTPUT_HPP
#define SASS_OUTPUT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "cssize.hpp"

namespace Sass {

  // Output is responsible to hoist imports and comments correctly.
  // We want comments occurring before an import to be hoisted with it.
  // Also applies to rules hoisted down due to imports being hoisted up.
  // We want to preserve comments occurring before important blocks.
  class Output : public Cssize
  {
  protected:

    // The parsed charset
    sass::string charset;

    // Hoist imports and their comments in the whole document.
    // This shuffles imports in the middle of a file to the top.
    CssNodeVector imports;

    // Flag if comments should be queued for hoisting.
    // This is typically only allowed on the root level.
    bool hoistComments = true;

    // The comments we queued up so far
    // Flush/process once we know what to do
    sass::vector<CssCommentObj> comments;

    // Helper function to append comment to output
    void printCssComment(CssComment* comment);

    // Flushes all queued comments to output
    // Also resets and clears the queue
    void flushCssComments();

  public:

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Value constructor
    Output(OutputOptions& opt);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Return buffer to compiler
    OutputBuffer getBuffer(void);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    virtual void visitCssImport(CssImport*) override;
    virtual void visitCssComment(CssComment*) override;
    virtual void visitCssMediaRule(CssMediaRule*) override;

    virtual void visitCssAtRule(CssAtRule*) override;
    virtual void visitCssStyleRule(CssStyleRule*) override;
    virtual void visitCssSupportsRule(CssSupportsRule*) override;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    virtual void visitMap(Map* value) override;
    virtual void visitString(String* value) override;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  };

}

#endif

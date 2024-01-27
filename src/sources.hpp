/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SOURCES_HPP
#define SASS_SOURCES_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "source.hpp"
#include "source_span.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Base class for our two main implementations.
  // The main API is `const char*` based.
  /////////////////////////////////////////////////////////////////////////
  class SourceWithPath : public SourceData
  {
  protected:
    
    // Import path
    sass::string imp_path;

    // Resolved path
    sass::string abs_path;

    // Raw length in bytes
    size_t len_content;
    size_t len_srcmaps;

    // Unique source id
    size_t srcidx;

    // Store byte offset for every line.
    // Lazy calculated within `countLines`.
    // Columns per line can be derived from it.
    sass::vector<size_t> lfs;

    // Returns the number of lines. On first call
    // it will calculate the linefeed lookup table.
    virtual size_t countLines() override;

  public:

    // Value move constructor
    SourceWithPath(
      sass::string&& imp_path,
      sass::string&& abs_path,
      size_t idx = sass::string::npos);

    // Value copy constructor
    SourceWithPath(
      const sass::string& imp_path,
      const sass::string& abs_path,
      size_t idx = sass::string::npos);

    // Returns the requested line. Will take interpolations into
    // account to show more accurate debug messages. Calling this
    // can be rather expensive, so only use it for debugging.
    virtual sass::string getLine(size_t line) override;

    // Return path as it was given for import
    const char* getImpPath() const override final
    {
      return imp_path.empty() ?
        nullptr : imp_path.c_str();
    }

    // Return path after it was resolved
    const char* getAbsPath() const override final
    {
      return abs_path.empty() ?
        nullptr : abs_path.c_str();
    }

    // The source id is uniquely assigned
    void setSrcIdx(size_t idx) override final
    {
      srcidx = idx;
    }

    // The source id is uniquely assigned
    size_t getSrcIdx() const override final
    {
      return srcidx;
    }

    size_t contentSize() const override final
    {
      return len_content;
    }

    size_t srcmapsSize() const override final
    {
      return len_srcmaps;
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // A SourceFile is meant to be used for externally loaded resource.
  // The resources passed in will be taken over and disposed at the end.
  // Resources must have been allocated via `sass_alloc_memory`.
  /////////////////////////////////////////////////////////////////////////
  class SourceFile final : public SourceWithPath
  {
  protected:

    // Raw source data
    char* _content;

    // Raw source data
    char* _srcmaps;

  public:

    // Value copy/move constructor
    // Copied: imp_path and abs_path
    // Moved: content and srcmaps data
    SourceFile(
      const char* imp_path, // copy
      const char* abs_path, // copy
      char* content, // take ownership
      char* srcmaps, // take ownership
      size_t srcidx = sass::string::npos);

    // Destructor
    ~SourceFile() override final;

    // Get raw iterator for actual source
    const char* content() const override final {
      return _content;
    }

    // Get raw iterator for actual source
    const char* srcmaps() const override final {
      return _srcmaps;
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // A SourceString is meant to be used internally when we need to
  // re-parse evaluated interpolations or static function signatures.
  /////////////////////////////////////////////////////////////////////////
  class SourceString :
    public SourceWithPath {

  protected:

    // Raw source data
    sass::string _content;

    // Raw source data
    sass::string _srcmaps;

  public:

    // Value move constructor without srcmaps
    // ToDo: should we try to parse srcmaps?
    SourceString(
      const char* path,
      sass::string&& data);

    // Value move constructor with srcmaps
    SourceString(
      const char* imp_path,
      const char* abs_path,
      sass::string&& data,
      sass::string&& srcmap,
      size_t srcidx = sass::string::npos);

    // Get raw iterator for actual source
    const char* content() const override final {
      return _content.c_str();
    }

    // Get raw iterator for actual source
    const char* srcmaps() const override final {
      return _srcmaps.c_str();
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // This class helps to report more meaningful errors when interpolations
  // are involved. We basically replace the original interpolation with the
  // result after evaluation. We can also adjust your parser state, since we
  // often only re-parse the partial interpolated object (e.g. selector in
  // the middle of a document). The error will be relative to this snippet.
  // E.g. on line 1, after adjusting it should be in sync with whatever the
  // `getLine` API returns. We do all this only on demand, since this is quite
  // expensive, so this is only intended to be used in error/debug cases!!
  /////////////////////////////////////////////////////////////////////////
  class SourceItpl :
    public SourceString {

  protected:

    // Account additional lines if needed.
    size_t countLines() override final;

  private:

    // The position where the interpolation occurred.
    // We also get the parent source from this state.
    // Plus the parent `SourceString` we have it all.
    SourceSpan pstate;

  public:

    // Create a synthetic interpolated source. The `data` is the
    // evaluated interpolation, while `around` is the original source
    // where the actual interpolation was given at `pstate` position.
    SourceItpl(SourceSpan pstate, sass::string&& data);

    // Returns source with this interpolation inserted.
    sass::string getLine(size_t line) override final;

    // Returns adjusted source span with interpolation in mind.
    // The input `pstate` is relative to the interpolation, will
    // return a source span with absolute position in regard of
    // the original document with the interpolation inserted.
    SourceSpan adjustSourceSpan(SourceSpan& pstate) const override final;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif

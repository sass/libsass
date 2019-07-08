#ifndef SASS_SOURCE_MAP_HPP
#define SASS_SOURCE_MAP_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"
#include "source_span.hpp"
#include "backtrace.hpp"
#include "base64vlq.hpp"
#include "mapping.hpp"

#define VECTOR_PUSH(vec, ins) vec.insert(vec.end(), ins.begin(), ins.end())
#define VECTOR_UNSHIFT(vec, ins) vec.insert(vec.begin(), ins.begin(), ins.end())

namespace Sass {

  class Context;
  class OutputBuffer;

  class SourceMap {

  public:
    sass::vector<size_t> source_index;
    SourceMap();
    // SourceMap(const sass::string& file);

    void append(const Offset& offset);
    void prepend(const Offset& offset);
    void append(const OutputBuffer& out);
    void prepend(const OutputBuffer& out);
    void add_open_mapping(const AstNode* node);
    void add_close_mapping(const AstNode* node);

    SourceSpan remap(const SourceSpan& pstate);

  private:

  public:
    sass::string render(const std::unordered_map<size_t, size_t>& remap_srcidx) const;

    // Deque is not faster, I checked
    sass::vector<Mapping> mappings;

    void reserve(size_t size) {
      source_index.reserve(size);
      mappings.reserve(size);
    }

  private:
    Offset current_position;
public:
    sass::string file;
private:
    Base64VLQ base64vlq;
  };

  class OutputBuffer {
  private:
    // Make sure we don't allow any copies
    OutputBuffer(const OutputBuffer&) = delete;
    OutputBuffer& operator=(const OutputBuffer&) = delete;
  public:

    // Allow to move the buffer
    OutputBuffer(OutputBuffer&&) noexcept;
    // The main buffer string
    sass::string buffer;
    // The optional source map
    SourceMap* smap;
    OutputBuffer(bool enabled)
      : smap()
    {
      if (enabled) smap = new SourceMap();
    }
  };

}

#endif

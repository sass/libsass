/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "source_map.hpp"

#include "source.hpp"
#include "ast_nodes.hpp"

/////////////////////////////////////////////////////////////////////////
// Most UAs don't allow very detailed navigation, although LibSass
// is able to give much more detailed information about where e.g.
// certain values from a long-hand property come from. Consider the
// longhand-property `border: 1px solid red`. Currently the best UA
// seems to be anything WebKit based (like Chrome). There you can
// actually reach the source (by clicking on it while holding the
// ctrl key) for `border` and `1px solid red`. We could actually
// give the source position for each value, e.g. `1px` or `red`
// individually, but currently no UA seems to supports this. Same
// applies for complex selector, where we could give the source for
// every compound selector (basically for every word token).
/////////////////////////////////////////////////////////////////////////
// Implementation of Source-Maps in UAs also have another flaw (IMO).
// It seems UAs work actively against us from embedding more detailed
// information, which could be interesting for re-mapping operations,
// e.g. when files flow through multiple processors. Consider the
// selector `foo bar baz`. Since UAs can at best only navigate to one
// certain source position, we have to decide where this should be.
// Naturally we probably want it to point the most inner block. To
// illustrate consider the following sass code (with marked position):
// `[A]foo { [B]bar { [C]baz { ... }}}`. Rendered with source positions
// this might look like this: `[A]foo [B]bar [C]baz { ... }`. In case
// we render the results like this, UAs will link the final selector
// to the most outer block [A], which is quite useless for Sass files,
// since you will probably have one big block there. Now you might
// think we could just fiddle with it a little to make it look like 
// `[C][A]foo [B]bar [C]baz`. Unfortunately this doesn't improve
// the situation, as the only way this will work correctly is
// `[A][C]foo [B]bar [C]baz`. Although not unsolvable, it really
// is pretty `out of order` and needs some dirty flags to work
// around in the code-base Well, it is what it is :-/
/////////////////////////////////////////////////////////////////////////
// Since I couldn't decide how to proceed, I decided to try to
// offer all possible direction one could like to take this. Not
// yet sure if we will ever expose this directly on the C-API side.
// I basically see a few different desirable cases:
// - Minimum payload to just make it work in UAs
// - More detailed version including crutch to fix UAs
// - Fully detailed version without crutch included
/////////////////////////////////////////////////////////////////////////
namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Empty constructor
  SourceMap::SourceMap() :
    position()
  {}

  // Call when text in the original was appended
  void SourceMap::append(const Offset& offset)
  {
    position += offset;
  }
  // EO append

  // Call when text in the original was prepended
  void SourceMap::prepend(const Offset& offset)
  {
    if (offset.line != 0 || offset.column != 0) {
      for (Mapping& mapping : mappings) {
        // move stuff on the first old line
        if (mapping.target.line == 0) {
          mapping.target.column += offset.column;
        }
        // make place for the new lines
        mapping.target.line += offset.line;
      }
    }
    if (position.line == 0) {
      position.column += offset.column;
    }
    position.line += offset.line;
  }
  // EO prepend

  // Call when another buffer is appended to the original
  void SourceMap::append(const OutputBuffer& out)
  {
    append(Offset(out.buffer));
  }
  // EO append

  // Call when another buffer is prepended to the original
  void SourceMap::prepend(const OutputBuffer& out)
  {
    if (out.srcmap) {
      Offset size(out.srcmap->position);
      for (const Mapping& mapping : out.srcmap->mappings) {
        if (mapping.target.line > size.line) {
          throw(std::runtime_error("prepend sourcemap has illegal line"));
        }
        if (mapping.target.line == size.line) {
          if (mapping.target.column > size.column) {
            throw(std::runtime_error("prepend sourcemap has illegal column"));
          }
        }
      }
      // adjust the buffer offset
      prepend(Offset(out.buffer));
      // now add the new mappings
      mappings.insert(mappings.begin(),
        out.srcmap->mappings.begin(),
        out.srcmap->mappings.end());
    }
  }
  // EO prepend

  // Add mapping pointing to ast node end position
  void SourceMap::moveNextMapping(int start, int end)
  {
    moveNextSrc = start;
    moveNextDst = end;
  }

  // Add mapping pointing to ast node start position
  void SourceMap::addOpenMapping(const AstNode* node, bool optional)
  {
    if (optional && !useOptionalOpeners) {
      moveNextSrc = 0;
      moveNextDst = 0;
      return;
    }
    const SourceSpan& pstate = node->pstate();
    if (pstate.getSrcIdx() != sass::string::npos) {
      auto source_start = pstate.position;
      if (moveNextSrc || moveNextDst) {
        source_start.column += moveNextSrc;
        position.column += moveNextDst;
      }
      mappings.emplace_back(Mapping{
        pstate.getSrcIdx(),
        source_start,
        position
      });
      if (moveNextSrc || moveNextDst) {
        source_start.column -= moveNextSrc;
        position.column -= moveNextDst;
        moveNextSrc = 0;
        moveNextDst = 0;
      }
    }
  }
  // EO addOpenMapping

  // Add mapping pointing to ast node end position
  void SourceMap::addCloseMapping(const AstNode* node, bool optional)
  {
    if (optional && !useOptionalClosers) {
      moveNextSrc = 0;
      moveNextDst = 0;
      return;
    }
    const SourceSpan& pstate = node->pstate();
    if (pstate.getSrcIdx() != sass::string::npos) {
      auto source_end = pstate.position + pstate.span;
      if (moveNextSrc || moveNextDst) {
        source_end.column += moveNextSrc;
        position.column += moveNextDst;
      }
      mappings.emplace_back(Mapping{
          pstate.getSrcIdx(),
          source_end,
          position
      });
      if (moveNextSrc || moveNextDst) {
        source_end.column -= moveNextSrc;
        position.column -= moveNextDst;
        moveNextSrc = 0;
        moveNextDst = 0;
      }
    }
  }
  // EO addCloseMapping

  sass::string SourceMap::render(const std::unordered_map<size_t, size_t>& idxremap) const
  {

    sass::string result;

    // Object for encoding state
    Base64VLQ base64vlq;

    // We can make an educated guess here
    // 3249594 mappings = 17669768 bytes
    result.reserve(mappings.size() * 5);

    int previous_generated_line = 0;
    int previous_generated_column = 0;
    int previous_original_line = 0;
    int previous_original_column = 0;
    int previous_original_file = 0;

    for (size_t i = 0; i < mappings.size(); ++i) {
      int generated_line = static_cast<int>(mappings[i].target.line);
      int generated_column = static_cast<int>(mappings[i].target.column);
      int original_line = static_cast<int>(mappings[i].origin.line);
      int original_column = static_cast<int>(mappings[i].origin.column);
      int original_file = static_cast<int>(idxremap.at(mappings[i].srcidx));
      bool linefeed = generated_line != previous_generated_line;

      if (linefeed) {
        previous_generated_column = 0;
        if (generated_line > previous_generated_line) {
          result += sass::string(size_t(generated_line) - previous_generated_line, ';');
          previous_generated_line = generated_line;
        }
      }

      auto generated_offset = generated_column - previous_generated_column;
      auto file_delta = original_file - previous_original_file;
      auto line_delta = original_line - previous_original_line;
      auto col_delta = original_column - previous_original_column;

      // maybe we can optimize this a bit in the future?
      // Only emit mappings if it is actually pointing at something new
      if (!i || generated_offset || file_delta || line_delta || col_delta) {
        if (!linefeed && i) result += ',';
        base64vlq.encode(result, generated_offset);
        base64vlq.encode(result, file_delta);
        base64vlq.encode(result, line_delta);
        base64vlq.encode(result, col_delta);
      }

      previous_generated_column = generated_column;
      previous_original_column = original_column;
      previous_original_line = original_line;
      previous_original_file = original_file;
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  OutputBuffer::OutputBuffer(bool srcmap) noexcept :
    srcmap(srcmap ? new SourceMap() : nullptr)
  {}

  OutputBuffer::OutputBuffer(OutputBuffer&& old) noexcept :
    buffer(std::move(old.buffer)),
    srcmap(std::move(old.srcmap))
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

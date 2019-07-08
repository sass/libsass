#include "source_map.hpp"

#include "source.hpp"
#include "ast_nodes.hpp"

namespace Sass {
  SourceMap::SourceMap() : current_position(), file("stdin") { }
  // SourceMap::SourceMap(const sass::string& file) : current_position(), file(file) { }


  sass::string SourceMap::render(const std::unordered_map<size_t, size_t>& idxremap) const
  {

    sass::string result;

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

      if (generated_line != previous_generated_line) {
        previous_generated_column = 0;
        if (generated_line > previous_generated_line) {
          result += sass::string(size_t(generated_line) - previous_generated_line, ';');
          previous_generated_line = generated_line;
        }
      }
      else if (i > 0) {
        result += ',';
      }

      // maybe we can optimize this a bit in the future?
      base64vlq.encode(result, generated_column - previous_generated_column);
      base64vlq.encode(result, original_file - previous_original_file);
      base64vlq.encode(result, original_line - previous_original_line);
      base64vlq.encode(result, original_column - previous_original_column);

      previous_generated_column = generated_column;
      previous_original_column = original_column;
      previous_original_line = original_line;
      previous_original_file = original_file;
    }

    // std::cerr << "RESULT " << result.size() << " from " << mappings.size() << "\n";

    return result;
  }

  void SourceMap::prepend(const OutputBuffer& out)
  {
    if (out.smap) {
      Offset size(out.smap->current_position);
      for (Mapping mapping : out.smap->mappings) {
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
        out.smap->mappings.begin(),
        out.smap->mappings.end());
    }

  }


  void SourceMap::append(const OutputBuffer& out)
  {
    append(Offset(out.buffer));
  }

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
    if (current_position.line == 0) {
      current_position.column += offset.column;
    }
    current_position.line += offset.line;
  }

  void SourceMap::append(const Offset& offset)
  {
    current_position += offset;
  }

  void SourceMap::add_open_mapping(const AstNode* node)
  {
    const SourceSpan& pstate = node->pstate();
    if (pstate.getSrcIdx() != sass::string::npos) {
      mappings.emplace_back(Mapping{
        pstate.getSrcIdx(),
        pstate.position,
        current_position
      });
    }
  }

  void SourceMap::add_close_mapping(const AstNode* node)
  {
    const SourceSpan& pstate = node->pstate();
    if (pstate.getSrcIdx() != sass::string::npos) {
      mappings.emplace_back(Mapping{
        pstate.getSrcIdx(),
        pstate.position + pstate.span,
        current_position
      });
    }
  }

  SourceSpan SourceMap::remap(const SourceSpan& pstate) {
    // for (size_t i = 0; i < mappings.size(); ++i) {
    //   if (
    //     mappings[i].file == pstate.getSrcId() &&
    //     mappings[i].destination.line == pstate.position.line &&
    //     mappings[i].destination.column == pstate.position.column
    //   ) return SourceSpan(pstate.path, pstate.src, mappings[i].source, pstate.offset);
    // }
    return SourceSpan(pstate.getSource());
  }

  OutputBuffer::OutputBuffer(OutputBuffer&& old) noexcept :
    buffer(std::move(old.buffer)),
    smap(std::move(old.smap))
  {
  }

}

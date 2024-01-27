/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "source_span.hpp"

#include "sources.hpp"
#include "ast_nodes.hpp"

namespace Sass {

  // Regular value constructor
  SourceSpan::SourceSpan(
    SourceDataObj source,
    const Offset& position,
    const Offset& span) :
    SourceState(source, position),
    span(span)
  {}

  // Create SourceSpan for internal things
  SourceSpan SourceSpan::internal(const char* label)
  {
    return SourceSpan(SASS_MEMORY_NEW(
      SourceString, "sass://internal", label),
      Offset{}, Offset{});
  }

  // Create span between `lhs.start` and `rhs.end` (must be same source)
  SourceSpan SourceSpan::delta(const SourceSpan& lhs, const SourceSpan& rhs)
  {
    return SourceSpan(
      lhs.getSource(), lhs.position,
      Offset::distance(lhs.position,
        rhs.position + rhs.span));
  }

  // Create span between two ast-node source-spans
  SourceSpan SourceSpan::delta(AstNode* lhs, AstNode* rhs)
  {
    return SourceSpan::delta(
      lhs->pstate(), rhs->pstate());
  }

  bool SourceSpan::operator==(const SourceSpan& rhs) const
  {
    return source.ptr() == rhs.source.ptr()
      && position == rhs.position
      && span == rhs.span;
  }

}

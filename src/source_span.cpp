#include "source_span.hpp"

#include "source.hpp"
#include "ast_nodes.hpp"

namespace Sass {

  SourceSpan::SourceSpan(
    SourceDataObj source,
    const Offset& position,
    const Offset& span) :
    SourceState(source, position),
    span(span)
  {}

  SourceSpan SourceSpan::tmp(const char* label)
  {
    return SourceSpan(SASS_MEMORY_NEW(
      SourceString, "sass://phony", label),
    {},
    {}
    );

  }

  SourceSpan SourceSpan::delta(const SourceSpan& lhs, const SourceSpan& rhs)
  {
    return SourceSpan(
      lhs.getSource(), lhs.position,
      Offset::distance(lhs.position,
        rhs.position + rhs.span));
  }

  SourceSpan SourceSpan::delta(AstNode* lhs, AstNode* rhs)
  {
    return SourceSpan::delta(
      lhs->pstate(), rhs->pstate());
  }

  bool SourceSpan::operator==(const SourceSpan& rhs) const {
    return source.ptr() == rhs.source.ptr()
      && position == rhs.position
      && span == rhs.span;
  }

}

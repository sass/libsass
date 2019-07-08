#include "highlighter.hpp"

namespace Sass {

  Hightlighter::Hightlighter(
    ParserState pstate,
    std::string color) :
    pstate(pstate),
    color(color),
    multiline(false),
    paddingBeforeSidebar(0),
    spacesPerTab(4)
  {
   // auto newSpan = _normalizeContext(span);
   // newSpan = _normalizeNewlines(newSpan);
   // newSpan = _normalizeTrailingNewline(newSpan);
   // newSpan = _normalizeEndOfLine(newSpan);
  }
}

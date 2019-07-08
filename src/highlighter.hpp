#ifndef SASS_HIGHLIGHTER_H
#define SASS_HIGHLIGHTER_H

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"
#include "ast.hpp"

#include <string>
#include <sstream>

namespace Sass {

  class Hightlighter {

  private:

    ParserState pstate;

    // The color to highlight [_span] within its context,
    // or empty if the span should not be colored.
    std::string color;

    // Whether [_span] covers multiple lines.
    bool multiline;

    // The number of characters before the bar in the sidebar.
    size_t paddingBeforeSidebar;

    // The buffer to which to write the result.
    std::stringstream buffer;

    /// The number of spaces to render for hard tabs that appear
    /// in `_span.text`. We don't want to render raw tabs, because
    /// they'll mess up our character alignment.
    size_t spacesPerTab = 4;

    Hightlighter(ParserState pstate,
      std::string color /* = default */);

    // static SourceSpanWithContext _normalizeContext(SourceSpan span);
  };


}

#endif

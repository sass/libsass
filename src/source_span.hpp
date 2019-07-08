#ifndef SASS_SOURCE_SPAN_HPP
#define SASS_SOURCE_SPAN_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "source_state.hpp"

namespace Sass {

	// ParseState is SourceSpan
	class SourceSpan : public SourceState
	{

	public:

    SourceSpan() {}

    SourceSpan(SourceDataObj source,
							 const Offset& position = Offset(),
							 const Offset& span = Offset());

    static SourceSpan tmp(const char* path);

		// Offset size
		Offset span;

		// Create span between `lhs.start` and `rhs.end` (must be same source)
		static SourceSpan delta(const SourceSpan& lhs, const SourceSpan& rhs);

		static SourceSpan delta(AstNode* lhs, AstNode* rhs);

    bool operator==(const SourceSpan& rhs) const;

	public: // down casts

    CAPI_WRAPPER(SourceSpan, SassSrcSpan);

	};

} // namespace Sass

#endif

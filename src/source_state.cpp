/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "source_state.hpp"

#include "source.hpp"

namespace Sass
{

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SourceState::SourceState(
    SourceData* source,
    Offset position) :
    source(source),
    position(position)
	{}

  SourceData* SourceState::getSource() const
	{
		return source.ptr();
	}

  size_t SourceState::getSrcIdx() const
	{
		return source->getSrcIdx();
	}

  // Return the requested import path
	const char* SourceState::getImpPath() const
	{
		return source->getImpPath();
	}

  // Return the resolved absolute path
  const char* SourceState::getAbsPath() const
  {
    return source->getAbsPath();
  }

  // Return the attached source
  const char* SourceState::getContent() const
  {
    return source->content();
  }

  sass::string SourceState::getDebugPath() const
	{
		const char* path = getAbsPath();
		sass::string rel_path(File::abs2rel(path, CWD, CWD));
		return File::rel2dbg(rel_path, path);
	}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

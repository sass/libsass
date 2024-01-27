/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "source_state.hpp"

#include "source.hpp"
#include "file.hpp"

namespace Sass
{

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Regular value constructor
  SourceState::SourceState(
    SourceData* source,
    Offset position) :
    source(source),
    position(position)
  {}

  // Return the attach source id
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

  // Return the resolved absolute path
  const char* SourceState::getFileName() const
  {
    return source->getFileName();
  }

  // Return the attached source
  SourceData* SourceState::getSource() const
  {
    return source.ptr();
  }

  // Return the attached source
  const char* SourceState::getContent() const
  {
    return source->content();
  }

  // Either return path relative to cwd if path is
  // inside cwd, otherwise return absolute path.
  sass::string SourceState::getDebugPath() const
  {
    const char* path = getAbsPath();
    sass::string rel_path(File::abs2rel(path, CWD(), CWD()));
    return rel_path.substr(0, 3) == "../" ? path : rel_path;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

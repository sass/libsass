/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "import.hpp"

#include "sources.hpp"
#include "exceptions.hpp"

namespace Sass {

  // Entry point for top level file import
  // Don't load like other includes, we do not
  // check inside include paths for this file!
  void Import::loadIfNeeded(BackTraces& traces)
  {
    // Only load once
    if (isLoaded()) return;
    // Check if entry file-path is given
    // Use err string of LoadedImport
    if (getAbsPath() == nullptr) {
      throw std::runtime_error(
        "No file path given to be loaded.");
    }
    // try to read the content of the resolved file entry
    // the memory buffer returned to us must be freed by us!
    if (char* contents = File::slurp_file(getAbsPath(), CWD())) {
      // Upgrade to a source file
      // ToDo: Add sourcemap parsing
      source = SASS_MEMORY_NEW(SourceFile,
        source->getImpPath(),
        source->getAbsPath(),
        contents, nullptr
      );
    }
    else {
      // Throw error if read has failed
      throw Exception::IoError(traces,
        "File not found or unreadable",
        File::abs2rel(source->getAbsPath()));
    }
  }

  const char* Import::getImpPath() const
  {
    return source->getImpPath();
  }

  const char* Import::getAbsPath() const
  {
    return source->getAbsPath();
  }

  const char* Import::getFileName() const
  {
    return source->getFileName();
  }

  const char* Import::getErrorMsg() const
  {
    return error;
  }

  void Import::setErrorMsg(const char* msg)
  {
    sass_free_c_string(error);
    error = sass_copy_c_string(msg);
  }

  Import::Import(
    SourceData* source,
    SassImportSyntax syntax) :
    source(source),
    syntax(syntax)
  {}

  bool Import::isLoaded() const
  {
    return source && source->content();
  }

}


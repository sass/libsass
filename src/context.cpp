#include "context.hpp"

#include "source.hpp"
#include "plugins.hpp"
#include "exceptions.hpp"

namespace Sass {
  
  using namespace File;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Initialize current directory once
  sass::string CWD(get_cwd());

  // Helper function to sort header and importer arrays by priorities
  inline bool cmpImporterPrio(struct SassImporter* i, struct SassImporter* j)
  {
    return sass_importer_get_priority(i) > sass_importer_get_priority(j);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Default constructor
  Context::Context() :
    SassOutputOptionsCpp(),
    varRoot(varStack)
  {
    // Sass 3.4: The current working directory will no longer be placed onto the Sass load path by default.
    // If you need the current working directory to be available, set SASS_PATH=. in your shell's environment.
    // Or add it explicitly in your implementation, e.g. includePaths.emplace_back(CWD or '.');
  }
  // EO Context ctor

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Add additional include paths, which can be path separated
  void Context::addIncludePaths(const sass::string& paths)
  {
    // Check if we have anything to do
    if (paths.empty()) return;
    // Load plugins from all paths
    sass::vector<sass::string> split =
      StringUtils::split(paths, PATH_SEP, true);
    for (sass::string& path : split) {
      if (*path.rbegin() != '/') path += '/';
      includePaths.emplace_back(path);
    }
  }
  // EO addIncludePaths

  // Load plugins from path, which can be path separated
  void Context::loadPlugins(const sass::string& paths)
  {
    Plugins plugins;
    // Check if we have anything to do
    if (paths.empty()) return;
    // Load plugins from all paths
    sass::vector<sass::string> split =
      StringUtils::split(paths, PATH_SEP, true);
    for (sass::string& path : split) {
      std::cerr << "Load " << path << "\n";
      if (*path.rbegin() != '/') path += '/';
      plugins.load_plugins(path);
      std::cerr << "Loaded " << path << "\n";
    }
    // Take over ownership from plugin
    plugins.consume_headers(cHeaders);
    plugins.consume_importers(cImporters);
    plugins.consume_functions(cFunctions);
    // Sort the merged arrays by callback priorities
    sort(cHeaders.begin(), cHeaders.end(), cmpImporterPrio);
    sort(cImporters.begin(), cImporters.end(), cmpImporterPrio);
  }
  // EO loadPlugins

  /////////////////////////////////////////////////////////////////////////
  // Helpers for `sass_prepare_context`
  // Obsolete when c_ctx and cpp_ctx are merged.
  /////////////////////////////////////////////////////////////////////////

  void Context::addCustomHeader(struct SassImporter* header)
  {
    if (header == nullptr) return;
    cHeaders.emplace_back(header);
    // need to sort the array afterwards (no big deal)
    sort(cHeaders.begin(), cHeaders.end(), cmpImporterPrio);
  }

  void Context::addCustomImporter(struct SassImporter* importer)
  {
    if (importer == nullptr) return;
    cImporters.emplace_back(importer);
    // need to sort the array afterwards (no big deal)
    sort(cImporters.begin(), cImporters.end(), cmpImporterPrio);
  }

  void Context::addCustomFunction(struct SassFunction* function)
  {
    if (function == nullptr) return;
    cFunctions.emplace_back(function);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Implementation for `sass_compiler_find_file`
  sass::string Context::findFile(const sass::string& path)
  {
    // Get last import entry for current base
    Import& import = import_stack.back();
    // create the vector with paths to lookup
    sass::vector<sass::string> incpaths(1 + includePaths.size());
    incpaths.emplace_back(dir_name(import.source->getAbsPath()));
    incpaths.insert(incpaths.end(), includePaths.begin(), includePaths.end());
    return find_file(path, CWD, incpaths, fileExistsCache);
  }

  // Look for all possible filename variants (e.g. partials)
  // Returns all results (e.g. for ambiguous valid imports)
  sass::vector<ResolvedImport> Context::findIncludes(const ImportRequest& import)
  {
    // make sure we resolve against an absolute path
    sass::string base_path(rel2abs(import.base_path, ".", CWD));
    // first try to resolve the load path relative to the base path
    sass::vector<ResolvedImport> vec(resolve_includes(
      base_path, import.imp_path, CWD, fileExistsCache));
    // then search in every include path (but only if nothing found yet)
    for (size_t i = 0, S = includePaths.size(); vec.size() == 0 && i < S; ++i)
    {
      sass::vector<ResolvedImport> resolved(resolve_includes(
        includePaths[i], import.imp_path, CWD, fileExistsCache));
      vec.insert(vec.end(), resolved.begin(), resolved.end());
    }
    // return vector
    return vec;
  }

  // Load import from the file-system and create source object
  Import* Context::loadImport(const ResolvedImport& import)
  {
    // Try to find the item in the cache first
    auto cached = sources.find(import.abs_path);
    if (cached != sources.end()) return cached->second;
    // Try to read source and (ToDo) optional mappings
    if (ImportObj loaded = read_file(import)) {
      sources.insert({ import.abs_path, loaded });
      return loaded.detach();
    }
    // Throw error if read has failed
    throw Exception::OperationError(
      "File to read not found or unreadable.");
  }
  // EO loadImport

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

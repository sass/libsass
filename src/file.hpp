/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_FILE_HPP
#define SASS_FILE_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <string>
#include <vector>

#include "ast_fwd_decl.hpp"
#include "ast_def_macros.hpp"
#include "backtrace.hpp"
#include "hashing.hpp"
#include "source.hpp"

namespace Sass {

  // return the current directory
  // always with forward slashes
  extern sass::string get_pwd();

  // return the current directory
  // always with forward slashes
  extern void set_cwd(const sass::string& path);

  // Should be thread_local?
  extern const sass::string& CWD();

  namespace File {


    // test if path exists and is a file
    bool file_exists(const sass::string& file, const sass::string& CWD,
      std::unordered_map<sass::string, bool>& cache);

    // return if given path is absolute
    // works with *nix and windows paths
    bool is_absolute_path(const sass::string& path);

    // return only the directory part of path
    sass::string dir_name(const sass::string& path);

    // return only the filename part of path
    sass::string base_name(const sass::string&);

    // do a logical clean up of the path
    // no physical check on the file-system
    sass::string make_canonical_path (sass::string path);

    // join two path segments cleanly together
    // but only if right side is not absolute yet
    sass::string join_paths(sass::string root, sass::string name);

    // create an absolute path by resolving relative paths with cwd
    sass::string rel2abs(const sass::string& path, const sass::string& base = Sass::CWD(), const sass::string& CWD = Sass::CWD());

    // create a path that is relative to the given base directory
    // path and base will first be resolved against cwd to make them absolute
    sass::string abs2rel(const sass::string& path, const sass::string& base = Sass::CWD(), const sass::string& CWD = Sass::CWD());

    // helper function to resolve a filename
    // searching without variations in all paths
    // sass::string find_file(const sass::string& file, const sass::string& CWD, struct SassCompilerCpp* options);
    sass::string find_file(const sass::string& file, const sass::string& CWD, const StringVector paths, std::unordered_map<sass::string, bool>& cache);

    // helper function to resolve a include filename
    // this has the original resolve logic for sass include
    sass::string find_include(const sass::string& file, const sass::string& CWD, const StringVector paths, bool forImport, std::unordered_map<sass::string, bool>& cache);

    // split a path string delimited by semicolons or colons (OS dependent)
    // StringVector split_path_list(sass::string paths);

    // try to load the given filename
    // returned memory must be freed
    char* slurp_file(const sass::string& file, const sass::string& CWD);

    // Read and return resolved import
    Import* read_import(const ResolvedImport& import);

    sass::vector<ResolvedImport> resolve_includes(const sass::string& root, const sass::string& file, const sass::string& CWD, bool forImport,
      std::unordered_map<sass::string, bool>& cache, const std::vector<sass::string>& exts = { ".sass", ".scss", ".css" });

  }

}

#endif

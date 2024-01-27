/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_IMPORT_HPP
#define SASS_IMPORT_HPP

#include "file.hpp"

namespace Sass {

  // requested import
  class ImportRequest {
  public:
    // requested import path
    sass::string imp_path;
    // parent context path
    sass::string ctx_path;
    // base derived from context path
    // this really just acts as a cache
    sass::string base_path;
    // Consider `.import` files?
    bool considerImports = false;
  public:
    ImportRequest(
      sass::string imp_path,
      sass::string ctx_path,
      bool considerImports) :
      imp_path(File::make_canonical_path(imp_path)),
      ctx_path(File::make_canonical_path(ctx_path)),
      base_path(File::dir_name(ctx_path)),
      considerImports(considerImports)
    {
      if (base_path == "stream://") {
        base_path = CWD();
      }
    }

    bool operator==(const ImportRequest& other) const {
      return considerImports == other.considerImports
        && imp_path == other.imp_path
        && ctx_path == other.ctx_path;
    }

    ImportRequest() {};
  };

  // a resolved include (final import)
  class ResolvedImport : public ImportRequest {
  public:
    // resolved absolute path
    sass::string abs_path;
    // which importer to use
    SassImportSyntax syntax;
  public:
    ResolvedImport(
      const ImportRequest& imp,
      sass::string abs_path,
      SassImportSyntax syntax)
      : ImportRequest(imp),
      abs_path(abs_path),
      syntax(syntax)
    {}
  };


  // Base class for entry points
  class Import : public RefCounted {
  public:
    SourceDataObj source;
    SassImportSyntax syntax;
    char* error = nullptr;
    void loadIfNeeded(BackTraces& traces);
    bool isLoaded() const;
    const char* getImpPath() const;
    const char* getAbsPath() const;
    const char* getFileName() const;
    // This is used by custom importer
    // Easiest way to communicate back
    const char* getErrorMsg() const;
    void setErrorMsg(const char* msg);
    Import(SassImportSyntax syntax = SASS_IMPORT_AUTO) :
      syntax(syntax)
    {}

    ~Import() {
      sass_free_c_string(error);
    }

    Import(
      SourceData* source,
      SassImportSyntax syntax);

    CAPI_WRAPPER(Import, SassImport);
  };

}

namespace std {
  template <> struct hash<Sass::ImportRequest> {
  public:
    inline size_t operator()(
      const Sass::ImportRequest& import) const
    {
      size_t hash = import.considerImports;
      Sass::hash_combine(hash,
        MurmurHash2(
          (void*)import.base_path.c_str(),
          (int)import.base_path.size(),
          Sass::getHashSeed()));

      Sass::hash_combine(hash,
        MurmurHash2(
          (void*)import.imp_path.c_str(),
          (int)import.imp_path.size(),
          Sass::getHashSeed()));
      return hash;
    }
  };
}

#endif

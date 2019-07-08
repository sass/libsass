#ifndef SASS_STYLESHEET_HPP
#define SASS_STYLESHEET_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "file.hpp"

namespace Sass {

  // parsed stylesheet from loaded resource
  // this should be a `Module` for sass 4.0
  class StyleSheet : public SharedObj {
    public:

      // Whether this was parsed from a plain CSS stylesheet.
      // bool plainCss33;

      // SourceDataObj source;

      // The canonical URL for this module's source file. This may be `null`
      // if the module was loaded from a string without a URL provided.
      // struct SassImport* import;

      // the import type
      // SassImportFormat syntax;

      // Modules that this module uses.
      // List<Module> get upstream;

      // The module's variables.
      // Map<String, Value> get variables;

      // The module's functions. Implementations must ensure
      // that each [Callable] is stored under its own name.
      // Map<String, Callable> get functions;

      // The module's mixins. Implementations must ensure that
      // each [Callable] is stored under its own name.
      // Map<String, Callable> get mixins;

      // The extensions defined in this module, which is also able to update
      // [css]'s style rules in-place based on downstream extensions.
      // Extender extender;
      ImportObj import;

      // The module's CSS tree.
      RootObj root2;

    public:

      // default argument constructor
      StyleSheet(ImportObj import, RootObj root);

  };


}

#endif

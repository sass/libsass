## API documentation basics

LibSass is C library, written in C++ with a C-API to link against.
It can either be statically included are linked against a system
wide installed shared library. The complete C-API surface is implemented
via functions and by passing around anonymous structures. The implementor
does not know any implementation detail about the objects passed around.

### C-API headers

You may include specific headers directly, which have been split into
groups as we saw fit, or you can simply include the main `sass.h` header.

### List of C-API headers

- base.h - Includes and defines basic stuff.
- fwdecl.h - Forward declares all known anonymous structures.
- enums.h - Enums for e.g. config options exposed on the C-API.
- error.h - C-API functions to access `SassError` structs.
- import.h - C-API functions to access `SassImport` structs.
- importer.h - C-API functions to access `SassImporter` structs.
- traces.h - C-API functions to access `SassTrace` structs.
- values.h - C-API functions to access `SassValue` structs.
- variable.h - C-API functions to access scoped variables.
- version.h - Header including hardcoded LibSass version.

## Anonymous structure pointers

LibSass returns and consumes anonymous structure pointers on its C-API.
Internally those are often directly mapped to an instance of a C++ object.
Since we don't want implementors to know anything about the implementation
details, we forward declare named structs (e.g. `SassCompiler`), but never
provide any implementation for it. LibSass will simply cast a pointer from
the c++ side to the anonymous struct and relies on the C-API to pass it
back as expected to the corresponding functions. There the pointer will
be statically cast back to the actual implementation. Since we provide
a unique anonymous struct for every internal type, this should be safe as
compilers should catch the case when arguments mismatch on the C-API side.

### List of anonymous structures

These structs have no real implementation and are only passed around as pointers.
Internally in LibSass these pointers represent mostly different c++ objects.

- `SassError` - An error object holding further information
- `SassTrace` - An single stack-trace object holding further information
- `SassSource` - Imported source with associated import and resolved path.
- `SassSrcSpan` - Parser state for column, line and source information.
- `SassCompiler` - Main object to hold state for whole compilation phase.
- `SassFunction` - Custom function object holding callback and cookie.
- `SassImport` - Single import for entry point or returned by custom importer.
- `SassImporter` - Custom importer function to be hooked into our loading.
- `SassImportList` - Custom importers can return a SassImport list.
- `SassMapIterator` - Object to support iteration API over map objects.

## Why using c++11 (or gcc4.4 compatibility)

Since LibSass 3.0 we started to use more and more c++11 features. Some just
crept in, others were utilized deliberately. With LibSass 4.0 I took the
decision to fully utilize whatever c++11 could offer. The main reason to
fully embrace c++11 is the move semantics it brings. Earlier we also tried
to support compilers that only had partial c++11 support (e.g. gnu++0x).
With LibSass 4.0 we don't really support this target anymore, as any compiler
not older than 5 years should support the c++11 syntax we use.

Note: currently the LibSass 4.0 release is on going and the final
target compiler is gcc 4.8, as it should fully support c++11.
More testing and tuning needs to be done after a 4.0 release!

## Binary distributions and linkage issues

LibSass itself does not have any official binary distribution. The main reason for
this is because it is nearly impossible to do so reliably for each and every
thinkable operating system. Since LibSass is written in c++ we e.g. depend on the
compiler c++ runtime library. On windows this problem is a bit less problematic,
and there is a [semi-official installer][1] for it. But on Linux this e.g. is also
depending on the used libc library. Since linux offers a choice here, a binary
distribution compiled with glibc may not be compatible on a system that uses musl,
or a compilation with latest glibc may not be compatible with older glibc versions.

By now LibSass is readily available on a lot of linux systems by their internal
packet managers, so please try to install it that way. Alternatively try to install
a recent compiler (e.g. gcc or clang) and compile it from source, preferably via the
autotools way, to ensure correct linkage with tools that link against system wide
installed LibSass (GNU coding style).

[1]: http://libsass.ocbnet.ch/installer/

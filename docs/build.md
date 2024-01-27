## Building libsass from source

LibSass is only a library and does not do much on its own, you'll need an additional
piece to tell LibSass what to do actually, called the implementor. The most basic
implementor is [`sassc`][6], as a basic command line interface (CLI). LibSass can
either be compiled as a static or shared library, and provides a [functional API][12]
to its consumer. Therefore it can be utilized to drive all kind of different
applications (CLI utility, bindings to other languages or even in browsers).

This guide will try to follow you through the steps to get [`sassc`][6] running.

You need a working C++ compiler (e.g. gcc 4.8 or newer). If you are new to C++,
please consult the internet how to get a working compiler toolchain running on your
operating system, or see our guide to [setup dev environment](setup-environment.md).

## Building on different Operating Systems

We try to keep the LibSass as OS independent and standard compliant as possible.
Reading files from the file-system has some OS depending code, but will ultimately
fall back to a posix compatible implementation. We do use newer `C++11` features,
meaning you will need a recent compiler (gcc 4.7 being the current minimum).

### Building on Linux (and other *nix flavors)

Linux is the main target for `libsass` and we support two ways to build `libsass` here.
The old plain makefiles should still work on most systems (including MinGW), while the
autotools build is preferred if you want to create a [system library].

- [Building with makefiles][1]
- [Building with autotools][2]

### Building on Windows (mileage may vary)

On windows you can either compile LibSass via MSVC or a posix compatible toolchain
like MinGW, MSYS2 or Cygwin. It should always compile fine with the latest compiler
toolchain provided by [Strawberry perl](https://strawberryperl.com/).

- [Building with MinGW][3]
- [Building with Visual Studio][11]

### Building on Max OS X (untested)

Works the same as on linux, but you can also install LibSass via `homebrew`.

- [Building on Mac OS X][10]

### Building a system library (experimental)

Since `libsass` is a library, it makes sense to install it as a system-wide shared
library. On linux this means creating versioned `.so` library via autotools.

- [Building shared system library][4]

#### Compiling with clang instead of gcc

To use clang you just need to set the appropriate environment variables:

```bash
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
```

#### Running the spec test-suite

We constantly and automatically test `libsass` against the official [spec test-suite][5].
To do this we need to have a test-runner (which is written in ruby) and a command-line
tool ([`sassc`][6]) to run the tests. Therefore we need to additionally compile `sassc`.
To do this, the build files of all three projects need to work together.

You also need some ruby gems installed:

```bash
ruby -v
gem install hrx
gem install minitest
# should be optional
gem install minitap
```

#### Including the LibSass version

There is a function in `libsass` to query the current version. This has to be defined at compile time.
We use a C macro for this, which can be defined by calling e.g. `g++ -DLIBSASS_VERSION="\"x.y.z.\""`.
The two quotes are necessary, since it needs to end up as a valid C string. Normally you do not
need to do anything if you use the makefiles or autotools. They will try to fetch the version
either via an optional VERSION file or via git directly. If you only have the sources without
the git repo, you can also pass the version as an environment variable to `make` or `configure`:

```
export LIBSASS_VERSION="x.y.z."
```

#### Continuous Integration

We use two CI services to automatically test all commits against the latest [spec test-suite][5].

- [LibSass on GitHub Actions (linux)][7]
[![Build Status](https://github.com/sass/libsass/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/sass/libsass/actions/workflows/build-and-test.yml)
- [LibSass on AppVeyor (windows)][8]
[![Build status](https://ci.appveyor.com/api/projects/status/github/sass/libsass?svg=true)](https://ci.appveyor.com/project/sass/libsass/branch/master)

#### Why not using CMake (or any other tool)?

There were some efforts to get `libsass` to compile with CMake, which should make it easier to
create build files for linux and windows. Unfortunately this was not completed. But we are certainly open for PRs!

[1]: build-with-makefiles.md
[2]: build-with-autotools.md
[3]: build-with-mingw.md
[4]: build-shared-library.md
[5]: https://github.com/sass/sass-spec
[6]: https://github.com/sass/sassc
[7]: https://github.com/sass/libsass/blob/master/.github/workflows
[8]: https://github.com/sass/libsass/blob/master/appveyor.yml
[9]: implementations.md
[10]: build-on-darwin.md
[11]: build-with-visual-studio.md
[12]: api-doc.md

# LibSass amalgamation script

This script concatenates LibSass sources into a single file.

This reduces single-core compilation time by 50% and the output shared library size by 10%.

SQLite has a great writeup on amalgamation here:
<https://www.sqlite.org/amalgamation.html>.

## Options

* `--inline`. Default: `false`\
  When `true`, all sources are inlined into the amalgam file (including headers.\
  When `false`, the amalgam file instead contains `#include` statements for each `.c,.cpp` file.
* `--root`. Default: `$PWD`.\
  The root directory.
  By default, the `src/` subdirectory of root is searched and includes are
  resolved relative to it.
* `--out`. Default: `/dev/stdout`.\
  Path to the amalgamated source output.

## Benchmarks

With amalgamation:

~~~bash
rm -f script/amalgamate/build/amalgamate && make clean AMALGAM=1 && \
  time make lib/libsass.so AMALGAM=1 && du -sh lib/libsass.so
~~~

Compilation time (1 core): 30s
`lib/libsass.so` size: 3.0M

These numbers are the same affected for both inline and include-style amalgamation.

Without amalgamation:

~~~bash
make clean AMALGAM=0 && time make -j`nproc` lib/libsass.so AMALGAM=0 && du -sh lib/libsass.so
~~~

Compilation time (1 core): 60s
Compilation time (8 cores): 16s
`lib/libsass.so` size: 3.3M

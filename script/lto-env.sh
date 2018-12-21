#!/bin/sh

cc=$1
if test -z "$cc" || test "$cc" = dummy; then
  cc=cc
fi

cc_version="$("$cc" -v 2>&1 | grep ' version')"
compiler_family="$(echo "$cc_version" | grep -o '[a-z]* version' | grep -o '^[^ ]*' | sed s/LLVM/clang/)"
enable_lto=no
if test -z "$NO_LTO" && test "$COVERAGE" != yes; then
  if test "$compiler_family" = clang; then
    enable_lto=yes AR=llvm-ar RANLIB=llvm-ranlib
  elif test "$compiler_family" = gcc; then
    enable_lto=yes AR=gcc-ar RANLIB=gcc-ranlib
  fi
fi

echo >&2 "ENABLE_LTO=$enable_lto
  cc -v | grep ' version': $cc_version
  Detected compiler family: $compiler_family
  ld -v: $(ld -v)"

if test "$enable_lto" = yes; then
  echo "ENABLE_LTO=yes AR=$AR RANLIB=$RANLIB"
else
  echo "ENABLE_LTO=no"
fi

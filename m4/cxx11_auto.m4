#serial 1
AC_DEFUN([CXX11_AUTO],
[
  AC_REQUIRE([CXX11_STD_AVAILABLE])
  CXX11_STD_TRY([auto], [
#include <string>
], [[
using namespace std;
  class A {
    void m() {
      auto string = 2;
    }
  }
]],
  [ $cxx11_cv_prog_cxx_cxx11 ],
  [
    AC_DEFINE([HAVE_CXX11_AUTO],[1],
      [Define to 1 C++11 auto variables are available])
  ],
  [])
])

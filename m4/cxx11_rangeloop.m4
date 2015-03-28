#serial 1
AC_DEFUN([CXX11_RANGELOOP],
[
  AC_REQUIRE([CXX11_STD_AVAILABLE])
  CXX11_STD_TRY([rangeloop],
  [[#include <string>]],
  [[
  std::string str = "Hello";
  for (char c : str) { }
  ]],
  [ $cxx11_cv_prog_cxx_cxx11 ],
  [
    AC_DEFINE([HAVE_CXX11_RANGE_LOOP],[1],
      [Define to 1 C++11 range loops are available])
  ],
  [])])
])

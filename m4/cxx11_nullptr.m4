#serial 1
AC_DEFUN([CXX11_NULLPTR],
[
  AC_REQUIRE([CXX11_STD_AVAILABLE])
  CXX11_STD_TRY([nullptr], [], [[char *s = nullptr;]],
  [ $cxx11_cv_prog_cxx_cxx11 ],
  [
    AC_DEFINE([HAVE_CXX11_NULLPTR],[1],
      [Define to 1 C++11 nullptr is available])
  ],
  [])
])

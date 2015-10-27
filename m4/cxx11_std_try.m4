#serial 1
# CXX11_STD_TRY(STANDARD, TEST-PROLOGUE, TEST-BODY, OPTION-LIST,
#                 ACTION-IF-AVAILABLE, ACTION-IF-UNAVAILABLE)
# ----------------------------------------------------------------
# Check whether the C++ compiler accepts features of STANDARD (e.g
# `cxx98', `cxx11') by trying to compile a program of TEST-PROLOGUE
# and TEST-BODY.  If this fails, try again with each compiler option
# in the space-separated OPTION-LIST; if one helps, append it to CXX.
# If eventually successful, run ACTION-IF-AVAILABLE, else
# ACTION-IF-UNAVAILABLE.
AC_DEFUN([_CXX11_STD_TRY],
[
AC_LANG_PUSH(C++)dnl
AC_CACHE_VAL(cxx11_cv_prog_cxx_$1,
[cxx11_cv_prog_cxx_$1=no
cxx11_save_CXX=$CXX
AC_LANG_CONFTEST([AC_LANG_PROGRAM([$2], [$3])])
for cxx11_arg in $4
do
  CXX="$cxx11_save_CXX $cxx11_arg"
  _AS_ECHO_N([$cxx11_arg ])
  AC_COMPILE_IFELSE([], [cxx11_cv_prog_cxx_$1=$cxx11_arg])
  test "x$cxx11_cv_prog_cxx_$1" != "xno" && break
done
rm -f conftest.$ac_ext
CXX=$cxx11_save_CXX
])# AC_CACHE_VAL
cxx11_prog_cxx_stdcxx_options=
case "x$cxx11_cv_prog_cxx_$1" in
  x)
    AC_MSG_RESULT([working as-is]) ;;
  xno)
    AC_MSG_RESULT([not working]) ;;
  *)
    cxx11_prog_cxx_stdcxx_options=" $cxx11_cv_prog_cxx_$1"
    CXX=$CXX$cxx11_prog_cxx_stdcxx_options
    AC_MSG_RESULT([working]) ;;
esac
AC_LANG_POP(C++)dnl
])# _CXX11_STD_TRY
AC_DEFUN([CXX11_STD_TRY], 
[
  AC_MSG_CHECKING([for $CXX option to enable ]m4_translit($1, [x], [+])[ feature])
  _CXX11_STD_TRY($1, $2, $3, ["" m4_expand($4)])
  AS_IF([test "x$cxx11_cv_prog_cxx_$1" != xno], [$5], [$6])
])
AC_DEFUN([CXX11_STD_AVAILABLE],
[
  cxx11_save_CXX=$CXX
  AC_MSG_CHECKING([for $CXX to have C++11 rvalue references])
  _CXX11_STD_TRY([cxx11], [], [[double&& d = 2]],
  [[ -std=c++11 -std=c++0x -qlanglvl=extended0x -AA ]])
  CXX=$cxx11_save_CXX
])

This page is mostly intended for people that want to build a system library that
gets distributed via RPMs or other means. This is currently in a experimental
phase, as we currently do not really guarantee any ABI forward compatibility.
The C API was rewritten to make this possible in the future, but we want to
wait some more time till we can call this final and stable.

Building via autotools
--

You want to build a system library only via autotools, since it will create the
proper `libtool` files to make it loadable on multiple systems. We hope this works
correctly, but nobody of the `libsass` core team has much knowledge in this area.
Therefore we are open for comments or improvements by people that have more experience
in that matter (like package maintainers from various linux distributions).

```bash
apt-get install autoconf libtool
git clone https://github.com/sass/libsass.git
cd libsass
autoreconf --force --install
./configure \
  --disable-tests \
  --disable-static \
  --enable-shared \
  --prefix=/usr
make -j5 install
cd ..
```

This should install these files
```bash
# $ ls -la /usr/lib/libsass.*
/usr/lib/libsass.la
/usr/lib/libsass.so -> libsass.so.2.0.0
/usr/lib/libsass.so.0 -> libsass.so.2.0.0
/usr/lib/libsass.so.2.0.0
# $ ls -la /usr/include/sass*
/usr/include/sass.h
/usr/include/sass/base.h
/usr/include/sass/compiler.h
/usr/include/sass/enums.h
/usr/include/sass/error.h
/usr/include/sass/function.h
/usr/include/sass/fwdecl.h
/usr/include/sass/import.h
/usr/include/sass/importer.h
/usr/include/sass/traces.h
/usr/include/sass/values.h
/usr/include/sass/variable.h
/usr/include/sass/version.h
```

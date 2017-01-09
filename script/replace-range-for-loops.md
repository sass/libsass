## Unroll C++11 range-based for loops for GCC 4.4 compatibility

By [Marcel Greter](@mgreter)

LibSass needs a pretty recent compiler in order to support
C++11. We only use a small subset of C++11 features, so most
of our code is still compatible with C++98. GCC added more
C++ features over time, so chances are that even versions that
do not have full C++11 support can compile LibSass (--gnu++11)

https://github.com/sass/libsass/pull/1623

I created a few small patches to make our use of the random number
generator compatible with older GCC versions. The only thing left
is our use of range-based for loops, which is not available on
GCC 4.4. Since I like that syntax very much, I decided to keep it
in the source and use a script to automatically unroll them.

`cd script && perl replace-range-for-loops.pl && cd ..`

### Verifying the resulting code base

I've come up with the following recipe to verify compilation:

```cmd
mkdir libsass-gcc-4.4
cd libsass-gcc-4.4
wget -c http://strawberryperl.com/download/5.12.3.0/strawberry-perl-5.12.3.0-portable.zip
mkdir perl
cd perl
unzip ..\strawberry-perl-5.12.3.0-portable.zip
portableshell.bat

git clone https://github.com/sass/perl-libsass.git
cd perl-libsass
git submodule update --init
cd libsass
git remote add mgreter https://github.com/mgreter/libsass.git
git fetch mgreter
git checkout -b compat/gcc-4.4 mgreter/compat/gcc-4.4
perl script\replace-range-for-loops.pl
cd ..
cpan ExtUtils::CppGuess
cpan Test::Differences
cpan Encode::Locale
gcc -v
cpan .
```

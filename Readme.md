NSass
=====

by TBAPI-0KA

About
-----

NSass is a .NET wrapper around libsass library. All information about libsass itself could be found in readme file under *NSass.LibSass* directory.

At this moment NSass is under development and contains very simple one-to-one C++/CLI wrapper around libsass, and high-level C# wrapper.

Main principles
---------------

* No modifications to original libsass code for easy merge;
* Simple as possible, almost without additional features;
* Easy to use.

Build process
-------------

NSass tries to play nice with x86/x64 issues. Thanks to Scott Bilas - http://scottbilas.com/blog/automatically-choose-32-or-64-bit-mixed-mode-dlls/.
But this approach inflict NSass build process while simplifying it for end-user.

If you want Intellisense/Resharper support for *NSass.Core*, just build *NSass.Wrapper* in Win32 or x64 mode.
But at the end, you should build *NSass.Core* in Win32 mode first, then in x64 mode. Or, use Batch Build, which is easier (check both Win32 and x64 configurations for *NSass.LibSass* and *NSass.Wrapper*, as well as AnyCPU for *NSass.Core*).

How to include in your project
------------------------------

First of all, you need to reference *NSass.Core*.

Second, until NuGet packages are not implemented to make all the dirty work, to use NSass in your web project, add the following post-build event to it:

```bat
if not exist "$(SolutionDir)$(ProjectName)\NSass.Wrapper\" mkdir "$(SolutionDir)$(ProjectName)\NSass.Wrapper\"
del /q "$(SolutionDir)$(ProjectName)\NSass.Wrapper\*.*"
copy "{NSass output directory}\NSass.Wrapper.*.dll" "$(SolutionDir)$(ProjectName)\NSass.Wrapper\"
copy "{NSass output directory}\NSass.Wrapper.*.pdb" "$(SolutionDir)$(ProjectName)\NSass.Wrapper\"
```

where *{NSass output directory}* should point to NSass binaries directory. Of course, you could use *$(SolutionDir)* variable and relative paths to keep thing clear.
To make it working with Publish, include NSass.Wrapper library in project after first build.

Usage
-----

Code is pretty straightforward. First of all, create an instance of SassCompiler:

```c#
ISassCompiler sassCompiler = new SassCompiler();
```

Compile regular string with SASS code inside:

```c#
string output = sassCompiler.Compile("/* COMMENT */ html { body { color: red; } }");
```

... or a file:

```c#
string output = sassCompiler.CompileFile(@"C:\Site.scss");
```

If you want to handle SASS compilation errors nice, just catch SassCompileException - message inside contains all the necessary information.

What about unit testing / dependency injection?
-----------------------------------------------

Well, NSass itself is very simple and just sends all the work to libsass.
So at this moment it does not contain any tests, but probably I shall add some "smoke tests" - I need to test NSass engine after changes anyway.

But, you could mock NSass classes in your tests or use your favorite IoC container for initialization.
There are two extension points - *ISassInterface* in C++/CLI code, and *ISassCompiler* in C# code, implemented as *SassInterface* and *SassCompiler* accordingly by default.
In most cases you should deal with *ISassCompiler* only.

Roadmap
-------

* HttpHandler;
* Bundle&Minification support;
* NuGet packages;
* Probably, simple SASS compilation tool for Windows for usage outside of .NET world.

All the other potential features (i. e. Compass support) will be merged from libsass when they will appear there.

Known issues
------------

* libsass does not support other output styles than nested and compressed now, but declares them on interface level. You will get an exception when trying to use expanded, echo, or compact output style.
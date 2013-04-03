NSass
=====

by TBAPI-0KA

About
-----

NSass is a .NET wrapper around libsass library. All information about libsass itself could be found in readme file under *NSass.LibSass* directory.

At this moment NSass is under development. By the way, you already may use:

* simple one-to-one C++/CLI wrapper around libsass
* high-level C# wrapper (however, without much of additional features)
* HTTP handler for live SASS processing

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

First of all, you need to reference *NSass.Handler* or *NSass.Core*.

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

If you have web project and just want to process your SASS files at runtime, all you need is this line in Web.config:

```xml
<add name="ScssSassHandler" verb="GET" path="*.scss" type="NSass.SassHandler, NSass.Handler, Version=0.0.1.0, Culture=neutral, PublicKeyToken=null" />
```

If you have more deeper purpose, follow the next. First of all, create an instance of SassCompiler:

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

Note about @import
------------------

By default, NSass core adds current directory as import path while compilining a file. NSass handler additionaly adds web project root.
So you may use both relative and absolute paths in includes. For example, if you have Site.scss in your Content folder, and this file includes Common.scss from the same folder, everything below will work in the same way:

* @import "Common.scss";
* @import "Content/Common.scss";
* @import "/Content/Common.scss";
* @import "../Common.scss";

What is important to know - only first found file will be included. Searching process goes through current directory first, then through web site root.
This may cause issues if you have another Content folder with Common.scss file inside, which is under root Content folder. Illustration:

```c#
/
	/Content
		Site.scss
		Common.scss // <-- this file will be not included
		/Content
			Common.scss // <-- only this
```

What about unit testing / dependency injection?
-----------------------------------------------

Well, NSass itself is very simple and just sends all the work to libsass.
So at this moment it does not contain any tests, but probably I shall add some "smoke tests" - I need to test NSass engine after changes anyway.

But, you could mock NSass classes in your tests or use your favorite IoC container for initialization.
There are two extension points - *ISassInterface* in C++/CLI code, and *ISassCompiler* in C# code, implemented as *SassInterface* and *SassCompiler* accordingly by default.
In most cases you should deal with *ISassCompiler* only.

Roadmap
-------

* Bundle&Minification support;
* NuGet packages;
* Probably, simple SASS compilation tool for Windows for usage outside of .NET world.

All the other potential features (i. e. Compass support) will be merged from libsass when they will appear there.

Known issues
------------

* libsass does not support other output styles than nested and compressed now, but declares them on interface level. You will get an exception when trying to use expanded, echo, or compact output style.
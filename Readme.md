NSass
=====

by TBAPI-0KA

About
-----

NSass is a .NET wrapper around libsass library. All information about libsass itself could be found in readme file under *NSass.LibSass* directory.

At this moment NSass is under development. By the way, the most necessary stuff is already implemented:

* simple one-to-one C++/CLI wrapper around libsass
* high-level C# wrapper (however, without much of additional features)
* HTTP handler for live SASS processing
* install all of the above throught NuGet

Main principles
---------------

* No modifications to original libsass code for easy merge;
* Simple as possible, almost without additional features;
* Easy to use.

Installation
------------

If you have web project and want live SASS processing, just install *NSass.Handler* through NuGet:

```
PM> Install-Package NSass.Handler
```

If you want to play with libsass through your own C# code, core library will be enough:

```
PM> Install-Package NSass.Core
```

[Manual build and istallation](https://github.com/TBAPI-0KA/NSass/wiki/Manual-build-and-installation)

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
* Probably, simple SASS compilation tool for Windows for usage outside of .NET world.

All the other potential features (i. e. Compass support) will be merged from libsass when they will appear there.

Known issues
------------

* libsass does not support other output styles than nested and compressed now, but declares them on interface level. You will get an exception when trying to use expanded, echo, or compact output style.
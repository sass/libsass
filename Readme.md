NSass
=====

by TBAPI-0KA

About
-----

NSass is a .NET wrapper around libsass library. All information about libsass itself could be found in readme file under NSass.LibSass directory.

At this moment NSass is under development and contains only very simple one-to-one C++/CLI wrapper around libsass. But you may use it for your own higher-level wrapper.

Main principles
---------------

* No modifications to original libsass code for easy merge;
* Simple as possible, almost without addtional features;
* Easy to use.

Roadmap
-------

* High-level C# wrapper;
* HttpHandler;
* Bundle&Minification support;
* Probably, simple SASS compilation tool for Windows for usage outside of .NET world.

All the other potential features (i. e. Compass support) will be merged from libsass when they will appear there.
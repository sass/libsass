## LibSass and file-watchers

Currently and in the foreseeable future LibSass will not support any file-watching mode.
Such a feature was always intended to be implemented by downstream consumers. In order
to support this feature, LibSass compilation phase is split into different phases.

LibSass supports to query all files included in a specific compilation. With the help
of this list, consumers can setup file-watching for all involved files. But note that
this list can change on any change, as the user may add or remove imports. This has to
be checked by consumers between different compilations. Normally the flow would look
something like this:

- Setup compiler with entry point
- Get initial list of relevant files
- Optionally compile it when starting-up
- On any change in any of the given files
  - Recompile the entry point and write result
  - Get the involved file-list of the entry-point
  - Re-initialize watcher if relevant files changed

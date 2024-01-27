# How LibSass handles variables, functions and mixins

This document is intended for developers of LibSass only and are of no use
for implementers. It documents how variable stacks are implemented.

## Foreword

LibSass uses an optimized stack approach similar to how C compilers have always
done it, by using a growable stack where we can push and pop items. Unfortunately
Sass has proven to be a bit more dynamic than static optimizers like, therefore we
had to adopt the principle a little to accommodate the edge-cases due to this.

There are three different kind of entities on the stack during runtime, namely
variables, functions and mixins. Each has its own dedicated stack to optimize
the lookups. In this doc we will often only cover one case, but it should be
applicable to any other stack object (with some small differences). Variables
are the most complicated ones, as functions and mixins can only be declared
on the root scope, so loops or functions don't need to be considered for them.

Also for regular sass code and style-rules we wouldn't need this setup, but
it becomes essential to correctly support mixins and functions, since those
can be called recursively. It is also vital for loops, like `@for` or `@each`.

## Overview

The whole process is split into two main phases. In order to correctly support
`@import` we had to introduce the preloader phase, where all `@use`, `@forward` and
`@import` rules are loaded first, before any evaluation happens. This ensures that
we know all entities before the evaluation phase in order to correctly setup
all stack frames before populating them.

### Parser/EnvFrame phase

During parsing every potential scope block creates an `EnvFrame`, which is
stored at the corresponding ast-node (e.g. on a `StyleRule`). The `EnvFrame`
is designed as a RAII stack object. It adds itself to the frame-stack on
creation and removes itself once it goes out of scope. Additionally it
creates an `EnvRefs` instance on the heap, which is later used by the
compile/evaluation phase.

### Compiler/EnvScope phase

The `EnvScope` is similar to the `EnvFrame`, as it is also designed to be a RAII
stack object. On creation it will increase the stack for each entity and update
the corresponding offset pointers and reverts it when it goes out of scope.

### EnvRefs heap object

The `EnvRefs` object is the main object holding all the information about a
block scope. It mainly holds three (flat) maps, one for every entity type.
These maps are used to resolve a name to an integer offset. Whenever a new
entity (e.g. variable assignment), the item is added to the map if it does
not already exist there and the offset is simply increased (size of the map).

### EnvRoot object

The `EnvRoot` is the main object where entities are stored. It mainly holds
two stack vectors for every entity type. The actual stack vector and an
additional vector holding the previous stack size (or offset position).
Further it holds on to all created `EnvRefs` and all built-in entities.

### Frame offset pointers

Every `EnvRefs` object get a unique number, increasing from 0 (which is
reserved for the root scope block). The `EnvRoot` objects has a vector
containing all `EnvRefs` which should correspond to that index offset.

## Basic example

Let's assume we have the following scss code:

```scss
$a: a;
b {
  $a: b;
}
```

### Parsing phase

The parser will first initialize the `EnvRoot` with the root `EnvRefs`.
First it will parse the top variable assignment, which will create a new
entry in the variable map of the `EnvRefs` heap object.

It will then parse the `StyleRule` and create a `EnvFrame` (which will also
create and register a new `EnvRefs` heap object). It will then parse the inner
assignment and create a new entry in the new `EnvRefs` of the `StyleRule`.

### Evaluation phase

First the compiler will create an `EnvScope` stack object, which will increase
the stack size of `EnvRoot` accordingly. In this case it will increase the size
of `varStack` by one to accommodate the single local variable. Note that the
variable object itself is still undefined at this point, but the slot on the
stack exists now. The `EnvScope` will also remember that it has to decrease
that stack vector by one, once it goes out of scope.

On the assignment the compiler knows that variable `$a` can be found at the
local offset `0` (as stored within the `EnvRefs` map). Now it only needs to
add the current `EnvRefs` position on the `varStack` to get the absolute
address of the requested variable.



 when it sees `b`, it will
create and assign a new `EnvFrame` with the `StyleRule`. Each `EnvRefs`
will have one Variable `$a` with the offset `0`. On runtime the compiler will
first evaluate the top assignment rule, thus assigning the string `a` to the
variable at offset `0` at the current active scope (root). Then it evaluates
the ruleset and creates a new `EnvScope`, which will push new instances onto
the env-stack for all previously parsed entities (one variable in this case).

We now have two variables on the actual env-scope with the inner still undefined.


This will allocate two independent variables on the stack. For easier reference
we can think of them as variable 0 and variable 1. So let's see what happens if
we introduce some VariableExpressions:

```scss
$a: 1;
b {
  a0: $a;
  $a: 2;
  a1: $a;
}
c {
  a: $a;
}
```

As you may have guesses, the `a0` expression will reference variable 0 and the
`a1` expression will reference variable 1, while the last one will reference
variable 0 again. Given this easy example this might seem overengineered, but
let's see what happens if we introduce a loop:

```scss
$a: 1;
b {
  @for $x from 1 through 2 {
    a0: $a;
    $a: 2;
    a1: $a;
  }
}
c {
  a: $a;
}
```

Here I want to concentrate on `a0`. In most programing languages, `a0: $a` would
always point to variable 0, but in Sass this is more dynamic. It will actually
reference variable 0 on the first run, and variable 1 on consecutive runs.

## What is an EnvFrame and EnvRef

Whenever we encounter a new scope while parsing, we will create a new EnvFrame.
Every EnvFrame (often also just called idxs) knows the variables, functions and
mixins that are declared within that scope. Each entity is simply referenced by
it's integer offset (first variable, second variable and so on). Each frame is
stored as long as the context/compiler lives. In order to find functions, each
frame keeps a hash-map to get the local offset for an entity name (e.g. varIdxs).
An EnvRef is just a struct with the env-frame address and the local entity offset.

## Where are entities actually stored during runtime

The `EnvRoot` has a growable stack for each entity type. Whenever we evaluate
a lexical scope, we will push the entities to the stack to bring them live.
By doing this, we also update the current pointer for the given env-frame to
point to the correct position within that stack. Let's see how this works:

```scss
$a: 1;
@function recursive($abort) {
  $a: $a + 1;
  @if ($abort) {
    @return $a;
  }
  @else {
    @return recursive(true);
  }
}
a {
  b: recursive(false);
}
```

Here we call the recursive function twice, so the `$a` inside must be independent.
The stack allocation would look the following in this case:

- Entering root scope
  - pushing one variable on the runtime var stack.
- Entering for scope for the first time
  - updating varFramePtr to 1 since there is already one variable.
  - pushing another variable on the runtime var stack
- Entering for scope for the second time
  - updating varFramePtr to 2 since there are now two variable.
  - pushing another variable on the runtime var stack
- Exiting second for scope and restoring old state
- Exiting first for scope and restoring old state
- Exiting root scope

So in the second for loop run, when we have to resolve the variable expression for `$a`,
we first get the base frame pointer (often called stack frame pointer in C). Then we only
need to add the local offset to get to the current frame instance of the variable. Once we
exit a scope, we simply need to pop those entities off the stack and reset the frame pointer.

## How ambiguous/dynamic lookup is done in loops

Unfortunately we are not able to fully statically optimize the variable lookups (as explained
earlier, due to the full dynamic nature of Sass). IMO we can do it for functions and mixins,
as they always have to be declared and defined at the same time. But variables can also just
be declared and defined later. So in order to optimize this situation we will first fetch and
cache all possible variable declarations (vector of VarRef). Then on evaluation we simply need
to check and return the first entity that was actually defined (assigned to).

## Afterword

I hope this example somehow clarifies how variable stacks are implemented in LibSass. This
optimization can easily bring 50% or more performance in contrast to always do the dynamic
lookup via the also available hash-maps. They are needed anyway for meta functions, like
`function-exists`. It is also not possible to (easily) create new variables once the parsing
is done, so the C-API doesn't allow to create new variables during runtime.

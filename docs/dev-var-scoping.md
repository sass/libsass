# Sass variable scoping

Variable scoping in Sass has some quirks which I'll try to
explain in this document. This partially also applies to
functions and mixins, as they are scope objects like variables
too. But they can only be defined on the root scope, so their
logic is not that complicated as variables.

## Variable declarations in Sass

Sass doesn't have explicit variable declarations. Variables
are declared whenever an assignment to a variable happens. It
is then local to the scope block the assignment was in. E.g.
every ruleset (opened via `{` in scss) is a new scope block.

On the surface this makes variable scoping quite simple, as
declarations always happen with the definition of the variable.
Under the hood it has some quirks, mainly in combination with loops.

## Block scopes and (semi) transparent loops

Consider the following example:

```scss
$a: 0;
@for $i from 1 through 3 {
  @debug $a;
  $a: $i;
}
@debug $a
```

The first `@debug` calls should be easy to guess (0,1,2). But the
last `@debug` is a bit trickier. In this case the `@for` loop is
transparent and the assignment inside it is made to the outer
variable on the root scope and will be reported as `3`.

Now lets wrap the loop into a ruleset scope:

```scss
$b: 0;
a {
  @for $i from 1 through 3 {
    @debug $b;
    $b: $i;
  }
  @debug $b
}
```

Again, the inner `@debug` calls are simply 0,1 and 2 and the
tricky question is again what the last `@debug` call will yield?
You might have guess it will report `3` again, but that is not
correct, as Sass will report `0` here.

Conclusion: Loops like `@for`, `@each` and `@while` are transparent
only on the root-scope, but not on any inner scopes.

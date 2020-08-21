# Examples ![examples](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/examples/badge.svg?branch=master)
To benchmark the implementation and also to provide examples of how to use
_COOM_ in your project, we provide multiple examples

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Examples](#examples)
    - [N-Queens](#n-queens)

<!-- markdown-toc end -->

## N-Queens (Simple)

**Files:** `n_queens.cpp`

**Target:** `make example-n-queens N=<?>`

Solve the N-Queens problem for _N = `?`_ (default: `8`). This is done by
constructing an OBDD `amo` (at-most-one), that checks whether an assignment has
any conflicts, and an OBDD `alo` (at-least-one), that checks whether at least
one queen is placed on each row. On the combined OBDD one then counts the number
of satisfying assignments.

To list the solutions, we take the same OBDD, but instead of count the number of
assignments within, we use it to prune a recursive enumeration of all possible
queen placements. For up to _N = 8_ we run both the solution counting procedure
and the enumeration procedure.

**Notice**: This example is primarily meant to show how to use the _COOM_
library, not to be efficient. At _N = 11_ the intermediate size explodes
uncotrollably, with some intermediate results reaching 35 GB! For a better
solution, consult the ideas of issue #42 .

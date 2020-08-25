# Examples ![examples](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/examples/badge.svg?branch=master)
To benchmark the implementation and also to provide examples of how to use
_COOM_ in your project, we provide multiple examples

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Examples](#examples)
    - [N-Queens](#n-queens)

<!-- markdown-toc end -->

## N-Queens

**Files:** `n_queens.cpp`

**Target:** `make example-n-queens N=<?>`

Solve the N-Queens problem for _N = `?`_ (default: `8`). This is done by
constructing an OBDD row-by-row that represents whether the row is a legal
state: is at least one queen placed on each row and it is also in no conflicts
with any other? On this OBDD we then counts the number of satisfying
assignments.

To list the solutions, we take the same OBDD, but instead of count the number of
assignments within, we use it to prune a recursive enumeration of all possible
queen placements. For up to _N = 8_ we run both the solution counting procedure
and the enumeration procedure.

**Notice**: This is a pretty simple example and has all of the normal
shortcomings for OBDDs trying to solve the N-Queens problem. At around _N_ = 14
the intermediate sizes explodes a lot. One can with about 100 GB of disk space
or memory compute it, but for a better solution, consult the ideas of issue #42 .
.

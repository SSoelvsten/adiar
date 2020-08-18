# Examples ![examples](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/examples/badge.svg?branch=master)
To benchmark the implementation and also to provide examples of how to use
_COOM_ in your project, we provide multiple examples

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Examples](#examples)
    - [N-Queens](#n-queens)

<!-- markdown-toc end -->

## N-Queens : Counting assignments

**Files:** `n_queens.cpp`

**Target:** `make example-n-queens N=<?>`

Solves the N-Queens problem for _N = `?`_ (default: `8`). This is done by
constructing an OBDD `amo` (at-most-one), that checks whether an assignment has
any conflicts, and an OBDD `alo` (at-least-one), that checks whether at least
one queen is placed on each row. On the combined OBDD one then counts the number
of satisfying assignments.

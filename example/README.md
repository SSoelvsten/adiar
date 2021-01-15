# Examples
![examples](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/examples/badge.svg?branch=master)

To benchmark the implementation and also to provide examples of how to use
_Adiar_ in your project, we provide multiple examples.

All examples have an _M_ variable to be set for the GB of memory to dedicate to
TPIE managing all the files underneath.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Examples](#examples)
    - [N-Queens](#n-queens)
    - [Tic-Tac-Toe](#tic-tac-toe)
    - [References](#references)

<!-- markdown-toc end -->

## N-Queens

**Files:** `n_queens.cpp`

**Target:** `make example-n-queens N=<?>`

Solve the N-Queens problem for _N = `?`_ (default: `8`). This is done by
constructing an OBDD row-by-row that represents whether the row is in a legal
state: is at least one queen placed on each row and is it also in no conflicts
with any other? On this OBDD we then counts the number of satisfying
assignments [[kunkle10](#references)].

To list the solutions, we take the same OBDD, but instead of counting the number
of assignments within, we use it to prune a recursive enumeration of all
possible queen placements. For up to _N = 8_ we run both the solution counting
procedure and the enumeration procedure.

**Notice**: This is a pretty simple example and has all of the normal
shortcomings for OBDDs trying to solve the N-Queens problem. At around _N_ = 14
the intermediate sizes explodes a lot. One can with about 100 GB of disk space
or memory compute it, but for a better solution, consult the ideas of issue
[#42](https://github.com/SSoelvsten/cache-oblivious-obdd/issues/42).


## Pigeonhole Principle

**Files:** `pigeonhole_principle.cpp`

**Target:** `make example-pigeonhole-principle N=<?>`

Proofs that there does not exist an isomorphism between {1, 2, ... , N+1} and {
1, 2, ..., N } based on [[Tveretina10](#references)]. This is disproven by use
of variables _P<sub>i,j</sub>_ saying that the _i_'th pigeon is mapped to the
_j_'th hole. These are written as a CNF formula with at-least-one and
at-most-one constraints. This CNF is then solved in a as described in [[Pan04;
Section 2](#references)]: sort all clauses by the maximal occuring literal into
clusters and then interchangably accumulate the conjunction of clauses and
existentially quantify the variables.


## Tic-Tac-Toe

**Files:** `tic_tac_toe.cpp`

**Target:** `make example-tic-tac-toe N=<?>`

Counts the number of possible ties in a 3D 4x4x4 Tic-Tac-Toe game where Player 1
has placed _N = `?`_ (default: `20`) crosses and Player 2 has placed noughts on
the remaining cells. Starting from an OBDD that is only true, if exactly N
crosses are placed, the constraints of at least one cross and one nought for all
rows, columns, and diagonals are added to it. On this OBDD we then counts the
number of satisfying assignments [[Kunkle10](#references)].

## References

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “_Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams_”. In: _PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation_. 2010

- [[Pan04](https://link.springer.com/chapter/10.1007/11527695_19)] Guoqiang
  Pan and Moshe Y. Vardi. “_Search vs. Symbolic Techniques in Satisfiability
  Solving_”. In: _SAT 2004: Theory and Applications of Satisfiability Testing_.
  2004

- [[Tveretina10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Olga
  Tveretina, Carsten Sinz and Hans Zantema. “_Ordered Binary Decision Diagrams,
  Pigeonhole Formulas and Beyond_”. In: _Journal on Satisfiability, Boolean
  Modeling and Computation 1_. 2010

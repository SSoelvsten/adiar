\mainpage Main Page

Adiar is a library for manipulating Decision Diagrams even when these grow
bigger than the memory one has available. To achieve this, all operations are
not implemented recursively with a shared unique node table, but instead they
are created as streaming algorithms that exploit a specific sorting of diagram
nodes on disk to delay recursion.

<div align="center">
  <img src="queens.png"
       alt="Running Time of Adiar and other BDD packages solving the N-Queens problem"
       style="max-width:32rem; width:32rem"
/>
</div>
<div align="center">
  **Figure**: Running time solving N -Queens (lower is better).
</div>

Quick Access
========================

1. Prerequisites
   - \ref page__getting_started
2. Decision Diagrams
   - \ref module__bdd module
   - \ref module__zdd module
3. \ref module__builder module
4. \ref module__statistics module
5. \ref page__tutorial
6. \ref page__papers
   - \ref page__cite
   - \ref page__amend

About the Project
========================

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

**Current Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

License
========================

The software and documentation files in this repository are distributed under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/main/LICENSE.md).

Using Adiar will indirectly use [TPIE](https://github.com/thomasmoelhave/tpie)
underneath, which in turn is licensed under the _LGPL v3_ license. Hence, a
binary of yours that is statically linked to Adiar will be affected by that
license. That is, if you share that binary with others, then you will be obliged
to make the source public. This can be resolved by using Adiar as a shared
library or have it use an alternative to TPIE, such as
[STXXL](https://stxxl.org/).

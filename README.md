# Cache-Oblivious OBDD Manipulation
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Cache-Oblivious OBDD Manipulation](#cache-oblivious-obdd-manipulation)
    - [Introduction](#introduction)
    - [Risks, Discussion and Future Work](#risks-discussion-and-future-work)

<!-- markdown-toc end -->

## Introduction
While at a research meeting I asked whether really _noone has yet been looking
into Cache-Oblivious OBDD manipulation_, which after a bit of searching turned
up a paper by Lars Arge from 1996 about external-memory OBDD manipulation. The
main points of the paper is:

- The DFS, BFS and other algorithms at the time had a worst case _O(N)_ I/O
  complexity.

- The lower bound for both a _Reduce_ is _O(sort(N))_. An _Apply_ that will have
  to output a minimal OBDD has by extension the same bound.

- It provides an I/O optimal external-memory algorithm for both _Reduce_ and
  _Apply_

  - If the OBDD is represented by the _low_ and _high_ arcs, then the output of
    _Reduce_ is already in sorted order for the _Apply_ algorithm, and vica
    versa.

Looking at the algorithm in [Arge96] you will further notice the following:

- The algorithm does not explicitly make use of _M_ or _B_ but only an I/O
  efficient sorting algorithm and priority queue. Both of these can be
  substituted for a _Cache-oblivious_ or _Cache-aware_ instance to immediately
  yield an optimal algorithm of both types!

## Risks, Discussion and Future Work
Immediate risks of this project is that since _Reduce_ and _Apply_ are separate
algorithms, then the implementation might end up much slower due to the
intermediate size of the OBDD or by having to run two algorithms separately.

This results in the following questions to improve the speed
- [ ] Can we circumvent this with pipelining (e.g.
      [TPIE](https://github.com/thomasmoelhave/tpie))?

    - [X] The output of _Apply_ and _Substitute_ directly generate the input of
          both the work list and the transposed graph of dependencies for the
          following _Reduce_. This definitely solves the need to sort the input
          at the beginning of every algorithm.

    - [ ] While the output is already sorted for the next algorithm, the
          computation order of every algorithm still is in reverse of the input
          given. That is, _Apply_ has to finish processing before the following
          _Reduce_ can start and vica versa. Is it possible though to output in
          the order used for computation to make them all into streaming
          algorithms?

- [ ] The idea of Time-Forward-Processing is inherently sequential, but can we
      parallelise parts of the algorithm?
      
    - Need to investigate the following data structures and algorithms:
      
      - [X] Parallelisable sorting algorithms (Sorting in TPIE is parallelised)
      - [ ] Parallelisable priority queue

    - In the _Reduce_ algorithm:

      - [ ] Parallelise applying the reduction at each layer, since at that
            point all information is already given and it would then be
            _p_-parallel scans through the list.

    - In the _Apply_ algorithm:

      - [ ] Notice at the time of processing a resulting layer (i.e. consider
            the output label), all requests for that layer already have been
            added to the queue. All requests at said layer are of the form _(v₁,
            v₂)_ with label _min(v₁.label, v₂.label)_ and will always create
            requests for nodes with a strictly higher level. So if each worker
            process can be fed with information from the request-queue, then
            they can work in parallel, as long as the output is sorted for the
            later _Reduce_.

- [ ] Can the algorithm or parts of it be done on the much faster GPU?

    - [ ] Sorting Algorithm
    - [ ] Priority Queue (e.g. [CUPQ](https://github.com/crosetto/cupq))

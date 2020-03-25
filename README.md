# Cache-Oblivious OBBD Manipulation
While at a research meeting I asked the whether really _noone has yet been
looking into Cache-Oblivious OBBD manipulation_, which after a bit of searching
turned up a paper by Lars Arge from 1996 about external-memory OBBD
manipulation. The main points of the paper is:

- The DFS, BFS and other algorithms at the time had a worst case _O(N)_ I/O
  complexity.

- The lower bound for both a _Reduce_ is _O(sort(N))_. An _Apply_ that will have
  to output a minimal OBBD has by extension the same bound.

- It provides an I/O optimal external-memory algorithm for both _Reduce_ and
  _Apply_

  - If the OBBD is represented by the _low_ and _high_ arcs, then the output of
    _Reduce_ is already in sorted order for the _Apply_ algorithm, and vica
    versa.

Looking at the algorithm in [Arge96] you will further notice the following:

- The algorithm does not explicitly make use of _M_ or _B_ but only an I/O
  efficient sorting algorithm and priority queue. Both of these can be
  substituted for a _Cache-oblivious_ or _Cache-aware_ instance to immediately
  yield an optimal algorithm of both types!

## Risks and Discussion
Immediate risks of this project is that since _Reduce_ and _Apply_ are separate
algorithms, then the implementation might end up much slower due to the
intermediate size of the OBBD or by having to run two algorithms separately.

This results in the following questions to improve the speed
- [ ] Can we circumvent this with pipelining (e.g.
      [TPIE](https://github.com/thomasmoelhave/tpie))? This seems not that
      trivial, since we cannot start _Reduce_ before the whole result of the
      _Apply_ algorithm is outputted, and vica versa.

- [ ] The output of the _Apply_ is already in sorted order for the _Reduce_, and
      vica. This results in one fewer steps of sorting and a speedup.

  - [ ] The sorted list of whereto computation has to be forwarded in the
        _Reduce_ algorithm can be replaced by a priority queue which can already
        be filled up during the _Apply_ computation. Or is such a queue so heavy
        a sorting of a massive input afterwards is quicker?

- [ ] The idea of Time-Forward-Processing is inherently sequential, but can we
      parallelise parts of the algorithm?

    - [ ] Parallelisable sorting algorithms?
    - [ ] Parallelisable priority queue?

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

## Repository Structure
The repository has different folders with different aspects of this project,
ranging from articles, report, through to implementation.

### Bibliography
Copies of the most relevant papers are placed here for convenience.

### Pseudocode
To better understand the algorithm in detail, the _Reduce_ and _Apply_ algorithm
have been written as pseudocode. These are _python_ files for the syntax
highlighting, but that does not mean they are supposed to be compilable and
possible to run. These have then been extended with other commonly used
algorithms.

### Report
The LaTeX source for a report on this project. This could be repurposed into a
paper in the event the benchmark results are promising.

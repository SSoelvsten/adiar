# Cache-Oblivious OBBD Manipulation
While at research meeting I asked the whether really _noone has yet been looking
into Cache-Oblivious OBBD manipulation_, which after a bit of searching turned
up a paper by Lars Arge from 1996 about external-memory OBBD manipulation. The
main points of the paper is:

- The DFS, BFS and other algorithms at the time had a worst case `O(N)` I/O
  complexity.
- The lower bound for both a _Reduce_ and _Apply_ algorithm is `O(sort(N)) =
  O(N/B log_{M/B}(N/B))`
- It provides an I/O optimal external-memory algorithm for both _Reduce_ and
  _Apply_
- If the OBBD is represented by the _low_ and _high_ arcs, then the output of
  _Reduce_ is already in sorted order for the _Apply_ algorithm, and vica versa.
  
Looking at the algorithm you will further notice the following:

- The algorithm does not explicitly make use of `M` or `B` but only an I/O
  efficient sorting algorithm and priority queue. Both of these can be
  substituted for a _Cache-oblivious_ instance to immediately yield a
  cache-oblivious algorithm!

Immediate risks of this project are is that since _Reduce_ and _Apply_ are
separate algorithms, then the implementation might end up much slower due to the
intermediate size of the OBBD or by having to run two algorithms separately.

- The major question is whether this can be circumvented with pipelining (e.g.
  [TPIE](https://github.com/thomasmoelhave/tpie)). Even though before starting
  the _Reduce_ the whole result of the _Apply_ algorithm has to be outputted,
  and vica versa, can we set off the cost by doing some of the things below?
  - The next algorithm does not sort its input (as it already is outputted in
    sorted order)
  - The sorted list of whereto computation has to be forwarded in the _Reduce_
    algorithm can be replaced by a priority queue which can already be filled up
    during the _Apply_ computation.
    - Or would this result in more cache-misses of the _Apply_ algorithm and a
      sorting of a massive input afterwards is __always__ quicker?

## Bibliography
Copies of the most relevant papers are placed here for convenience.

## Paper
The LaTeX source for the (knock-on-wood) resulting paper.


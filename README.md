# Cache-Oblivious OBDD Manipulation
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)
![test](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/test/badge.svg?branch=master)

Following up on the work of [[Arge96](/bib/%5Barge%5D%20IO%20Complexity%20of%20OBDD%20Manipulation.pdf)],
this project investigates the use of _Time-Forward Processing_ to improve the
I/O complexity and performance of OBDD Manipulation.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Cache-Oblivious OBDD Manipulation](#cache-oblivious-obdd-manipulation)
    - [Introduction](#introduction)
    - [Repository Content](#repository-content)
        - [LaTeX](#latex)
        - [C++ implementation](#c-implementation)
    - [Future Work](#future-work)
        - [Use non-comparison based sorting on numbers](#use-non-comparison-based-sorting-on-numbers)
        - [Pipelining](#pipelining)
        - [Parallelisation](#parallelisation)
            - [Delegation of tasks to the GPU](#delegation-of-tasks-to-the-gpu)
    - [References](#references)

<!-- markdown-toc end -->

## Introduction
In [[Arge96](/bib/%5Barge%5D%20IO%20Complexity%20of%20OBDD%20Manipulation.pdf)],
Lars Arge concludes that the I/O complexity of the DFS, BFS and other algorithms
are _O(N)_, while he proves that the _Reduce_ has an _Ω(sort(N))_ I/O lower
bound. An _Apply_ that will have to output a minimal OBDD has by extension the
same bound.

He then provides a description of an I/O optimal external-memory algorithm for
both _Reduce_ and _Apply_. It makes use of _Time-Forward Processing_ to delay
recursion until the needed data is in memory. These algorithms are designed,
sucht that the output of _Reduce_ is already in sorted order for the following
_Apply_ algorithm, and vica versa. When further looking into these algorithms,
we notice, that the algorithm does not explicitly make use of _M_ or _B_ but
only an I/O efficient sorting algorithm and priority queue. Both of these can be
substituted for a _Cache-oblivious_ or _Cache-aware_ instance to immediately
yield an optimal algorithm of both types!

Following up on Arge's work, we extend this approach to other core OBDD
algorithms and implement it in C++ to benchmark the performance in practice
compared to conventional recursive procedures.

## Repository Content
The repository contains multiple works on the same

### LaTeX
This folder contains LaTeX figures, listings and reports that describes the
algorithm and on a more abstract level. The LaTeX projects have immediate
dependencies to my [LaTeX preamble](https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples),
which should be cloned next to this one.

### C++ implementation
The `./src/` folder contains a _C++_ implementation of the proposed algorithm,
which are accompanied by unit tests in `./test/`. Every commit on master and
pull request submitted will automatically have the test-suite run.

The project makes use of the following dependencies
- [TPIE](https://github.com/thomasmoelhave/tpie):
  Framework for implementation of I/O efficient algorithms. It directly provides
  sorting algorithms and a priotity queue. Both are much faster than the
  algorithms in the _C++_ standard library
  [[Mølhave](bib/%5Bm%C3%B8lhave%5D%20Using%20TPIE%20for%20processing%20massive%20data%20sets%20in%20C%2B%2B.pdf)].

- [Bandit](https://github.com/banditcpp/bandit):
  Writing and running unit tests

All dependencies are directly imported as submodules. If you have not cloned the
repository recursively, then run the following commands
```bash
git submodule init
git submodule update
```

The root _Makefile_ provides the following for targets

| Target | Effect             |
|--------|--------------------|
| `test` | Run all test cases |

## Future Work
Contributions are very welcome. If one investigates possible extensions and
optimisations, and they prove fruitful, then please submit a pull request! The
following are

### Integrate reduction into the other algorithms
One major issue with the _Time-Forward Processing_ approach is that the
intermediate graph after the _Apply_ and before its subsequent _Reduce_ can be
quadratic in size. That is, given two OBBDs of size _10⁹_ (24GB each) will
before the reduce possibly be _10¹⁸_! The same also applies for the
_Substitute_ and _RelProd_ algorithms etc.

### Use non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe not due to the _O(sort(N))_ I/O lower bound.

### Pipelining
Can we fully embrace the idea of pipelining by making the algorithms streamable
 (e.g. [TPIE](https://github.com/thomasmoelhave/tpie))?

- [ ] While the output is already sorted for the next algorithm, the computation
      order of every algorithm still is in reverse of the input given. That is,
      _Apply_ has to finish processing before the following _Reduce_ can start
      and vica versa. Is it possible though to output in the order used for
      computation to make them all into streaming algorithms?

### Parallelisation
The idea of Time-Forward-Processing is inherently sequential, but can we
parallelise parts of the algorithm? We would first need to investigate
parallelising the underlying data structures and algorithms:

  - [X] Parallelisable sorting algorithms (Sorting in TPIE is parallelised)
  - [ ] Parallelisable priority queue, such as the one in
        [[Sitchinava12](/bib/%5Bsitchinava%5D%20A%20Parallel%20Buffer%20Tree.pdf)]

Using these we have the following ideas for parallelisation.

- In the _Reduce_ algorithm:

  - [ ] The early application of Reduction Rule 1
        [[Bryant86](/bib/%5Bbryant%5D%20Graph-Based%20Algorithms%20for%20Boolean%20Function%20Manipulation.pdf)]
        can be done after the merge with children from the queue, but be placed
        in a seperate list. Then the priority queue can be filled with these
        elements simultaneously with the rest being processed for Reduction Rule
        2.

  - [ ] Parallelise applying the reduction at each layer, since at that
        point all information is already given and it would then be
        _p_-parallel scans through a single list.

- In the _Apply_ algorithm:

  - [ ] Notice at the time of processing a resulting layer (i.e. consider
        the output label), all requests for that layer already have been
        added to the queue. All requests at said layer are of the form _(v₁,
        v₂)_ with label _min(v₁.label, v₂.label)_ and will always create
        requests for nodes with a strictly higher level. So if each worker
        process can be fed with information from the request-queue, then
        they can work in parallel, as long as the output is sorted for the
        later _Reduce_.

#### Delegation of tasks to the GPU
Can the algorithm or parts of it be moved to the much faster GPU? By the design
of the algorithms, every layer in the OBDD is processed independently of each
other. The CPU can merge the Priority Queue with the whole layer to then
delegate the processing of the whole layer to the GPU.

Data structures to look into:

- [ ] Sorting Algorithm. Here one might want to explot the avenue of
      [non-comparison based sorting](#use-non-comparison-based-sorting-on-numbers)
      at the same time.

- [ ] Priority Queue. This one can then be used to forward information across
      the layer inside the GPU. This also improves the speed and size of the
      priority queue in the main memory that would only be used for
      communication between layers.

## References
The references given below and other relevant papers can be found in the _bib_
folder.

- [[Arge96](/bib/%5Barge%5D%20IO%20Complexity%20of%20OBDD%20Manipulation.pdf)]
  The I/O-Complexity of Ordered Binary-Decision Diagram Manipulation

- [[Bryant86](/bib/%5Bbryant%5D%20Graph-Based%20Algorithms%20for%20Boolean%20Function%20Manipulation.pdf)]
  Graph-Based Algorithms for Boolean Function Manipulation

- [[Mølhave](/bib/%5Bm%C3%B8lhave%5D%20Using%20TPIE%20for%20processing%20massive%20data%20sets%20in%20C%2B%2B.pdf)]
  Using TPIE for Processing Massive Data Sets in C++

- [[Sitchinava12](/bib/%5Bsitchinava%5D%20A%20Parallel%20Buffer%20Tree.pdf)]
  A Parallel Buffer Tree

# COOM: Cache-Oblivious OBDD Manipulation
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)
[![Built with TPIE](https://img.shields.io/badge/built%20with-TPIE-blue)](https://users-cs.au.dk/~rav/tpie/)
[![test](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/test/badge.svg?branch=master)](/actions?query=workflow%3Atest)
[![docs](https://github.com/SSoelvsten/cache-oblivious-obdd/workflows/docs/badge.svg?branch=master)](/actions?query=workflow%3Adocst)

Following up on the work of [[Arge96](#references)], this project investigates
the use of _Time-Forward Processing_ to improve the I/O complexity and
performance of OBDD Manipulation.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [COOM: Cache-Oblivious OBDD Manipulation](#coom-cache-oblivious-obdd-manipulation)
    - [Introduction](#introduction)
    - [C++ implementation](#c-implementation)
        - [Dependencies](#dependencies)
        - [Building](#building)
    - [Documentation](#documentation)
        - [Dependencies](#dependencies-1)
    - [Future Work](#future-work)
        - [Integrate reduction into the other algorithms](#integrate-reduction-into-the-other-algorithms)
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
such that the output of _Reduce_ is already in sorted order for the following
_Apply_ algorithm, and vica versa. When further looking into these algorithms,
we notice, that the algorithm does not explicitly make use of _M_ or _B_ but
only an I/O efficient sorting algorithm and priority queue. Both of these can be
substituted for a _Cache-oblivious_, such as [[Sanders01](#references)], or
_Cache-aware_ instance to immediately yield an optimal algorithm of both types!

Following up on Arge's work, we extend this approach to other core OBDD
algorithms and implement it in C++ to benchmark the performance in practice
compared to conventional recursive procedures.

## C++ implementation
The `./src/` folder contains a _C++_ implementation of the proposed algorithm,
which are accompanied by unit tests in `./test/`. Every commit on master and
pull request submitted will automatically have the test-suite run.

The project makes use of the following dependencies
- [TPIE](https://github.com/thomasmoelhave/tpie):
  Framework for implementation of I/O efficient algorithms. It directly provides
  sorting algorithms and a priotity queue. Both are much faster than the
  algorithms in the _C++_ standard library
  [[Mølhave12](#references)].

- [Bandit](https://github.com/banditcpp/bandit):
  Writing and running unit tests

### Dependencies
All dependencies are directly imported as submodules. If you have not cloned the
repository recursively, then run the following commands

```bash
git submodule init
git submodule update
```

One also needs a _C++_ compiler of ones choice. All development has currently
been with the _g++_ compiler, so we cannot guarantee other compilers will work
out-of-the-box. The project also has dependencies on _CMake_, _Boost Library_,
and _Aptitude_. On Ubuntu 18+ you can obtain all these dependencies with the
following commands.

```bash
apt install g++
apt install cmake
apt install libboost-all-dev
apt install aptitude
```

### Building
The project is build with _CMake_, though for convenience I have simplified the
_CMake_ interactions to a single `make` use of the root _Makefile_ which works
on a local machine. This has only been tested on `Ubuntu 18.04 LTS` and does
depend on `apt`.
```python
+ build                # Build everything
|- build-test          # Build unit tests

+ clean                # Clean 'build/' folder

+ test                 # Compile and run tests

+ setup                # Fetch all project dependencies
|- setup-c             # Run 'sudo apt get' on all C++ library dependencies
|- setup-submodules    # Setup and update all submodules
```

## Documentation
The primary documentation is provided as a technical report written in LaTeX. It
provides figures, listings that describes the algorithm on an abstract level
together with a description of and benchmarks of the implementation.

### Dependencies
The LaTeX documents have immediate dependencies to my [LaTeX
preamble](https://github.com/SSoelvsten/LaTeX-Preamble_and_Examples), which is
included as a submodule dependency (See [C++/Dependencies](#dependencies)).


## Future Work
Contributions are very welcome. If one investigates possible extensions and
optimisations, and they prove fruitful, then please submit a pull request! The
following are ideas for avenues to explore.

### Add Complement Edges
Currently we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique.
[[Brace90](#references)]

### Integrate reduction into the Apply algorithm
One major issue with the _Time-Forward Processing_ approach is that the
intermediate graph after the _Apply_ and before its subsequent _Reduce_ can be
quadratic in size. That is, given two OBBDs of size _10⁹_ (24GB each) will
before the reduce possibly be _10¹⁸_!

### Pipelining
Can we fully embrace the idea of pipelining by making the algorithms streamable
 (e.g. [Arge17](#references))?

- [ ] While the output is already sorted for the next algorithm, the computation
      order of every algorithm still is in reverse of the input given. That is,
      _Apply_ has to finish processing before the following _Reduce_ can start
      and vica versa. Is it possible though to output in the order used for
      computation to make them all into streaming algorithms?

### Use non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe not due to the _O(sort(N))_ I/O lower bound.

### Parallelisation
The idea of Time-Forward-Processing is inherently sequential, but can we
parallelise parts of the algorithm? We would first need to investigate
parallelising the underlying data structures and algorithms:

  - [X] Parallelisable sorting algorithms (Sorting in TPIE is parallelised)
  - [ ] Parallelisable priority queue, such as the one in
        [[Sitchinava12](#references)]

Using these we have the following ideas for parallelisation.

- In the _Reduce_ algorithm:

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

- [[Arge96](https://tidsskrift.dk/brics/article/view/20010/17643)]
  Lars Arge. “_The I/O-complexity of Ordered Binary-Decision Diagram
  Manipulation_”. In: _Efficient External-Memory Data Structures and
  Applications_. 1996

- [[Arge05](https://imada.sdu.dk/~rolf/Edu/DM808/F08/Handouts/ionotes.pdf)]
  Lars Arge. “_External Memory Geometric Data Structures_”. In: _External Memory
  Geometric Data Structures_. 2005

- [[Arge10](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5470440)]
  Lars Arge, Michael T. Goodrich, Nodari Sitchinava. “_Parallel external memory
  graph algorithms_”. In: _2010 IEEE International Symposium on Parallel
  Distributed Processing (IPDPS)_. 2010

- [[Arge17](https://pure.au.dk/portal/files/119531804/paper_bigdata17_camera.pdf)]
  Lars Arge, Mathias Rav, Svend C. Svendsen, Jakob Truelsen. “_External Memory
  Pipelining Made Easy With TPIE_”. In: _2017 IEEE International Conference on
  Big Data_. 2017

- [[Brace90](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=114826)]
  Karl S. Brace, Richard L. Rudell, and Randal E. Bryant. “_Efficient
  implementation of a BDD package_”. In: _27th ACM/IEEE Design Automation
  Conference_. 1990, pp. 40 – 45.

- [[Bryant86](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1676819)]
  Randal E. Bryant. “_Graph-Based Algorithms for Boolean Function Manipulation_”.
  In: _IEEE Transactions on Computers_. (1986)

- [[Dijk16](https://link.springer.com/content/pdf/10.1007/s10009-016-0433-2.pdf)]
  Tom van Dijk, Jaco van de Pol. “_Sylvan: multi-core framework for decision
  diagrams_”. In: _International Journal on Software Tools for Technology
  Transfer_. 2016

- [[Mølhave12](https://dl.acm.org/doi/pdf/10.1145/2367574.2367579)]
  Thomas Mølhave. “_Using TPIE for Processing Massive Data Sets in C++_”. 2012

- [[Sanders01](https://dl.acm.org/doi/pdf/10.1145/351827.384249)]
  Peter Sanders. “_Fast Priority Queues for Cached Memory_”. In: _J. Exp.
  Algorithmics 5_. 2001

- [[Sitchinava12](https://dl.acm.org/doi/pdf/10.1145/2312005.2312046)]
  Nodari Sitchinava, Norbert Zeh. “_A Parallel Buffer Tree_”. In: _Proceedings
  of the Twenty-Fourth Annual ACM Symposium on Parallelism in Algorithms and
  Architectures_. 2012

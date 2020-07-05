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
    - [Installation](#installation)
        - [Dependencies](#dependencies)
    - [Usage](#usage)
        - [Examples and benchmarks](#examples-and-benchmarks)
        - [In your own project](#in-your-own-project)
    - [Documentation](#documentation)
    - [Future Work](#future-work)
        - [Optimisations](#optimisations)
            - [Non-comparison based sorting on numbers](#non-comparison-based-sorting-on-numbers)
            - [Pipelining](#pipelining)
            - [Parallelisation](#parallelisation)
        - [Extensions](#extensions)
            - [Complement Edges](#complement-edges)
            - [Non-boolean Decision Diagrams](#non-boolean-decision-diagrams)
    - [License](#license)
    - [Credits](#credits)
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


## Installation
The algorithms are implemented in _C++_ making use of the following external
dependencies

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

## Usage
The project is build with _CMake_, though for convenience I have simplified the
_CMake_ interactions to a single _Makefile_ which works on a local machine. This
has only been tested on `Ubuntu 18.04 LTS`.

The primary targets are as follows

| target  | effect                 |
|---------|------------------------|
| `build` | Build the source files |
| `clean` | Remove all build files |
| `test`  | Run all unit tests     |


### Examples and benchmarks
The _/examples_ folder contains examples for using the data structure to solve
various verification and satisfaction problems.


### In your own project
To use this OBDD implementation in your own project, then _CMake_ still needs to
be properly configured (e.g. see issue [#4](/issues/4)). Contributions and help
to this end would be very much appreciated.


## Documentation
The primary documentation is provided as a technical report written in LaTeX. It
provides figures, listings that describes the algorithm on an abstract level
together with a description of and benchmarks of the implementation.


## Future Work
Contributions are very welcome. If one investigates possible extensions and
optimisations, and they prove fruitful, then please submit a pull request! The
following are ideas for avenues to explore.

### Optimisations

#### Non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe not due to the _O(sort(N))_ I/O lower bound.

#### Pipelining
While the output is already sorted for the next algorithm, the computation order
of every algorithm still is in reverse of the input given. That is, _Apply_ has
to finish processing before the following _Reduce_ can start and vica versa. Is
it possible though to output in the order used for computation to make them all
into streaming algorithms and with that fully embrace the idea of pipelining
(e.g. [Arge17](#references))?

#### Parallelisation
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

**Delegation of tasks to the GPU**

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

### Extensions

#### Complement Edges
Currently, we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique. In the
recursive algorithms, one can even expect a factor two decrease in the
algorithms execution time [[Brace90](#references)].

#### Non-boolean Decision Diagrams
One can easily extend the proposed representation of sink nodes to encompass
non-boolean values, such as integers or floats. Thereby, the algorithms
immediately yield a Cache-oblivious implementation of the _Multi-Terminal Binary
Decision Diagrams_ (MTBDD) of [[Fujita97](#references)]. By solely using an
edge-based representation of the data-structure one can also implement a
_Multi-valued Decision Diagram_ (MDD) of [[Kam98](#references)].


## Credits

- **[Steffan Sølvsten](https://github.com/SSoelvsten)**:
  [soelvsten@cs.au.dk](mailto:soelvsten@cs.au.dk)

Thank you, to the following wonderful people:

- **[Jaco van de Pol](https://github.com/jacopol)**:
  Supervisor and extremely supportive sparring partner.


## License
The software and documentation files in this repository are provided under the
[MIT License](/LICENSE.md).


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

- [[Fujita97](https://link.springer.com/article/10.1023/A:1008647823331#citeas)]
  M. Fujita, P.C. McGeer, J.C.-Y. Yang . “_Multi-Terminal Binary Decision
  Diagrams: An Efficient Data Structure for Matrix Representation_”. In: _Formal
  Methods in System Design_. 2012

- [Kam98]
  Timothy Kam, Tiziano Villa, Robert K. Brayton, and L. Sangiovanni-vincentelli
  Alberto. “_Multi-valued decision diagrams: Theory and applications_”. In:
  _Multiple- Valued Logic 4.1_ 1998

- [[Mølhave12](https://dl.acm.org/doi/pdf/10.1145/2367574.2367579)]
  Thomas Mølhave. “_Using TPIE for Processing Massive Data Sets in C++_”. 2012

- [[Sanders01](https://dl.acm.org/doi/pdf/10.1145/351827.384249)]
  Peter Sanders. “_Fast Priority Queues for Cached Memory_”. In: _J. Exp.
  Algorithmics 5_. 2001

- [[Sitchinava12](https://dl.acm.org/doi/pdf/10.1145/2312005.2312046)]
  Nodari Sitchinava, Norbert Zeh. “_A Parallel Buffer Tree_”. In: _Proceedings
  of the Twenty-Fourth Annual ACM Symposium on Parallelism in Algorithms and
  Architectures_. 2012

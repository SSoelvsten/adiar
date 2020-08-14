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
    - [Credits](#credits)
    - [License](#license)
    - [References](#references)

<!-- markdown-toc end -->


## Introduction
In [[Arge96](/bib/%5Barge%5D%20IO%20Complexity%20of%20OBDD%20Manipulation.pdf)],
Lars Arge concludes that the I/O complexity of the DFS, BFS and other algorithms
are _O(N)_, while he proves that the _Reduce_ has an _Ω(sort(N))_ I/O lower
bound. An _Apply_ that will have to output a minimal OBDD has by extension the
same bound.

He then provides a description of an I/O optimal external-memory algorithm for
both _Reduce_ and _Apply_. They makes use of _Time-Forward Processing_ to delay
recursion until the needed data is in memory. These algorithms are designed,
such that the output of _Reduce_ is already in sorted order for the following
_Apply_ algorithm, and vica versa. When further looking into these algorithms,
we notice, that they do not explicitly make use of _M_ or _B_ but only an I/O
efficient sorting algorithm and priority queue. Both of these can be substituted
for a _Cache-oblivious_, such as [[Sanders01](#references)], or _Cache-aware_
instance to immediately yield an optimal algorithm of both types!

Following up on Arge's work, we extend this approach to other core OBDD
algorithms and implement it in C++ to benchmark the performance in practice
compared to conventional recursive procedures.


## Installation
The algorithms are implemented in _C++_ creating a fully fledged OBDD library.
They make use of the following external dependencies

- [TPIE](https://github.com/thomasmoelhave/tpie):
  Framework for implementation of I/O efficient algorithms. It directly provides
  sorting algorithms and a priotity queue. Both are much faster than the
  algorithms in the _C++_ standard library
  [[Mølhave12](#references)].

- [Bandit](https://github.com/banditcpp/bandit):
  Writing and running unit tests

### Dependencies
All dependencies are directly imported as submodules. If you have not cloned the
repository recursively, then run the following command

```bash
git submodule update --init --recursive
```

One also needs a _C++_ compiler of ones choice. All development has currently
been with the _g++_ compiler, so we cannot guarantee other compilers will work
out-of-the-box. The project also has dependencies on _CMake_, the _Boost
Library_, and _Aptitude_. On Ubuntu 18+ you can obtain all these dependencies
with the following commands.

```bash
apt install g++ cmake libboost-all-dev aptitude
```

As a visual aid, the internal representation of the Decision Diagrams can be
output as [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language))
files. These can then be turned into a graphical representation by use of a
number of tools. Internally, the program also attempts to produce a picture
using _dot_, which can be installed as follows.

```bash
apt install graphviz
```

The installation of _graphviz_ or any other _dot_ tool is not, per se, necessary
to run the algorithms, though one will not automatically obtain _.png_ files,
and one has to ignore the printout of failed system calls.

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

For development and more we also provide the following targets

| target          | effect                                                       |
|-----------------|--------------------------------------------------------------|
| `dot F=<files>` | Convert TPIE persisted file streams `<files>` into DOT files |
| `main`          | Run the _main_ function in `main.cpp` with console debugging |


### Examples and benchmarks
The _/examples_ folder contains examples for using the data structure to solve
various verification and satisfaction problems.

| target                   | Example                                                     |
|--------------------------|-------------------------------------------------------------|
| `example-n-queens N=<?>` | N-Queens board creation and counting satisfying assignments |


### In your own project
To use this OBDD implementation in your own project, then _CMake_ still needs to
be properly configured (e.g. see issue
[#4](https://github.com/SSoelvsten/cache-oblivious-obdd/issues/4)).
Contributions and help to this end would be very much appreciated.


## Documentation
The primary documentation is provided as a technical report written in LaTeX. It
provides figures, listings that describes the algorithm on an abstract level
together with a description and benchmarks of the implementation.


## Future Work
There are many possible avenues of improving on this approach to the data
structure. We list quite a few [FUTURE_WORK.md](/FUTURE_WORK.md). Many of these
may constitute interesting undergraduate research projects. We would love to see
the results of such projects as pull requests to improve the main project.


## Credits

- **[Steffan Sølvsten](https://github.com/SSoelvsten)**:
  [soelvsten@cs.au.dk](mailto:soelvsten@cs.au.dk)

Thank you, to the following wonderful people:

- **[Anna Blume Jakobsen](https://github.com/bluekeladry)**: Bachelor student,
  who implemented _Apply_, _Evaluate_, _Count Assignments_, _Count Paths_,
  _Reduce_, and _Restrict_ as part of a summer project (cf. [Talent Track
  Project](TALENT_TRACK.md)).

- **[Jaco van de Pol](https://github.com/jacopol)**:
  Supervisor and extremely supportive sparring partner.

- **[Mathias Rav](https://github.com/Mortal)**:
  Developer of TPIE and always helpful oracle about everything _TPIE_ and _C++_.

- **[Mathias Weller Berg Thomasen](https://github.com/MathiasWeller42)**:
  Bachelor student, who implemented _Apply_, _Evaluate_, _Count Assignments_,
  _Count Paths_, _Reduce_, and _Restrict_ as part of a summer project (cf.
  [Talent Track Project](TALENT_TRACK.md)).


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

- [Meinel94]
  J. Gergov and C. Meinel. “_Efficient analysis and manipulation of OBDDs can
  be extended to FBDDs_”. 1994

- [[Mølhave12](https://dl.acm.org/doi/pdf/10.1145/2367574.2367579)]
  Thomas Mølhave. “_Using TPIE for Processing Massive Data Sets in C++_”. 2012

- [[Sanders01](https://dl.acm.org/doi/pdf/10.1145/351827.384249)]
  Peter Sanders. “_Fast Priority Queues for Cached Memory_”. In: _J. Exp.
  Algorithmics 5_. 2001

- [[Sitchinava12](https://dl.acm.org/doi/pdf/10.1145/2312005.2312046)]
  Nodari Sitchinava, Norbert Zeh. “_A Parallel Buffer Tree_”. In: _Proceedings
  of the Twenty-Fourth Annual ACM Symposium on Parallelism in Algorithms and
  Architectures_. 2012

# Adiar
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)
[![Built with TPIE](https://img.shields.io/badge/built%20with-TPIE-blue)](https://users-cs.au.dk/~rav/tpie/)
[![test](https://github.com/SSoelvsten/adiar/workflows/test/badge.svg?branch=master)](/actions?query=workflow%3Atest)
[![examples](https://github.com/SSoelvsten/adiar/workflows/examples/badge.svg?branch=master)](/actions?query=workflow%3Aexamples)

Following up on the work of [[Arge96](#references)], this implementation of
a BDD library makes use of _Time-Forward Processing_ to improve the I/O
complexity of BDD Manipulation to achieve efficient manipulation of BDDs, that
far outgrow the memory limit of the given machine.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Adiar: Cache-Oblivious OBDD Manipulation](#adiar-cache-oblivious-obdd-manipulation)
    - [Installation](#installation)
        - [Dependencies](#dependencies)
        - [In your own project](#in-your-own-project)
    - [Documentation](#documentation)
    - [Usage](#usage)
        - [Examples and benchmarks](#examples-and-benchmarks)
    - [Future Work](#future-work)
    - [Credits](#credits)
    - [License](#license)
    - [References](#references)

<!-- markdown-toc end -->

## Installation
The algorithms are implemented in _C++_ creating a fully fledged BDD library,
dependant on the the following external libraries

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
been with the _gcc_ compiler, so we cannot guarantee other compilers will work
out-of-the-box. The project also has dependencies on _CMake_ and the _Boost
Library_. On Ubuntu 18+ you can obtain all these dependencies
with the following commands.

```bash
apt install g++ cmake libboost-all-dev
```

As a visual aid, the internal representation of the Decision Diagrams can be
output as [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language))
files. These can then be turned into a graphical representation by use of a
number of tools, such as _graphviz_.

```bash
apt install graphviz
```

### In your own project
The _Adiar_ library is built with _CMake_. First you'll have to include the
project as a _subdirectory_
```cmake
add_subdirectory (<path/to/adiar> adiar)
```
Then you link up your executable.
```cmake
add_executable(<target> <source>)
target_link_libraries(<target> adiar)
set_target_properties(<target> PROPERTIES CXX_STANDARD 17)
```


## Documentation
We provide the documentation in two ways:

- A [Github Pages](https://ssoelvsten.github.io/adiar/) website focuses on the
  usage of the _C++_ library.

- A [Technical Report](https://github.com/SSoelvsten/adiar-report) provides an
  “_academic_” documentation that focuses on the theory and the algorithmic
  technique behind this libary.


## Usage
The project is build with _CMake_, though for convenience I have simplified the
_CMake_ interactions to a single _Makefile_ which works on a local machine. This
has only been tested on _Ubuntu 18.04 LTS_ and _20.04 LTS_.

The primary targets are as follows

| target  | effect                 |
|---------|------------------------|
| `build` | Build the source files |
| `clean` | Remove all build files |
| `test`  | Run all unit tests     |

For development and more we also provide the following targets

| target          | effect                                                          |
|-----------------|-----------------------------------------------------------------|
| `dot F=<files>` | Convert TPIE persisted file streams `<files>` into DOT files    |
| `main M=<mem>`  | Run the _main_ function in `main.cpp` with `<mem>` MB of memory |

To further convert a _.dot_ file into a picture run the following command

```
dot -O -Tpng filename.dot
```

### Examples and benchmarks
The _/examples_ folder contains examples for using the data structure to solve
various verification and satisfaction problems. The `main` function also
includes timing how long it takes to solve and provides other interesting
statistics.

| target                               | Example                                                             |
|--------------------------------------|---------------------------------------------------------------------|
| `example-n-queens N=<?>`             | N-Queens board creation, counting and enumerating solutions         |
| `example-pigeonhole-principle N=<?>` | Proves the isomorphism of the _Pigeonhole Principle_ does not exist |
| `example-tic-tac-toe N=<?>`          | Counting the number of ties on a 3D 4✕4✕4 with _N_ crosses placed  |


## Future Work
There are many possible avenues of improving on this approach to the data
structure. We list quite a few in [FUTURE_WORK.md](/FUTURE_WORK.md). Many of
these may constitute interesting undergraduate research projects. We would love
to see the results of such projects as pull requests to improve the main
project.


## Credits

- **[Steffan Sølvsten](https://github.com/SSoelvsten)**:
  [soelvsten@cs.au.dk](mailto:soelvsten@cs.au.dk)

- **[Jaco van de Pol](https://github.com/jacopol)**:
  [jaco@cs.au.dk](mailto:jaco@cs.au.dk)

Thank you, to the following wonderful people:

- **[Anna Blume Jakobsen](https://github.com/bluekeladry)**: Bachelor student,
  who implemented _Apply_, _Evaluate_, _Count Assignments_, _Count Paths_,
  _Reduce_, and _Restrict_ as part of a summer project.

- **[Asger Hautop Drewsen](https://github.com/Tyilo)**: Previous maintainer of
  _TPIE_, who helped debug a few issues and gave general feedback on the
  codebase.

- **[Mathias Rav](https://github.com/Mortal)**:
  Developer of TPIE and always helpful oracle about everything _TPIE_,
  _I/O_ algorithms and _C++_.

- **[Mathias Weller Berg Thomasen](https://github.com/MathiasWeller42)**:
  Bachelor student, who implemented _Apply_, _Evaluate_, _Count Assignments_,
  _Count Paths_, _Reduce_, and _Restrict_ as part of a summer project.


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

- [[Coudert90](http://www.ocoudert.com/papers/pdf/iccad90.pdf)]
  Olivier Coudert and Jean Christophe Madre. “_A Unified Framework for the
  Formal verification of sequential circuits_”. In: _Computer-Aided Design /
  IEEE International Conference_. (1990)

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

- [[Kunkle10](https://dl.acm.org/doi/abs/10.1145/1837210.1837222)] Daniel
  Kunkle, Vlad Slavici, Gene Cooperman. “_Parallel Disk-Based Computation for
  Large, Monolithic Binary Decision Diagrams_”. In: _PASCO '10: Proceedings of
  the 4th International Workshop on Parallel and Symbolic Computation_. 2010

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

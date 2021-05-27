# Adiar
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)
[![Built with TPIE](https://img.shields.io/badge/built%20with-TPIE-blue)](https://users-cs.au.dk/~rav/tpie/)
[![test](https://github.com/SSoelvsten/adiar/workflows/test/badge.svg?branch=main)](/actions?query=workflow%3Atest)
[![codecov](https://codecov.io/gh/SSoelvsten/adiar/branch/main/graph/badge.svg?token=106RCIR4DJ)](https://codecov.io/gh/SSoelvsten/adiar)
[![examples](https://github.com/SSoelvsten/adiar/workflows/examples/badge.svg?branch=main)](/actions?query=workflow%3Aexamples)

Following up on the work of [[Arge96](#references)], Adiar<sup>[1](#footnotes)</sup> is a BDD
[[Bryant86](#references)] library that makes use of _Time-Forward Processing_ to
improve the I/O complexity of BDD Manipulation to achieve efficient manipulation
of BDDs, that far outgrow the memory limit of the given machine.

**Table of Contents**

- [Documentation](#documentation)
- [Dependencies](#dependencies)
- [Usage](#usage)
    - [Makefile targets](#makefile-targets)
    - [Examples](#examples)
- [Contributions](#contributions)
- [Credits](#credits)
- [License](#license)
- [References](#references)


## Documentation
The documentation is provided as a collection of Markdown files in the _docs/_
folder, which are also viewable on [Github Pages](https://ssoelvsten.github.io/adiar/).


## Dependencies
The implementation is dependant on the the following external libraries

- [**TPIE**](https://github.com/thomasmoelhave/tpie):

  Framework for implementation of I/O efficient algorithms. It directly provides
  sorting algorithms and a priotity queue. Both are much faster than the
  algorithms in the _C++_ standard library
  [[Vengroff94,Mølhave12](#references)].

- [**Bandit**](https://github.com/banditcpp/bandit):

  Unit testing framework that is easy-to-read and write.

These are directly imported as submodules. If you have not cloned the repository
recursively, then run the following command

```bash
git submodule update --init --recursive
```

Other dependencies that we cannot provide as a submodule are shown below. The
_ticked_ dependencies are mandatory to install.

- [x] **CMake** and a **C++ compiler**

  One also needs a _C++_ compiler that supports the _17_ standard and _CMake_.
  All development has currently been with the _gcc_ compiler, so we cannot
  guarantee other compilers will work out-of-the-box. On Ubuntu you can obtain
  these with the following command

  ```bash
  sudo apt install g++ cmake
  ```

- [x] **Boost**

  Furthermore, TPIE has a dependency on the [Boost](https://www.boost.org/)
  Library. On Ubuntu you can obtain this with the following command.

  ```bash
  sudo apt install libboost-all-dev
  ```

  You can also build and install it from its
  [source](https://github.com/boostorg/boost).

- [ ] **DOT**

  As a visual aid, the internal representation of the Decision Diagrams can be
  output as
  [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) files.
  These can then be turned into a graphical representation by use of a number of
  tools, such as _graphviz_.

  ```bash
  sudo apt install graphviz
  ```

## Usage

### Makefile targets

The project is build with _CMake_, though for convenience I have simplified the
_CMake_ interactions to a single _Makefile_ which works on a local machine. This
has only been tested on _Ubuntu 18.04 LTS_ and _20.04 LTS_.

The _Makefile_ provides the following targets

| target          | effect                                                               |
|-----------------|----------------------------------------------------------------------|
| `build`         | Build the source files                                               |
| `clean`         | Remove all build files                                               |
|                 |                                                                      |
| `test`          | Run all unit tests (with _O2_ optimisations)                         |
| `coverage`      | Run all unit tests (with no optimisations) and create _lcov_ report  |
|                 |                                                                      |
| `main M=<MiB>`  | Run the _main_ function in `src/main.cpp` with `<MiB>` MiB of memory |

### Examples
The _example/_ folder contains examples for using the data structures in Adiar.
The _README.md_ file in said folder contains a more in-depth description of each
of the examples. Even more examples and benchmarks are provided in a separate
[BDD Benchmarking repository](https://github.com/SSoelvsten/bdd-benchmark).

Each example takes a set of arguments as input, which can be parsed to the
program as a makefile variable. That is, to change the value of _N_ to to value
follow the `make <target>` with `N=<value>`.

**example/queens**

| variable | description                   |
|----------|-------------------------------|
| _N_      | Board size (default: 8)       |
| _M_      | Memory in MiB (default: 1024) |

**example/knights-tour**

| variable | description                   |
|----------|-------------------------------|
| _N_      | Board size (default: 8)       |
| _M_      | Memory in MiB (default: 1024) |


## Contributions
Adiar is not yet feature complete, and there are still many interesting things
left for _you_ to do. We already list lots of suggestions for possible projects
as [future work](/FUTURE_WORK.md) and other possible things to contribute with
as [issues](https://github.com/SSoelvsten/adiar/issues).


## Credits

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

- **[Steffan Sølvsten](https://github.com/SSoelvsten)**:
  [soelvsten@cs.au.dk](mailto:soelvsten@cs.au.dk)

- **[Jaco van de Pol](https://github.com/jacopol)**:
  [jaco@cs.au.dk](mailto:jaco@cs.au.dk)

Thank you, to the following wonderful people:

- **[Anna Blume Jakobsen](https://github.com/AnnaBlume99)**:
  Bachelor student, who during a summer project wrote the prototype
  implementation of many core algorithms.

- **[Asger Hautop Drewsen](https://github.com/Tyilo)**: Previous maintainer of
  _TPIE_, who helped debug a few issues and gave general feedback on the
  codebase.

- **[Mathias Rav](https://github.com/Mortal)**:
  Developer of _TPIE_, always helpful oracle about everything _TPIE_,
  _I/O_ algorithms, and _C++_.

- **[Mathias Weller Berg Thomasen](https://github.com/MathiasWeller42)**:
  Bachelor student, who during a summer project wrote the prototype
  implementation of many core algorithms.


## License
The software and documentation files in this repository are provided under the
[MIT License](/LICENSE.md).


## References

- [[Arge96](https://tidsskrift.dk/brics/article/view/20010/17643)]
  Lars Arge. “_The I/O-complexity of Ordered Binary-Decision Diagram
  Manipulation_”. In: _Efficient External-Memory Data Structures and
  Applications_. (1996)

- [[Bryant86](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1676819)]
  Randal E. Bryant. “_Graph-Based Algorithms for Boolean Function Manipulation_”.
  In: _IEEE Transactions on Computers_. (1986)

- [[Mølhave12](https://dl.acm.org/doi/pdf/10.1145/2367574.2367579)]
  Thomas Mølhave. “_Using TPIE for Processing Massive Data Sets in C++_”. (2012)

- [[Vengroff94](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.38.3030)]
  D.E. Vengroff. “_A transparent parallel I/O environment_”. In: _In Proc. 1994
  DAGS Symposium on Parallel Computation_. pp. 117–134 (1994)

## Footnotes

1. **adiar** &#10216; portugese &#10217; ( *verb* ) : to defer, to postpone

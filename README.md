# Adiar
[![MIT License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE.md)
[![Built with TPIE](https://img.shields.io/badge/built%20with-TPIE-blue)](https://users-cs.au.dk/~rav/tpie/)
[![test](https://github.com/SSoelvsten/adiar/workflows/test/badge.svg?branch=master)](/actions?query=workflow%3Atest)
[![codecov](https://codecov.io/gh/SSoelvsten/adiar/branch/master/graph/badge.svg?token=106RCIR4DJ)](https://codecov.io/gh/SSoelvsten/adiar)
[![examples](https://github.com/SSoelvsten/adiar/workflows/examples/badge.svg?branch=master)](/actions?query=workflow%3Aexamples)

Following up on the work of [[Arge96](#references)], this implementation of a
BDD [[Bryant86](#references)] library makes use of _Time-Forward Processing_ to
improve the I/O complexity of BDD Manipulation to achieve efficient manipulation
of BDDs, that far outgrow the memory limit of the given machine.

**Table of Contents**

- [Documentation](#documentation)
- [Dependencies](#dependencies)
- [Usage](#usage)
    - [Makefile targets](#makefile-targets)
    - [Examples and benchmarks](#examples-and-benchmarks)
- [Contributions](#contributions)
- [Credits](#credits)
- [License](#license)
- [References](#references)


## Documentation
The documentation is provided as a collection of Markdown files in the
[/docs](https://github.com/SSoelvsten/adiar/tree/master/docs) folder, which are also
viewable on [Github Pages](https://ssoelvsten.github.io/adiar/).


## Dependencies
The implementation is dependant on the the following external libraries

- [TPIE](https://github.com/thomasmoelhave/tpie):
  Framework for implementation of I/O efficient algorithms. It directly provides
  sorting algorithms and a priotity queue. Both are much faster than the
  algorithms in the _C++_ standard library
  [[Vengroff94,Mølhave12](#references)].

- [Bandit](https://github.com/banditcpp/bandit):
  Writing and running unit tests

These are directly  imported as submodules. If you have not cloned the
repository recursively, then run the following command

```bash
git submodule update --init --recursive
```

One also needs a _C++_ compiler that supports the _17_ standard. All development
has currently been with the _gcc_ compiler, so we cannot guarantee other
compilers will work out-of-the-box. The project is built with _CMake_ and has
dependencies on the _Boost Library_. On Ubuntu 20+ you can obtain all these with
the following commands.

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

### Examples and benchmarks
The _/examples_ folder contains examples for using the data structure to solve
various verification and satisfaction problems. Even more examples and
benchmarks are provided in a separate
[BDD Benchmarking repository](https://github.com/SSoelvsten/bdd-benchmark).

The problem size and memory used for the examples mentioned in the targets below
can be varied with the following two Makefile variables

- `N`: The _N_ to be used in the problem.
- `M`: The amount of memory given to Adiar in MiB (default: 1024).

| target                         | Example                                                             |
|--------------------------------|---------------------------------------------------------------------|
| `example-queens`               | _N_-Queens board creation, counting and enumerating solutions       |


## Contributions
Adiar is not yet feature complete, and there are still many interesting things
left for _you_ to do. We already list lots of suggestions for possible projects
and contributions as [future work](/FUTURE_WORK.md).


## Credits

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

- **[Steffan Sølvsten](https://github.com/SSoelvsten)**:
  [soelvsten@cs.au.dk](mailto:soelvsten@cs.au.dk)

- **[Jaco van de Pol](https://github.com/jacopol)**:
  [jaco@cs.au.dk](mailto:jaco@cs.au.dk)

Thank you, to the following wonderful people:

- **[Anna Blume Jakobsen](https://github.com/bluekeladry)**:
  Bachelor student, who during a summer project wrote the first implementation
  of the _Apply_, _Evaluate_, _Count Assignments_, _Count Paths_, _Reduce_, and
  _Restrict_ algorithms.

- **[Asger Hautop Drewsen](https://github.com/Tyilo)**: Previous maintainer of
  _TPIE_, who helped debug a few issues and gave general feedback on the
  codebase.

- **[Mathias Rav](https://github.com/Mortal)**:
  Developer of _TPIE_, always helpful oracle about everything _TPIE_,
  _I/O_ algorithms, and _C++_.

- **[Mathias Weller Berg Thomasen](https://github.com/MathiasWeller42)**:
  Bachelor student, who during a summer project wrote the first implementation
  of the _Apply_, _Evaluate_, _Count Assignments_, _Count Paths_, _Reduce_, and
  _Restrict_ algorithms.


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

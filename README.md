# Adiar
[![MIT License](https://img.shields.io/github/license/ssoelvsten/adiar)](LICENSE.md)
&nbsp;
[![Release](https://img.shields.io/github/v/release/ssoelvsten/adiar)](https://github.com/SSoelvsten/adiar/releases)
&nbsp;
[![Documentation](https://img.shields.io/website?down_message=not%20available&label=docs&up_message=available&url=https%3A%2F%2Fssoelvsten.github.io%2Fadiar)](https://ssoelvsten.github.io/adiar)
&nbsp;
[![codecov](https://img.shields.io/codecov/c/github/ssoelvsten/adiar?logo=codecov&logoColor=white&token=106RCIR4DJ)](https://codecov.io/gh/SSoelvsten/adiar)
&nbsp;
[![test](https://img.shields.io/github/workflow/status/ssoelvsten/adiar/unit%20test/main?label=test&logo=github&logoColor=white)](https://github.com/SSoelvsten/adiar/actions/workflows/unit_test.yml)
&nbsp;
[![examples](https://img.shields.io/github/workflow/status/ssoelvsten/adiar/examples/main?label=examples&logo=github&logoColor=white)](https://github.com/SSoelvsten/adiar/actions/workflows/examples.yml)

Based on the work of Lars Arge [[Arge96](#references)], Adiar<sup>[1](#footnotes)</sup>
is a BDD package [[Bryant86](#references)] that makes use of time-forward processing
to improve the I/O complexity of BDD manipulation. This makes it able to achieve
efficient manipulation of BDDs, even when they outgrow the memory limit of the
given machine.

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

**Table of Contents**

- [Documentation](#documentation)
- [Dependencies](#dependencies)
- [Usage](#usage)
    - [Makefile targets](#makefile-targets)
    - [Examples](#examples)
- [Contributions](#contributions)
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
  [[Vengroff94](#references), [Mølhave12](#references)].

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
  guarantee other compilers will work out-of-the-box.

- [x] **Boost**

  Furthermore, TPIE has a dependency on the [Boost](https://www.boost.org/)
  Library.

- [ ] **DOT**

  As a visual aid, the internal representation of the Decision Diagrams can be
  output as
  [DOT](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) files.
  These can then be turned into a graphical representation by use of a number of
  tools, such as _graphviz_.

To install all of the above then run the respective below.

| Operating System | Shell command                                         |
|------------------|-------------------------------------------------------|
| Ubuntu 18+       | `apt install cmake g++     libboost-all-dev graphviz` |
| Fedora 35+       | `dnf install cmake gcc-c++ boost-devel      graphviz` |

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

**example/knights-tour/[T]** 

The value **T** can either be _all_ or _closed_.

| variable | description                                      |
|----------|--------------------------------------------------|
| _N_      | Board size (default: 8)                          |
| _M_      | Memory in MiB (default: 1024)                    |


## Contributions

Adiar is not yet feature complete, and there are still many interesting things
left for _you_ to do. We already list lots of suggestions for possible projects
as [future work](/FUTURE_WORK.md) and other possible things to contribute with
as [issues](https://github.com/SSoelvsten/adiar/issues).


## License
The software and documentation files in this repository are provided under the
[MIT License](/LICENSE.md).

Using Adiar will indirectly use [TPIE](https://github.com/thomasmoelhave/tpie)
underneath, which in turn is licensed under the _LGPL v3_ license. Hence, a
binary of yours that is statically linked to Adiar will be affected by that
license. That is, if you share that binary with others, then you will be obliged
to make the source public. This can be resolved by using Adiar as a shared
library or have it use an alternative to TPIE, such as
[STXXL](https://github.com/stxxl/stxxl).


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

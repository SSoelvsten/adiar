---
layout: default
title: Home
nav_order: 1
description: "An External Memory Binary Decision Diagram Library"
permalink: /
---

# Adiar
{: .fs-9 }

_Adiar_ is a library for manipulating Binary Decision Diagrams even when these
grow bigger than the memory one has available (cf. Figure [1](#figure-1)). To
achieve this, all operations are not implemented recursively with a global
memoization table, but instead they are created as streaming algorithms that
exploit the sorting of the graph on disk to delay recursion with Time-Forward
Processing. This approach proves viable compared to prior methods for very large
instances (cf. Figure [2](#figure-2)).
{: .fs-6 .fw-300 }

[View it on GitHub](https://github.com/ssoelvsten/adiar){: .btn .fs-5 .mb-4 .mb-md-0 }

![](img/memory.png)

## Figure 1
{: .no_toc .text-delta }

Running time of _Adiar_ solving the _N-Queens_ problem with different amounts of
main memory available.

![](img/queens.png)

## Figure 2
{: .no_toc .text-delta }

Performance of _Adiar_ compared to _BuDDy_ [[Lind-Nielsen99](#references)] and
_Sylvan_ [[Dijk16](#references)] on solving the _N-Queens_ problem. The
available memory for _Adiar_ and _Sylvan_ (annotated at the top) is chosen to
minimise the running time of _Sylvan_.

The data in the two figures above has been obtained at the
[Centre for Scientific Computing, Aarhus](phys.au.dk/forskning/cscaa).

---

## Table of Content

1. [Getting started](getting_started)

  Dependencies and installation of _Adiar_ and how to initialise it in your C++
  program.

2. [BDD](bdd)

  The _BDD_ data structure and the functions provided to manipulate it.

3. [Core](core)

  The underlying data structures _nodes_ and _files_ that you would use to
  quickly construct larger Decision Diagrams programmatically.

4. [Examples](examples)

  Examples that cover the functionalities of _Adiar_.

  1. [Queens](examples/queens)


## About the project

**Current Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

### License
The software and documentation files in this repository are distributed under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/master/LICENSE.md).

## References

- [[Dijk16](https://link.springer.com/content/pdf/10.1007/s10009-016-0433-2.pdf)]
  Tom van Dijk, Jaco van de Pol. “_Sylvan: multi-core framework for decision
  diagrams_”. In: _International Journal on Software Tools for Technology
  Transfer_. (2016)

- [[Lind-Nielsen99](http://www.itu.dk/research/buddy)]
  Jørn Lind-Nielsen. “_BuDDy: A binary decision diagram package_”. Tech. rep.:
  _Department of Information Technology, Technical University of Denmark_. (1999)


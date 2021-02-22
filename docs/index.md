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
grow bigger than the memory one has available. To achieve this, all operations
are not implemented recursively with a global memoization table, but instead
they are created as streaming algorithms that exploit the sorting of the graph
on disk to delay recursion with Time-Forward Processing.
{: .fs-6 .fw-300 }

[View it on GitHub](https://github.com/ssoelvsten/adiar){: .btn .fs-5 .mb-4 .mb-md-0 }

---

## Table of Content

- [**Getting Started**](/getting_started)

  Dependencies and installation of _Adiar_ and how to initialise it in your C++
  program.

- [**BDD**](/bdd)

  The _BDD_ data structure and the functions provided to manipulate it.

- [**Core**](/core)

  The underlying data structures _nodes_ and _files_ that you would use to
  quickly construct larger Decision Diagrams programmatically.

- [**Examples**](/examples)

  Examples that cover the functionalities of _Adiar_.

  - [**N-Queens**](/examples/queens)


## About the project

**Current Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

### License
The software and documentation files in this repository are distributed under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/master/LICENSE.md).

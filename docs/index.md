---
layout: default
title: Home
nav_order: 1
description: "An External Memory Decision Diagram Package"
permalink: /
---

# Adiar
{: .fs-9 }

*Adiar* is a library for manipulating Decision Diagrams even when these grow
bigger than the memory one has available. To achieve this, all operations are
not implemented recursively with a shared unique node table, but instead they
are created as streaming algorithms that exploit a specific sorting of diagram
nodes on disk to delay recursion.
{: .fs-6 .fw-300 }

[View it on GitHub](https://github.com/ssoelvsten/adiar){: .btn .fs-5 .mb-4 .mb-md-0 }

## Table of Content
{: .text-delta }

1. [Getting started](getting_started.md)

   Dependencies and installation of *Adiar* and how to initialise it in your C++
   application.

2. [Data Structures](data_structures.md)
   1. [Binary Decision Diagrams (BDD)](data_structures/bdd.md)
   2. [Zero-suppressed Decision Diagrams (ZDD)](data_structures/zdd.md)
   3. [Labels and Assignments](data_structures/labels_and_assignments.md)

3. [Manual Construction](manual_construction.md)

   How to construct BDDs and ZDDs bottom-up by hand.

4. [Statistics](statistics.md)

   Statistics on the internal algorithms and data structures.

5. [Settings](settings.md)

   Global settings of *Adiar* that can be set at run or at compile-time.

6. [Examples](examples.md)

   Examples that cover the functionalities of *Adiar*.

7. [Citing this project](cite.md)

   Overview of the papers relevant for citing this project.

---

## About the project

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

**Current Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

### License
The software and documentation files in this repository are distributed under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/main/LICENSE.md).

Using Adiar will indirectly use [TPIE](https://github.com/thomasmoelhave/tpie)
underneath, which in turn is licensed under the _LGPL v3_ license. Hence, a
binary of yours that is statically linked to Adiar will be affected by that
license. That is, if you share that binary with others, then you will be obliged
to make the source public. This can be resolved by using Adiar as a shared
library or have it use an alternative to TPIE, such as
[STXXL](https://stxxl.org/).

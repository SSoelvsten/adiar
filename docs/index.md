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

2. [BDD](bdd.md)

   Binary Decision Diagrams and its supported functions.

3. [ZDD](zdd.md)

   Zero-suppressed Decision Diagrams and its supported functions.

4. [Statistics](statistics.md)

   Statistics on the internal algorithms and data structures.

5. [Core](core.md)

   The underlying *data types* and *files* that you can use to construct larger
   decision diagrams programmatically.

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

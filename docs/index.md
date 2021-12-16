---
layout: default
title: Home
nav_order: 1
description: "An External Memory Decision Diagram Library"
permalink: /
---

# Adiar
{: .fs-9 }

_Adiar_ is a library for manipulating Decision Diagrams even when these grow
bigger than the memory one has available. To achieve this, all operations are
not implemented recursively with a shared unique node table, but instead they
are created as streaming algorithms that exploit the sorting of the graph on
disk to delay recursion with Time-Forward Processing.
{: .fs-6 .fw-300 }

[View it on GitHub](https://github.com/ssoelvsten/adiar){: .btn .fs-5 .mb-4 .mb-md-0 }

## Table of Content
{: .text-delta }

1. [Getting started](getting_started)

   Dependencies and installation of _Adiar_ and how to initialise it in your C++
   program.

2. [BDD](bdd)

   The _BDD_ data structure and the functions provided to manipulate it.

3. [ZDD](zdd)

   The _ZDD_ data structure and the functions provided to manipulate it.

4. [Core](core)

   The underlying _data types_ and _files_ that you would use to construct
   larger decision diagrams programmatically.
   
5. [Examples](examples)

   Examples that cover the functionalities of _Adiar_.

---

## About the project

This project has been developed at the [Logic and Semantics](https://logsem.github.io/)
group at [Aarhus University](https://cs.au.dk).

**Current Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

### License
The software and documentation files in this repository are distributed under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/main/LICENSE.md).

Using Adiar will indirectly use [TPIE](https://github.com/thomasmoelhave/tpie)
underneath, which in turn is licensed under the _GPL 3_ license. Hence, your
resulting binaries linked to Adiar will also be under that license. If you share
that binary with others, then you will be obliged to make the source public.
This can be resolved by making Adiar use an alternative to TPIE, such as
[STXXL](https://github.com/stxxl/stxxl).

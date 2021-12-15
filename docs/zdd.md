---
layout: default
title: ZDD
nav_order: 3
description: "The ZDD data structure and the functions provided to manipulate it"
permalink: /zdd
---

# ZDD
{: .no_toc }

A Zero-suppressed Decision Diagram (ZDD) represents a family of set of numbers
{: .fs-6 .fw-300 }

<p class="fs-6 fw-300" style="text-align: center;">
  S ⊆ 2<sup>{0, ..., n-1}</sup>
</p>

The `zdd` class takes care of reference counting and optimal garbage collection
of the underlying files (cf. [Core/Files](./core/files#nodes)). To ensure the
most disk-space is available, try to garbage collect the `zdd` objects as
quickly as possible and/or minimise the number of lvalues of said type.

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Basic Constructors

To construct a more complex but well-structured `zdd` than what follows below,
create a [`node_file`](./core/files#nodes) and write the nodes bottom-up with
the [`node_writer`](./core/files#node-writer). The `zdd` object can then be
copy-constructed from the `node_file`.

### `zdd zdd_sink(bool v)`
{: .no_toc }

Create a sink-only ZDD with the provided boolean value _v_. More meaningful
alternatives are:
- `zdd_empty()` to create Ø
- `zdd_null()` to create {Ø}

### `zdd zdd_ithvar(label_t i)`
{: .no_toc }

Create a ZDD representing the family _{ {i} }_.

### `zdd zdd_vars(label_file is)`
{: .no_toc }

Create a ZDD representing the family _{ {is<sub>1</sub>, is<sub>2</sub>, ..., is<sub>k</sub>} }_.

### `zdd zdd_singletons(label_file is)`
{: .no_toc }

Create a ZDD representing the family _{ {is<sub>1</sub>}, {is<sub>2</sub>}, ..., {is<sub>k</sub>} }_.

### `zdd zdd_powerset(label_file is)`
{: .no_toc }

Create a ZDD representing the family _{ {is<sub>1</sub>}, {is<sub>2</sub>}, ..., {is<sub>1</sub>, is<sub>2</sub>}, {is<sub>1</sub>, is<sub>3</sub>}, ..., {is<sub>1</sub>, ..., is<sub>k</sub>} }_.

### `zdd zdd_sized_set<pred_t>(label_file is, k, pred)`
{: .no_toc }

Create a ZDD representing the family _{ s ∈ 2<sup>vars</sup> | pred(|s|, k) }_, where pred
is a comparison predicate such as `std::less`, `std::greater`, and
`std::equal_to`.

## Basic Manipulation

### `zdd zdd_binop(zdd A, zdd B, bool_op op)`
{: .no_toc }

Construct the ZDD for _{ s ∈ A ∪ B | op(s ∈ A, s ∈ B) }_, where *op* is a binary
operator whether to include a set _s_ based on whether they are included in _A_
and/or _B_. Some operators are also provided with an alias function:

- `zdd zdd_intsec(zdd A, zdd B)` (operator `|`)
  
  Same as `zdd_binop(A, B, or_op)` and computes _A ∪ B_.

- `zdd zdd_intsec(zdd A, zdd B)` (operator `&`)
  
  Same as `zdd_binop(A, B, and_op)` and computes _A ∩ B_.

- `zdd zdd_diff(zdd A, zdd B)` (operator `&`)
  
  Same as `zdd_binop(A, B, diff_op)` and computes _A \ B_.

### `zdd zdd_project(zdd A, label_file is)`
{: .no_toc }

Construct the ZDD for π<sub>i<sub>1</sub>, ..., i<sub>k</sub></sub>(A) = { s \ {
i<sub>1</sub>, ..., i<sub>k</sub> }<sup>c</sup> | s ∈ A }, where i<sub>1</sub>,
..., i<sub>k</sub> are the elements in _is_. That is, this constructs a ZDD
of the same sets, but where only the elements in _is_ are kept.

## Counting Operations

### `uint64_t zdd_nodecount(zdd A)`
{: .no_toc }

Return the number of nodes (not counting sink nodes) in the ZDD for _A_.

### `uint64_t zdd_varcount(zdd A)`
{: .no_toc }

Return the number of variables present in the ZDD for _A_.

### `uint64_t bdd_size(bdd A)`
{: .no_toc }

Return _\|A\|_, i.e. the number of sets of elements in the family of sets _A_.

## Predicates

### `bool zdd_equal(zdd A, zdd B)` (operator `==`)
{: .no_toc }

Return whether _A = B_.

### `bool zdd_unequal(zdd A, zdd B)` (operator `!=`)
{: .no_toc }

Return whether _A ≠ B_.

### `bool zdd_subseteq(zdd A, zdd B)` (operator `<=`)
{: .no_toc }

Return whether _A ⊆ B_.

### `bool zdd_subset(zdd A, zdd B)` (operator `<`)
{: .no_toc }

Return whether _A ⊂ B_.

### `bool zdd_disjoint(zdd A, zdd B)`
{: .no_toc }

Return whether _A ∩ B = Ø_.

## Other Functions

### `bool is_sink(zdd A, sink_pred)`
{: .no_toc }

Whether the ZDD for _A_ only consists of a sink satisfying the given sink
predicate. By default the predicate for _any_ kind of sink is used.

### `label_t min_label(zdd A)`
{: .no_toc }

Return the smallest element that exists in some set in _A_, i.e. the label of
the root of the DAG in the ZDD.

### `label_t max_label(zdd A)`
{: .no_toc }

Return the largest element that exists in some set in _A_, i.e. the label of the
deepest node of the DAG in the ZDD.

## DOT Output

### `void output_dot(zdd A, std::string filename)`
{: .no_toc }

Prints the ZDD _A_ to a _.dot_ file with the given _filename_.

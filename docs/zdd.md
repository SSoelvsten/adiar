---
layout: default
title: ZDD
nav_order: 3
description: "Zero-suppressed Decision Diagrams"
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
of the underlying files (cf. [Core/Files](core/files.md#nodes)). To ensure the
most disk-space is available, try to garbage collect the `zdd` objects as
quickly as possible and/or minimise the number of lvalues of said type.

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Basic Constructors

To construct a more complex but well-structured `zdd` than what follows below,
create a [`node_file`](core/files.md#nodes) and write the nodes bottom-up with
the [`node_writer`](core/files.md#node-writer). The `zdd` object can then be
copy-constructed from the `node_file`.

### `zdd zdd_sink(bool v)`
{: .no_toc }

Create a sink-only ZDD with the provided boolean value *v*. More meaningful
alternatives are:
- `zdd_empty()` to create Ø
- `zdd_null()` to create {Ø}

### `zdd zdd_ithvar(label_t var)`
{: .no_toc }

Create a ZDD representing the family *{ {var} }*.

### `zdd zdd_vars(label_file vars)`
{: .no_toc }

Create a ZDD representing the family *{ {vars<sub>1</sub>, vars<sub>2</sub>, ..., vars<sub>k</sub>} }*.
The labels in *vars* must be sorted in increasing order.

### `zdd zdd_singletons(label_file vars)`
{: .no_toc }

Create a ZDD representing the family *{ {vars<sub>1</sub>}, {vars<sub>2</sub>}, ..., {vars<sub>k</sub>} }*.
The labels in *vars* must be sorted in increasing order.

### `zdd zdd_powerset(label_file vars)`
{: .no_toc }

Create a ZDD representing the family *pow(vars) = 2<sup>vars</sup>*. The labels
in *vars* must be sorted in increasing order.

### `zdd zdd_sized_set<pred_t>(label_file vars, k, pred)`
{: .no_toc }

Create a ZDD representing the family *{ s ∈ pow(vars) | pred(|s|, k) }*, where pred
is a comparison predicate such as `std::less`, `std::greater`, and
`std::equal_to`.

## Basic Manipulation

### `zdd zdd_binop(zdd A, zdd B, bool_op op)`
{: .no_toc }

Construct the ZDD for *{ s ∈ A ∪ B | op(s ∈ A, s ∈ B) }*, where *op* is a binary
operator whether to include a set *s* based on whether they are included in *A*
and/or *B*. Some operators are also provided with an alias function:

- `zdd zdd_union(zdd A, zdd B)` (operator `|`)
  
  Same as `zdd_binop(A, B, or_op)` and computes *A ∪ B*.

- `zdd zdd_intsec(zdd A, zdd B)` (operator `&`)
  
  Same as `zdd_binop(A, B, and_op)` and computes *A ∩ B*.

- `zdd zdd_diff(zdd A, zdd B)` (operator `-`)
  
  Same as `zdd_binop(A, B, diff_op)` and computes *A* \ *B*.

### `zdd zdd_change(zdd A, label_file vars)`
{: .no_toc }

Constructs the ZDD for *{ vars Δ a | a ∈ A }*, where Δ is the symmetric
difference between the two sets of variables *a* and *vars*. In other words, for
each set in *A* the value of each variable *i* from *vars* is flipped.

### `zdd zdd_complement(zdd A, label_file dom)`
{: .no_toc }

Constructs the ZDD for *pow(dom)* \ *A*, i.e. the complement of *A* with respect
to the variable domain, *dom*. The variables in *A* have to exist in *dom* too.

### `zdd zdd_expand(zdd A, label_file vars)`
{: .no_toc }

Expands the domain of A to also include the variables in *vars*, i.e. it
computes the set *U<sub>a ∈ A, i ∈ pow(vars)</sub> (a ∪ i)*. The variables in
*vars* are not allowed to be present in *A*.

### `zdd zdd_offset(zdd A, label_file vars)`
{: .no_toc }

Computes the ZDD for the subset *{ a ∈ A | ∀i ∈ vars : i ∉ a }*, i.e. where the
variables *i* in *vars* are set to 0.

### `zdd zdd_onset(zdd A, label_file vars)`
{: .no_toc }

Computes the ZDD for the subset *{ a ∈ A | ∀i ∈ vars : i ∈ a }*, i.e. where the
variables *i* in *vars* are set to 1.

### `zdd zdd_project(zdd A, label_file dom)`
{: .no_toc }

Construct the ZDD for π<sub>i<sub>1</sub>, ..., i<sub>k</sub></sub>(A) = { s \ {
i<sub>1</sub>, ..., i<sub>k</sub> }<sup>c</sup> | s ∈ A }, where i<sub>1</sub>,
..., i<sub>k</sub> are the elements in *vars*. That is, this constructs a ZDD of
the same sets, but where only the elements in the new domain *dom* are kept.

## Counting Operations

### `uint64_t zdd_nodecount(zdd A)`
{: .no_toc }

Return the number of nodes (not counting sink nodes) in the ZDD for *A*.

### `uint64_t zdd_varcount(zdd A)`
{: .no_toc }

Return the number of variables present in the ZDD for *A*.

### `uint64_t zdd_size(zdd A)`
{: .no_toc }

Return \|*A*\|, i.e. the number of sets of elements in the family of sets *A*.

## Predicates

### `bool zdd_equal(zdd A, zdd B)` (operator `==`)
{: .no_toc }

Return whether *A = B*.

### `bool zdd_unequal(zdd A, zdd B)` (operator `!=`)
{: .no_toc }

Return whether *A ≠ B*.

### `bool zdd_subseteq(zdd A, zdd B)` (operator `<=`)
{: .no_toc }

Return whether *A ⊆ B*.

### `bool zdd_subset(zdd A, zdd B)` (operator `<`)
{: .no_toc }

Return whether *A ⊂ B*.

### `bool zdd_disjoint(zdd A, zdd B)`
{: .no_toc }

Return whether *A ∩ B = Ø*.

## Set elements

### `bool zdd_contains(zdd A, label_file a)`
{: .no_toc }

Return whether *a ∈ A*.

### `std::optional<label_file> zdd_minelem(zdd A)`
{: .no_toc }

Finds the *a ∈ A* (if any) that is lexicographically smallest when interpreting
*a* as a binary number with 0 being the most significant bit.

### `std::optional<label_file> zdd_maxelem(zdd A)`
{: .no_toc }

Finds the *a ∈ A* (if any) that is lexicographically largest when interpreting
*a as a binary number with 0 being the most significant bit.

### `label_t min_label(zdd A)`
{: .no_toc }

Return the smallest element that exists in some set *a* in *A*, i.e. the label
of the root of the DAG in the ZDD.

### `label_t max_label(zdd A)`
{: .no_toc }

Return the largest element that exists in some set *a* in *A*, i.e. the label of
the deepest node of the DAG in the ZDD.

## Other Functions

### `bool is_sink(zdd A, sink_pred)`
{: .no_toc }

Whether the ZDD for _A_ only consists of a sink satisfying the given sink
predicate. By default the predicate for *any* kind of sink is used.

### `zdd zdd_from(bdd f, label_file dom)`
{: .no_toc }

Converts a [BDD](bdd.md) into a ZDD interpreted within the variable domain
*dom*. The domain should be a superset of the variables in the given BDD.

## DOT Output

### `void output_dot(zdd A, std::string filename)`
{: .no_toc }

Prints the ZDD *A* to a *.dot* file with the given *filename*.

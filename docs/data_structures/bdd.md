---
layout: default
title: BDD
nav_order: 1
parent: Data Structures
description: "Binary Decision Diagrams"
permalink: data_structures/bdd
---

# BDD
{: .no_toc }

A Binary Decision Diagram (BDD) represents a boolean function
{: .fs-6 .fw-300 }

<p class="fs-6 fw-300" style="text-align: center;">
  {0,1}<sup>n</sup> → {0,1}
</p>

The `bdd` class takes care of reference counting and optimal garbage collection
of the underlying files. To ensure the most disk-space is available, try to
garbage collect the `bdd` objects as quickly as possible and/or minimise the
number of lvalues of said type.

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Basic Constructors

To construct a more complex but well-structured `bdd` by hand see the section on
[Manual Construction](../manual_construction).

### `bdd bdd_terminal(bool v)`
{: .no_toc }

Create a terminal-only BDD with the provided boolean value *v*. Alternatives are
`bdd_true()` and `bdd_false()` or to assign a *bool* directly to a BDD.

### `bdd bdd_ithvar(label_t var)`
{: .no_toc }

Create a BDD representing the literal with the given label *var*.

### `bdd bdd_nithvar(label_t var)`
{: .no_toc }

Create a BDD representing the negation of the literal with the given label
*var*.

### `bdd bdd_and(label_file vars)`
{: .no_toc }

Construct a BDD representing the *and* of all the literals with the provided
[labels](./labels_and_assignments.md#labels). The *vars* must be sorted in
increasing order.

### `bdd bdd_or(label_file vars)`
{: .no_toc }

Construct a BDD representing the *or* of all the literals with the provided
[labels](./labels_and_assignments.md#labels). The *vars* must be sorted in
increasing order.

### `bdd bdd_counter(label_t min_var, label_t max_var, label_t threshold)`
{: .no_toc }

Construct a BDD that is true if exactly *threshold* many of the variables in the
interval [*min_var*; *max_var*] are true.

## Basic Manipulation

### `bdd bdd_apply(bdd f, bdd g, bool_op op)`
{: .no_toc }

Construct a BDD representing the *op* applied onto the BDDs for *f* and *g*. For
each operator, we provide the following alias functions:

- `bdd bdd_and(bdd f, bdd g)` (operator `&`)

  Same as `bdd_apply(f, g, and_op)` and computes *f ∧ g*.

- `bdd bdd_nand(bdd f, bdd g)`

  Same as `bdd_apply(f, g, nand_op)` and computes *¬(f ∧ g)*.

- `bdd bdd_or(bdd f, bdd g)` (operator `|`)

  Same as `bdd_apply(f, g, or_op)` and computes *f ∨ g*.

- `bdd bdd_nor(bdd f, bdd g)`

  Same as `bdd_apply(f, g, nor_op)` and computes *¬(f ∨ g)*.

- `bdd bdd_xor(bdd f, bdd g)` (operator `^`)

  Same as `bdd_apply(f, g, xor_op)` and computes *f ⊕ g*.

- `bdd bdd_xnor(bdd f, bdd g)`

  Same as `bdd_apply(f, g, xnor_op)` and computes *¬(f ⊕ g)*.

- `bdd bdd_imp(bdd f, bdd g)`

  Same as `bdd_apply(f, g, imp_op)` and computes *f ⇒ g*.

- `bdd bdd_invimp(bdd f, bdd g)`

  Same as `bdd_apply(f, g, invimp_op)` and computes *g ⇒ f*.

- `bdd bdd_equiv(bdd f, bdd g)`

  Same as `bdd_apply(f, g, equiv_op)` and computes *f = g* which is equivalent
  to *xnor*.

- `bdd bdd_diff(bdd f, bdd g)`

  Same as `bdd_apply(f, g, diff_op)` and computes *f ∧ ¬g*.

- `bdd bdd_less(bdd f, bdd g)`

  Same as `bdd_apply(f, g, less_op)` and computes *¬f ∧ g*.

### `bdd bdd_exists(bdd f, label_t var)`
{: .no_toc }

Existential quantification of the variable with label *var* in the BDD for *f*,
i.e. *∃v : f[x<sub>var</sub> / v]*.

### `bdd bdd_exists(bdd f, label_file vars)`
{: .no_toc }

Existentially quantify *f* for variables in *vars* in the very order these
variables are provided in the
[`label_file`](./labels_and_assignments.md#labels).

### `bdd bdd_forall(bdd f, label_t l)`
{: .no_toc }

Forall quantification of the variable with label *var* in the BDD for _f_, i.e.
*∀v : f[x<sub>var</sub> / v]*.

### `bdd bdd_forall(bdd f, label_file ls)`
{: .no_toc }

Forall quantify *f* for variables in *vars* in the very order these variables
are provided in the [`label_file`](./labels_and_assignments.md#labels).

### `bdd bdd_ite(bdd f, bdd g, bdd h)`
{: .no_toc }

Return the BDD representing the if-then-else of *f*, *g*, and *h*, i.e. *f ? g :
h*. In other BDD packages such a function is good for manually constructing a
BDD bottom-up, but for those purposes one should here instead use the
[`bdd_builder`](../manual_construction/builder.md#builderdd_policy-class) class.

### `bdd bdd_not(bdd f)` (operator: `~`)
{: .no_toc }

Return the BDD representing the negation of *f*, i.e. *¬f*.

### `bdd bdd_restrict(bdd f, assignment_file as)`
{: .no_toc }

Return the BDD representing *f[x<sub>s</sub> / v<sub>s</sub>, ..., x<sub>t</sub>
/ v<sub>t</sub>]* for *(s ; v<sub>s</sub>), ..., (t, v<sub>t</sub>)* in *as*.
That is, the BDD for *f* where the variable x<sub>s</sub> is restricted to the
fixed value v<sub>s</sub>, and so on. The
[assignments](./labels_and_assignments.md#assignments) in *as* must be in
increasing order wrt. the label.

## Counting Operations

### `size_t bdd_nodecount(bdd f)`
{: .no_toc }

Return the number of nodes (not counting terminal nodes) in the BDD for *f*.

### `size_t bdd_varcount(bdd f)`
{: .no_toc }

Return the number of variables present in the BDD for *f*.

### `size_t bdd_pathcount(bdd f)`
{: .no_toc }

Return the number of unique (but not necessarily disjoint) paths in the BDD for
*f* that lead to a *true* terminal.

### `size_t bdd_satcount(bdd f, size_t varcount)`
{: .no_toc }

Count the number of satisfying assignments (i.e. the number of *x* such that
*f(x) = 1*), given the total number of expected variables. The given *varcount*
should be greater or equal to its default value, which is the number of
variables within the BDD for *f*, i.e. `bdd_varcount(f)`.

## Predicates

### `bool bdd_equal(bdd f, bdd g)` (operator: `==`)
{: .no_toc }

Returns whether *f* ≡ *g*.

### `bool bdd_unequal(bdd f, bdd g)` (operator: `!=`)
{: .no_toc }

Returns whether *f* and *g* are different functions.

### `bool is_terminal(bdd f)`
{: .no_toc }

Whether the BDD for *f* only consists of a terminal, i.e. *f* is a constant
function.

### `bool value_of(bdd f)`
{: .no_toc }

Assuming that `is_terminal(f)` evaluates to true, i.e. *f* is a constant
function *v*, then returns the value *v*.

### `bool is_false(bdd f)`
{: .no_toc }

Whether *f* is the always false constant function. This is merely shorthand for
`is_terminal(f) && !value_of(f)`.

### `bool is_true(bdd f)`
{: .no_toc }

Whether *f* is the always true constant function. This is merely shorthand for
`is_terminal(f) && value_of(f)`.

## Input variables

### `bool bdd_eval(bdd f, assignment_func x)`
{: .no_toc }

Return *f(x)*, i.e. the evaluation of the given BDD for *f* according to the
total function, [`assignment_func`](./labels_and_assignments.md#assignments) *x*.

### `bool bdd_eval(bdd f, assignment_file x)`
{: .no_toc }

Return *f(x)*, i.e. the evaluation of the given BDD for *f* according to the
[`assignment_file`](./labels_and_assignments.md#assignments) *x*.

### `assignment_file bdd_satmin(bdd f)`
{: .no_toc }

Return the lexicographically smallest
[assignment](./labels_and_assignments.md#files) *x* such that *f(x) =
1*. The variables mentioned in *x* are for all levels in the given BDD.

### `assignment_file bdd_satmax(bdd f)`
{: .no_toc }

Return the lexicographically largest
[assignment](./labels_and_assignments.md#files) *x* such that *f(x) = 1*. The
variables mentioned in *x* are for all levels in the given BDD.

### `label_t min_label(bdd f)`
{: .no_toc }

Return the smallest [label](./labels_and_assignments.md#labels) in the BDD for
*f*, i.e. the label of the root. This is the smallest input variable that has an
effect on the output of *f*.

### `label_t max_label(bdd f)`
{: .no_toc }

Return the largest [label](./labels_and_assignments.md#labels) in the BDD for
*f*, i.e. the label of the deepest node. This is the largest input variable that
has an effect on the output of *f*.

### `label_file bdd_varprofile(bdd f)`
{: .no_toc }

Obtain a [file of labels](./labels_and_assignments.md#files) of the existing
levels in *f*.

## Other Functions

### `bdd bdd_from(zdd A, label_file dom)`
{: .no_toc }

Converts a [`zdd`](zdd.md) into a `bdd` interpreted within the variable domain
*dom*. The domain should be a superset of the variables in the given ZDD. If not
provided, then *dom* is set to the [globally set domain](../settings/domain.md).


## DOT Output

### `void output_dot(bdd f, std::string filename)`
{: .no_toc }

Prints the bdd *f* to a *.dot* file with the given *filename*.

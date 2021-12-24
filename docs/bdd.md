---
layout: default
title: BDD
nav_order: 3
description: "The BDD data structure and the functions provided to manipulate it"
permalink: /bdd
---

# BDD
{: .no_toc }

A Binary Decision Diagram (BDD) represents a boolean function
{: .fs-6 .fw-300 }

<p class="fs-6 fw-300" style="text-align: center;">
  {0,1}<sup>n</sup> → {0,1}
</p>

The `bdd` class takes care of reference counting and optimal garbage collection
of the underlying files (cf. [Core/Files](./core/files#nodes)). To ensure the
most disk-space is available, try to garbage collect the `bdd` objects as
quickly as possible and/or minimise the number of lvalues of said type.

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Basic Constructors

To construct a more complex but well-structured `bdd` than what follows below,
create a [`node_file`](./core/files#nodes) and write the nodes bottom-up with
the [`node_writer`](./core/files#node-writer). The `bdd` object can then be
copy-constructed from the `node_file`.

### `bdd bdd_sink(bool v)`
{: .no_toc }

Create a sink-only BDD with the provided boolean value _v_. Alternatives are
`bdd_true()` and `bdd_false()` or to assign a _bool_ directly to a BDD.
  
### `bdd bdd_ithvar(label_t i)`
{: .no_toc }

Create a BDD representing the literal with the given label _i_
  
### `bdd bdd_nithvar(label_t i)`
{: .no_toc }

Create a BDD representing the negation of the literal with the given label _i_
  
### `bdd bdd_and(label_file ls)`
{: .no_toc }

Construct a BDD representing the _and_ of all the literals with the provided
labels. The _ls_ must be sorted in increasing order.

### `bdd bdd_or(label_file ls)`
{: .no_toc }

Construct a BDD representing the _or_ of all the literals with the provided
labels. The _ls_ must be sorted in increasing order.

### `bdd bdd_counter(label_t min_label, label_t max_label, label_t threshold)`
{: .no_toc }

Construct a BDD that is true if exactly _threshold_ many of the variables in the
interval [*min_label*; *max_label*] are true.

## Basic Manipulation

### `bdd bdd_apply(bdd f, bdd g, bool_op op)`
{: .no_toc }

Construct a BDD representing the *op* applied onto the BDDs for _f_ and _g_. For
each operator, we provide the following alias functions:
  
- `bdd bdd_and(bdd f, bdd g)` (operator `&`)
  
  Same as `bdd_apply(f, g, and_op)` and computes _f ∧ g_.
  
- `bdd bdd_nand(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, nand_op)` and computes _¬(f ∧ g)_.
  
- `bdd bdd_or(bdd f, bdd g)` (operator `|`)

  Same as `bdd_apply(f, g, or_op)` and computes _f ∨ g_.

- `bdd bdd_nor(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, nor_op)` and computes _¬(f ∨ g)_.
  
- `bdd bdd_xor(bdd f, bdd g)` (operator `^`)
  
  Same as `bdd_apply(f, g, xor_op)` and computes _f ⊕ g_.

- `bdd bdd_xnor(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, xnor_op)` and computes _¬(f ⊕ g)_.
  
- `bdd bdd_imp(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, imp_op)` and computes _f ⇒ g_.
  
- `bdd bdd_invimp(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, invimp_op)` and computes _g ⇒ f_.
  
- `bdd bdd_equiv(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, equiv_op)` and computes _f = g_ which is equivalent
  to _xnor_.
    
- `bdd bdd_diff(bdd f, bdd g)`
  
  Same as `bdd_apply(f, g, diff_op)` and computes _f ∧ ¬g_.
  
- `bdd bdd_less(bdd f, bdd g)`
    
  Same as `bdd_apply(f, g, less_op)` and computes _¬f ∧ g_.

### `bdd bdd_ite(bdd f, bdd g, bdd h)`
{: .no_toc }

Return the BDD representing the if-then-else of _f_, _g_, and _h_, i.e. _f ? g :
h_. In other BDD packages such a function is good for manually constructing a
BDD bottom-up, but for those purposes one should here instead use the
[`node_writer`](./core/files#node-writer) class.

### `bdd bdd_not(bdd f)` (operator: `~`)
{: .no_toc }

Return the BDD representing the negation of _f_, i.e. _¬f_.

### `bdd bdd_restrict(bdd f, assignment_file as)`
{: .no_toc }

Return the BDD representing `f[xₛ / vₛ, ..., xₜ / vₜ]` for `{s ; vₛ}`, ...,
`{t ; vₜ}` in *as*. That is, the BDD for _f_ where the variable xₛ is restricted
to the fixed value vₛ, and so on. The assignments in _as_ must be in increasing
order wrt. the label.

### Variable quantification
{: .no_toc }

  - `bdd bdd_exists(bdd f, label_t l)`

     Existential quantification of the variable with label _l_ in the BDD for
     _f_, i.e. _∃v : f[xₗ / v]_.
     
  - `bdd bdd_exists(bdd f, label_file ls)`
  
     Existentially quantify _f_ for variables in _ls_ in the very order these
     variables are provided.

  - `bdd bdd_forall(bdd f, label_t l)`

     Forall quantification of the variable with label _l_ in the BDD for _f_,
     i.e. _∀v : f[xₗ / v]_.
     
  - `bdd bdd_forall(bdd f, label_file ls)`
  
     Forall quantify _f_ all variables in _ls_ in the very order these variables
     are provided.


## Counting Operations

### `uint64_t bdd_nodecount(bdd f)`
{: .no_toc }

Return the number of nodes (not counting sink nodes) in the BDD for _f_.

### `uint64_t bdd_varcount(bdd f)`
{: .no_toc }

Return the number of variables present in the BDD for _f_.

### `uint64_t bdd_pathcount(bdd f)`
{: .no_toc }

Return the number of unique (but not necessarily disjoint) paths in the BDD for
_f_ that lead to a _true_ sink.

### `uint64_t bdd_satcount(bdd f, size_t varcount)`
{: .no_toc }

Count the number of satisfying assignments (i.e. the number of _x_ such that
_f(x) = 1_), given the total number of expected variables. The given _varcount_
should be greater or equal to its default value, which is the number of
variables within the BDD for _f_, i.e. `bdd_varcount(f)`.

## Other Functions

### `bool bdd_eval(bdd f, T x)`
{: .no_toc }

Return _f(x)_, i.e. the evaluation of the given BDD for _f_ according to the
assignment _x_. The type _T_ of _x_ can either be any function _label_t → bool_
(e.g. a lambda function) or an *assignment_file*.

### `assignment_file bdd_satmin(bdd f)`
{: .no_toc }

Return the _lexicographically smallest_ _x_ such that _f(x) = 1_. The variables
mentioned in _x_ are for all levels in the given BDD.

### `assignment_file bdd_satmax(bdd f)`
{: .no_toc }

Return the _lexicographically largest_ _x_ such that _f(x) = 1_. The variables
mentioned in _x_ are for all levels in the given BDD.

### `bool is_sink(bdd f, sink_pred)`
{: .no_toc }

Whether the BDD for _f_ only consists of a sink satisfying the given sink
predicate, i.e. _f_ is a constant function. By default the predicate for _any_
kind of sink is used.

### `label_t min_label(bdd f)`
{: .no_toc }

Return the smallest label in the BDD for _f_, i.e. the label of the root.

### `label_t max_label(bdd f)`
{: .no_toc }

Return the largest label in the BDD for _f_, i.e. the label of the deepest node.

## DOT Output

### `void output_dot(bdd f, std::string filename)`
{: .no_toc }

Prints the bdd _f_ to a _.dot_ file with the given _filename_.

---
layout: default
title: Builder
nav_order: 1
parent: Manual Construction
description: "Builder Class"
permalink: manual_construction/builder
---

# Builder
{: .no_toc }

The builder classes provide easy-to-use interfaces for a manual bottom-up
construction of a BDD and a ZDD that is similar to manual construction in any
other BDD package.
{: .fs-6 .fw-300 }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## `builder<dd_policy>` class

The builder classes are implemented as a single templated class
`builder<dd_policy>` where the template argument carries the logic related to
the specific type of decision diagram. These builders provide the user with
"pointers" to the nodes created within for use them during construction. In what
follows we will write `dd` as a placeholder for both
[`bdd`](../data_structures/bdd.md) and [`zdd`](../data_structures/zdd.md).

We provide the following aliases for both the builder and their pointers so one
does not need to take the templated argument into account.

### `bdd_builder`
{: .no_toc }

Builder for Binary Decision Diagram ([`bdd`](../data_structures/bdd.md)).

### `zdd_builder`
{: .no_toc }

Builder for Zero-suppressed Decision Diagram ([`zdd`](../data_structures/zdd.md)).

---

### Constructors
{: .no_toc }

### `dd_builder::dd_builder()`
{: .no_toc }

Default constructor. After construction it is ready to add nodes with any of the
*add_node* member functions below.

---

### Member Functions
{: .no_toc }

### `dd_ptr dd_builder::add_node(bool v)`
{: .no_toc }

Adds a terminal node with a given value *v*. This may be called at any time,
regardless of whether any other nodes have been created.

### `dd_ptr dd_builder::add_node(label_t label, dd_ptr low, dd_ptr high)`
{: .no_toc }

Adds a decision diagram node with a given *label* and the given *low* and *high*
child.

This will apply the first reduction rule associated with the specific type of
decision diagram `dd`, e.g. not construct a *don't care* node for BDDs but
instead just return its child. The builder cannot apply the second reduction
rule, i.e. merging of duplicate nodes, so that is still left up to you to do.

This will throw a `std::invalid_argument` exception in the following cases:
- The provided *low* or *high* pointers originate from another builder object.
- Nodes are not added bottom-up, i.e. the *label* provided is strictly larger
  than the label of the previously added non-terminal node.
- The provided *label* is larger than what is supported by *Adiar*.
- It violates the variable ordering, i.e. the label of one or both its children
  is smaller than or equals to the provided *label*.

### `dd_ptr dd_builder::add_node(label_t label, bool low, dd_ptr high)`
{: .no_toc }

Similar to `add_node(label, add_node(low), high)`.

### `dd_ptr dd_builder::add_node(label_t label, dd_ptr low, bool high)`
{: .no_toc }

Similar to `add_node(label, low, add_node(high))`.

### `dd_ptr dd_builder::add_node(label_t label, bool low, bool high)`
{: .no_toc }

Similar to `add_node(label, add_node(low), add_node(high))`.

### `dd dd_builder::build()`
{: .no_toc }

Build the final decision diagram `dd` object with the nodes that were added
prior to this call. This will also immediately [*clear()*](#void-builderclear)
it to create a new one.

This will throw a `std::domain_error` if
- *add_node(...)* has not been called, i.e. no nodes have been added before this.
- The resulting decision diagram has more than one root.

### `void dd_builder::clear()`
{: .no_toc }

Clear the builder of all its current content, discarding all nodes and
invalidating any pointers to them. This is similar to constructing a new
builder.

## Example

Consider the following BDD, equivalent to the formula (x<sub>0</sub> ∧
x<sub>1</sub>) ∨ x<sub>2</sub>.


![Example (a) of a BDD](./example_a.png){: style="max-width: 10rem; margin: 0 auto; display: block" }


This can be constructed as follows

```cpp
bdd_builder b;

const bdd_ptr p2 = b.add_node(2, false, true);
const bdd_ptr p1 = b.add_node(1, false, true);
const bdd_ptr p0 = b.add_node(0, p2, p1);

bdd example_a = b.build();
```

Notice, construction has to be done bottom up! So, one has to create the node
for *x<sub>2</sub>* first even though the subtrees stored in *p1* and *p2* are
disjoint.


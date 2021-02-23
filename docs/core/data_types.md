---
layout: default
title: Data types
nav_order: 1
parent: Core
description: "The data types placed in streams"
permalink: core/data_types
---

# Data types
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Nodes

### Identifiers and ordering
A non-sink node is uniquely identified by two values: its _label_ and its _id_.
The prior is the variable it represents, whereas _id_ is a unique number
specific to the number's _label_. Together they create a unique identifier _uid_
of a node. This identifier is supposed to reflect the following total ordering.

<p style="text-align: center;">
  n < m ≡ n.label < m.label || (n.label = m.label && n.id < m.id)
</p>

These uids can be stored within a single unsigned 64-bit integer, which then
acts as a "pointer" to the node and can be constructed as follows.

- `uid_t create_node_uid(label_t l, id_t id)`

  Create the unique identifier for the node with label _l_ and identifier _id_.
  We provide `ptr_t create_node_ptr(l, id)` as an alternative.

One then must use the following two functions to again retrieve the label or id
from a uid.

- `label_t label_of(uid_t u)`

- `id_t id_of(uid_t u)`

Here `id_t`, `uid_t`, and `ptr_t` are aliases for an unsigned 64-bit integer and
`label_t` for an unsigned 32-bit integer. The uid constructed (and the
algorithms by extension) only works as intended, if the label, resp. id, is less
or equal to `MAX_LABEL`, resp. `MAX_ID`.

A unique identifier for a sink is recognised by a single bit-flag within the
64-bit number of the identifer. One can create, read from, and manipulate
sink-identifiers by using the following functions.

- `uid_t create_sink_uid(bool v)`

  Create the unique identifier to a sink with a given boolean value. We also
  provide `ptr_t create_sink_ptr(v)` as an alternative.

- `bool value_of(ptr_t p)`

  Extract the boolean value of the given sink identifier.

- `uid_t negate(ptr_t p)`

  Negate the value of the sink

One can identify whether a given `ptr_t` or `uid_t` is to a node or a sink with
the following two predicates.

- `bool is_node(uid_t p)`

- `bool is_sink(uid_t p)`

### The Node struct

With the above a node in _Adiar_ is the following combination of 3 unsigned
64-bit numbers

```c++
struct node { uid_t uid; ptr_t low; ptr_t high; };
```

For which the operators `<` , `>`, `==`, and `!=` have been defined to reflect
the ordering based on the uid discussed above. For convenience, one can create a
node with the following function.

- `node_t create_node(label_t label, id_t id, uid_t low, uid_t high)`

  Create a node, given a label and an id for the specific node, together with
  the identifier for its low and high child. We also provide variants of this
  function, where `low` and `high` are themselves nodes.

 The functions on uids above are also lifted to nodes

- `label_t label_of(node_t n)`

  Extract the label from within the uid of a non-sink node.

- `id_t id_of(node_t n)`

  Extract the id from within the uid of a non-sink node.

- `node_t create_sink(bool value)`

  Create a sink node with a specific boolean value.

- `bool is_sink(node_t n)`

  Assert whether the node is a sink node.

- `bool value_of(node_t n)`

  Extract the value of the given sink node.

- `node_t negate(node_t n)` (operator `!`)

  Negates the content of a node. If the node is a sink, then its value is
  negated, otherwise if it has a pointer to a sink, then that pointer is
  negated.


## Assignments

An assignment to a variable depends on the _label_ of the variable together with
the boolean _value_ it is assigned to.

```c++
struct assignment { label_t label; bool value; };
```

For which the operators `<` , `>`, `==`, and `!=` have been defined to reflect
an increasiing ordering on the label. The `!` operator negates the value in the
assignment.

- `assignment_t create_assignment(label_t label, bool value)`

  Create an assignment given a label and value.

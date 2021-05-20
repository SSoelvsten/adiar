---
layout: default
title: Files
nav_order: 2
parent: Core
description: "Files, file streams, and file writers"
permalink: core/files
---

# Files
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Nodes

BDDs are stored on disk with the specific ordering mentioned in Section
[Core/Data types](./data_types#identifiers-and-ordering). If you want to
construct a BDD by hand then you have to explicitly follow this ordering;
otherwise the algorithms will have _undefined behaviour_. Yet, that is not the
whole story: In _Adiar_ a set of nodes are stored in a `node_file` in which all
these nodes are stored in _reverse_ of the ordering.


### Node Stream

One can then read from a `node_file` by use of the `node_stream<bool>` class,
where the boolean template argument specifies whether the content should be
reversed (default is an in-order traversal of the nodes). This class attaches to
the `node_file` (and the [BDD](../bdd) wrapping class) on construction and
detaches again on deconstruction. The class provides the following member
functions

- `void reset()`

  Reset the node_stream back to its beginning.
  
- `bool can_pull()`

  Return whether there is a next element to pull.

- `const node pull()`

  Get the next node from the stream and move to the next.

- `const node peek()`

  Get the next node from the stream without moving to the next.

### Node Writer

To follow the ordering in a `node_file` one has to write nodes bottom-up and in
reverse for each level with respect to the _id_. One can write nodes to the file
by use of the `node_writer` object, that can be constructed in two ways.

- `node_writer()`

  Construct a `node_writer` attached to nothing.

- `node_writer(const node_file)`

  Construct a `node_writer` attached to the given `node_file`.

The `node_writer` class provides the following member functions

- `void push(node_t n)` (operator `<<`)

  Push a single node to the `node_file`. This also applies a few sanity checks
  on the provided input, such as checks on the ordering.

- `void attach(node_file f)`

  Attach the `node_writer` to a given `node_file`.

- `bool attached() const`

  Whether the current `node_writer` currently is attached to a `node_file`.

- `void detach()`

  Detach the `node_writer` from its current `node_file`, if any.

It is important to note, that one cannot have multiple `node_writers` attached
to the same `node_file`. Furthermore, one also has to detach the `node_writer`
before anything can be read from the `node_file` or that the algorithms of
_Adiar_ can process on them. So, remember to either detach it explicitly or have
the `node_writer` destructed before calling any such functions.

#### A note on equality checking

The _equality checking_ algorithm (`==`) of _Adiar_ exploits multiple
characteristics of decision diagrams to speed up its computation. If you want to
use your self-made decision diagrams in equality checking, then you have to be
sure to adhere to write it in its reduced form.

1. Do not write nodes that are suppressed in the decision diagram, e.g. for BDDs
   do not write nodes with _low_ == _high_.

2. Do not write duplicate nodes to the same file, i.e. nodes where both their
   _low_ children are the same and their _high_ children are the same.

Equality checking also is much faster if the constructed decision diagram is on
_canonical_ form. The decision diagram being canonical means that it also
satisfies the following two constraints.

1. The first node pushed to each level has _id_ `MAX_LABEL`, the next has _id_
   `MAX_LABEL - 1`, and so on.

2. The nodes within each level are lexicographically ordered by their children
  (_high_ first then _low_). That is, a node _n_ written to the file after
  already having written node _m_ on the same level must not only satisfy _n <
  m_ from Section [Core/Data types](./data_types#identifiers-and-ordering) but
  also the following extended constraint

<p style="text-align: center;">
  n.id < m.id ≡ n.high < m.high || (n.high = m.high && n.low < m.low)
</p>

## Assignments and Labels

Some functions take a list of assignments (cf. [Core/Data types](./data_types#assignments)))
as input or return them as an output. To create such input, resp. traverse such
output, one can use the `assignment_writer`, resp. `assignment_stream`. Other
functions work on lists of labels (cf. [Core/Data types](./data_types#identifiers-and-ordering))
for which we provide the `label_writer` and `label_stream`.

These writers provide the same interface as the `node_writer`. They also have
sanity checks on the ordering of their elements when pushing elements. The
streams also provide the same member functions as above with the addition of the
`attach(x_file)`, `attached()`, and `detach()` member functions.


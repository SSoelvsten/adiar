---
layout: default
title: Nodes and Writers
nav_order: 2
parent: Manual Construction
description: "Nodes, their Files and Writers"
permalink: manual_construction/node_writer
---

# Nodes, their Files, and their Writers
{: .no_toc }

The [builder](./builder.md) provides an abstraction of the representation of
decision diagrams in *Adiar* at a minor cost to the performance of manually
constructing. An expert user may desire to get closer to the raw files for the
sake of performance.
{: .fs-6 .fw-300 }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Unique Identifiers

A non-terminal node is uniquely identified by a tuple (*i*, *id*). The first
entry, *i*, is the label of the variabel whereas *id* is an identifier of that
specific node. This identifier only needs to be unique on each level, i.e. for
each *i*, since nodes are still uniquely identifiable by the combination of
their label and identifier. This tuple create a unique identifier *uid* of a
node which substitute as a "pointer" to it.

| C++ Type   | Type      | Semantics                                                           |
|------------|-----------|---------------------------------------------------------------------|
| `uint32_t` | `label_t` | The label, i.e. input variable, of a BDD node.                      |
| `uint64_t` | `id_t`    | The level-identifier of a node, i.e. the index on said level.       |
| `uint64_t` | `uid_t`   | Unique identifier of a node consisting of a label and an identifier |

For sake of performance, these uids are encoded as an unsigned 64-bit integer.
Hence, the label and the id cannot make use of all their bits available and so
their values are upper bounded by the constants `MAX_LABEL` and `MAX_ID`.

### `uid_t create_node_uid(label_t l, id_t id)`
{: .no_toc }

Create the unique identifier for the node with label *l* and identifier *id*.

### `label_t label_of(uid_t u)`
{: .no_toc }

Obtain the label from a unique identifier.

### `id_t id_of(uid_t u)`
{: .no_toc }

Obtain the label from a unique identifier.

### `uid_t create_terminal_uid(bool v)`
{: .no_toc }

Create the unique identifier to a terminal with a given boolean value.

### `bool value_of(ptr_t p)`
{: .no_toc }

Extract the boolean value of the given terminal identifier.

### `uid_t negate(ptr_t p)`
{: .no_toc }

Negate the value of the terminal

### `bool is_node(uid_t p)`
{: .no_toc }

Whether a unique identifier is for a non-terminal decision diagram node.

### `bool is_terminal(uid_t p)`
{: .no_toc }

Whether a unique identifier is for a terminal node.

### `bool is_false(uid_t p)`
{: .no_toc }

Shorthand for `is_terminal(p) && !value_of(p)`.

### `bool is_true(uid_t p)`
{: .no_toc }

Shorthand for `is_terminal(p) && value_of(p)`.

## Nodes

With the above a node in _Adiar_ is the following combination of 3 unsigned
64-bit numbers

```cpp
struct node { uid_t uid; uid_t low; uid_t high; };

typedef node node_t;
```

### `node_t create_node(label_t label, id_t id, uid_t low, uid_t high)`
{: .no_toc }

Create a node, given a label and an id for the specific node, together with the
identifier for its low and high child. We also provide variants of this
function, where `low` and `high` are of type `node_t`.

### `label_t label_of(node_t n)`
{: .no_toc }

Extract the label of a non-terminal node, i.e. `label_of(n.uid)`.

### `id_t id_of(node_t n)`
{: .no_toc }

Extract the id of a non-terminal node, i.e. `id_of(n.uid)`.

### `node_t create_terminal(bool value)`
{: .no_toc }

Create a terminal node with a specific boolean value.

### `bool is_terminal(node_t n)`
{: .no_toc }

Assert whether the node represents a terminal node.

### `bool value_of(node_t n)`
{: .no_toc }

Extract the value of the given terminal node.

### `bool is_false(node_t)`
{: .no_toc }

Whether *n* represents the false terminal, i.e. similar to `is_terminal(n) && !value_of(n)`.

### `bool is_true(node_t)`
{: .no_toc }

Whether *n* represents the false terminal, i.e. similar to `is_terminal(n) && value_of(n)`.

### `node_t negate(node_t n)` (operator `!`)
{: .no_toc }

Negates a node
- If it is a terminal, then its value is negated
- If it is a non-terminal node with a pointer to a terminal, then said pointer is negated.

## `node_file` class

A decision diagram is represented in *Adiar* as a list of nodes that are in
reverse of the following total ordering of their [unique
identifiers](#unique-identifiers).

<p style="text-align: center;">
  (i<sub>1</sub>, id<sub>1</sub>) < (i<sub>2</sub>, id<sub>2</sub>) ≡
  i<sub>1</sub> < i<sub>2</sub> || (labe<sub>1</sub> = i<sub>2</sub> && id<sub>1</sub> < id<sub>2</sub>)
</p>

That is, decision diagrams in *Adiar* are list of nodes in *descending* order
with respect to the nodes' unique identifiers.

For example, consider the following four BDDs.

![Examples BDDs](./example_dags.png)

These are represented in *Adiar* as a `node_file` with the following list of
nodes.

- (a) : [ { (2,0) , ⊥ , ⊤ } ]
- (b) : [ { (1,0) , ⊥ , ⊤ }, { (0,0) , ⊥ , (1,0) } ]
- (c) : [ { (1,1) , ⊤ , ⊥ }, { (1,0) , ⊥ , ⊤ }, { (0,0) , (1,0) , (1,1) } ]
- (d) : [ { (2,0) , ⊥ , ⊤ }, { (1,0) , (2,0) , ⊤ } ]

### Equality Checking and Canonicity

Since *Adiar* represents each BDD in a separate file, then it cannot merely
check for equality by merely comparing pointers in constant time. Instead,
multiple features of reduced decision diagrams are exploited to speed up its
computation.

If your manually constructed decision diagrams is supposed to be used in the
context of equality checking then you have to be sure to adhere to write it in
its reduced form.

1. Do not write nodes that are suppressed in the decision diagram, e.g. for BDDs
   do not push nodes where *low* == *high*.

2. Do not write duplicate nodes to the same file, i.e. nodes where both their
   *low* children are the same and their *high* children are the same.

Equality checking also is much faster if the constructed decision diagram is on
*canonical* form. The decision diagram being canonical means that it also
satisfies the following two constraints.

1. The first node pushed to each level has *id* `MAX_ID`, the next has *id*
   `MAX_ID - 1`, and so on.

2. The nodes within each level are lexicographically ordered by their children
  (*high* first then *low*). That is, a node *n* written to the file after
  already having written node *m* on the same level must not only satisfy *n.uid
  < m.uid* but also the following extended constraint

<p style="text-align: center;">
  n.id < m.id ≡ n.high < m.high || (n.high = m.high && n.low < m.low)
</p>

Note that the above examples are **not** canonical. Their identifiers are not
starting at `MAX_ID` and in (b) are the two nodes on level *1* in the wrong
order.

### Meta variables

A `node_file` also contains several pieces of meta information, but you can skip
this section since the [`node_writer`](node_writer) class provides the
`.push(node_t n)` function takes care of computing these very values. Yet, to
further improve performance you can skip the logic and use the `.unsafe_push`
function instead and then provide these values yourself.

Below is a list of all pieces of meta information stored in a `node_file`.
Points that are checkmarked are updated by the `.unsafe_push(node_t)` function
and require no updates from the user.

#### Level Information
{: .no_toc }

Next to the file containing nodes, there is another file with `level_information`

```cpp
struct level_info { label_t label; size_t width; };
```

The examples above would have the following level information stored.

- (a) : [ { 2, 1 } ]
- (b) : [ { 1, 1 } , { 0, 1 } ]
- (c) : [ { 1, 2 } , { 0, 1 } ]
- (d) : [ { 2, 1 } , { 1, 1 } ]

#### Canonicity
{: .no_toc }

A single boolean value `canonical` (default `false`) marks whether the nodes
are on a [*canonical form*](#equality-checking-and-canonicity).

#### Number of Terminals
{: .no_toc }

The 2-length array `number_of_terminals` includes the number of arcs (directed
edges) from an internal node to a terminal. The *0* index includes the number of
arcs to *false* whereas *1* the number of arcs to *true*.

In the four examples above these ought to be set to.

- (a) : [ 1, 1 ]
- (b) : [ 2, 1 ]
- (c) : [ 2, 2 ]
- (d) : [ 1, 2 ]

#### Maximum Levelized Cut
{: .no_toc }

The 4-length array `max_1level_cut` and `max_2level_cut` provide an upper bound
on the maximum *1*-level and *2*-level cuts in the directed acyclic graph (DAG).
A *1-level* cut divides the DAG in two right in-between two levels. A *2-level*
cut has a bit more wiggle-room in its shape: nodes on one specific level may
choose to be on either side of the cut depending on whether they have more
in-going or out-going arcs.
  
The four entries in the array correspond to the size of the cut of a specific
subgraph with or withour arcs to one or both terminals. The following enum in
`<adiar/internal/cut.h` can be used to index properly into the array.
  
| Index | Enum                       | Subgraph                     |
|-------|----------------------------|------------------------------|
| 0     | `cut_type::INTERNAL`       | Internal arcs only           |
| 1     | `cut_type::INTERNAL_FALSE` | Internal and false arcs only |
| 2     | `cut_type::INTERNAL_TRUE`  | Internal and true arcs only  |
| 3     | `cut_type::ALL`            | All arcs                     |

In the above four examples, the *1-level* and *2-level* cuts are the same and
they should be any list that for each entry is greater than or eqauls to the
following.
  
- (a) : [ 1, 1, 1, 2 ]
- (b) : [ 1, 2, 1, 3 ]
- (c) : [ 2, 2, 2, 4 ]
- (d) : [ 1, 1, 2, 3 ]
  
Note that in (a) the number of *internal* arcs is *1* because there is an
ingoing arc to the root.

By default, when a `node_write` detaches from a file then these values are set
to *N+1* where *N* is the number of nodes. So, you may choose to do nothing
yourself. Yet, these values are used to improve performance of other algorithms
so the tighter these values are, the better. If the *1-level* cut values are set
to something lower, then from it is derived a tighter 3/2 factor upper bound of
the *2-level* cut. If the *2-level* cut values are set to something even lower
than that, then these values are also set.

## `node_writer` class

The `node_writer` class is used to write content into a `node_file`. It both
provides a `.push(...)` and an `.unsafe_push(...)` member function. You should
only use one of the two, when constructing a decision diagram - doing otherwise
leads to undefined (and unknown) behaviour.

---

### Constructors
{: .no_toc }

### `node_writer::node_writer()`
{: .no_toc }

Construct the writer attached to nothing.

### `node_writer::node_writer(const node_file)`
{: .no_toc }

Construct the writer and immediately attach to the given *node_file*.

### `node_writer::~node_writer(const node_file)`
{: .no_toc }

Detaches from any current file to which it is attached before destruction.

---

### Member Functions
{: .no_toc }

### `void node_writer::push(node_t n)` (operator `<<`)
{: .no_toc }

Push the node *n* to the end of the file containing nodes and update
the meta information.

### `void node_writer::unsafe_push(node_t n)`
{: .no_toc }

Push the node *n* to the end of the file containing nodes. This does not update
any meta information.

### `void node_writer::unsafe_push(level_info li)`
{: .no_toc }

Push *li* to the end of the file containing level information. This does not
update any other meta information.

### `size_t node_writer::size()`
{: .no_toc }

The number of nodes pushed to the file.

### `void node_writer::attach(simple_file<T> f)`
{: .no_toc }

Attach the writer to the given file.

### `bool node_writer::attached()`
{: .no_toc }

Whether the writer is attached to a file.

### `void node_writer::detach()`
{: .no_toc }

Detach the writer from its current file, if any. This will also update the meta
information with sound upper bounds for the *1-level* and the *2-level* cuts.

## Examples

We will construct the following BDD in two ways with the `node_writer`.

![Example (a) of a BDD](./example_a.png){: style="max-width: 10rem; margin: 0 auto; display: block" }

### Using `node_writer::push(...)`

When using the `node_writer::push(node_t)` function one only needs to construct
the nodes and push them in the correct order.

```cpp
node_file nf;

{ // Scope to get node_writer to deconstruct and detach early
  node_writer nw(nf);
  
  const uid_t terminal_F = create_terminal_uid(false);
  const uid_t terminal_T = create_terminal_uid(true);
  const node_t n2 = create_node(2, MAX_ID, terminal_F, terminal_T);
  const node_t n1 = create_node(1, MAX_ID, terminal_F, terminal_T);
  const node_t n0 = create_node(0, MAX_ID, n2, n1);
  
  nw << n2 << n1 << n0;
}

bdd result(nf);
```

### Using `node_writer::unsafe_push(...)`

If one wants to skip the cost of the logic inside of the *push* function then
one also needs to add the necessary meta information.

```cpp
node_file nf;

{ // Scope to get node_writer to deconstruct and detach early
  node_writer nw(nf);
  
  // Nodes
  const uid_t terminal_F = create_terminal_uid(false);
  const uid_t terminal_T = create_terminal_uid(true);

  const node_t n2 = create_node(2, MAX_ID, terminal_F, terminal_T);
  nw.unsafe_push(n2);

  const node_t n1 = create_node(1, MAX_ID, terminal_F, terminal_T);
  nw.unsafe_push(n1);

  const node_t n0 = create_node(0, MAX_ID, n2, n1);
  nw.unsafe_push(n0);
  
  // Level information
  nw.unsafe_push(create_level_info(2,1));
  nw.unsafe_push(create_level_info(1,1));
  nw.unsafe_push(create_level_info(0,1));

  // Canonicity
  nf->canonical = true;

  // Number of Terminals
  //   Is already taken care of by '.unsafe_push(node_t n)', so it should not
  //   be updated.

  // 1-level cut
  nf->max_1level_cut[cut_type::INTERNAL] = 2;
  nf->max_1level_cut[cut_type::INTERNAL_FALSE] = 3;
  nf->max_1level_cut[cut_type::INTERNAL_TRUE] = 3;
  nf->max_1level_cut[cut_type::ALL] = 4;

  // 2-level cut
  //   Technically, the destructor of the 'node_writer' already notices during
  //   the '.detach()' that each level is a single node wide and so the 1-level
  //   and 2-level cut must be the same.
  //
  //   Yet, for the sake of completeness.
  nf->max_2level_cut[cut_type::INTERNAL] = 2;
  nf->max_2level_cut[cut_type::INTERNAL_FALSE] = 3;
  nf->max_2level_cut[cut_type::INTERNAL_TRUE] = 3;
  nf->max_2level_cut[cut_type::ALL] = 4;
}

bdd result(nf);
```

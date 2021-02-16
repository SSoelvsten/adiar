# Core

Most of the features of _Adiar_ can be used without knowing anything about how
the underlying algorithms work. Yet, few BDD functions involve a `label_file` or
`assignment_file` (c.f. [Assignment and Labels](#assignments-and-labels)), and
especially an efficient manual construction of a well structured BDDs will
require direct interaction with the underlying data types and files.

**Table of Contents**

- [Data types](#data-types)
    - [Nodes and Pointers](#nodes-and-pointers)
    - [Assignments](#assignments)
- [Files](#files)
    - [Nodes](#nodes)
        - [Node Stream](#node-stream)
        - [Node Writer](#node-writer)
    - [Assignments and Labels](#assignments-and-labels)

## Data types

### Nodes and Pointers
A non-sink node is uniquely identified by two values: its _label_ and its _id_.
The prior is the variable it represents, whereas _id_ is a unique number
specific to the number's _label_. Together they create a unique identifier _uid_
of a node. This identifier is supposed to reflect the following total ordering.

<p style="text-align: center;">
  n < m ≡ n.label < m.label || (n.label = m.label && n.id < m.id)
</p>

These uids can be stored within a single unsigned 64-bit integer, which then
acts as a "pointer" to the node and can be constructed as follows.

- `uid_t create_node_uid(label_t label, id_t id)`

  Create the identifier for the node with label `label` and identifier `id`.
  We provide `ptr_t create_node_ptr(label, id)` as an alternative.

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

  Create the identifier to a sink with a given boolean value. We also provide
  `ptr_t create_sink_ptr(v)` as an alternative.

- `bool value_of(ptr_t p)`

  Extract the boolean value of the given sink identifier.

- `uid_t negate(ptr_t p)`

  Negate the value of the sink

One can identify whether a given `ptr_t` or `uid_t` is to a node or a sink with
the following two predicates.

- `bool is_node(uid_t p)`

- `bool is_sink(uid_t p)`

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


### Assignments

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

## Files

All algorithms of _Adiar_ rely on exploiting an explicit ordering of elements on
disk.

### Nodes

BDDs are stored on disk with the specific ordering mentioned above (c.f. [Nodes
and Pointers](#nodes-and-pointers)). If you want to construct a BDD by hand then
you have to explicitly follow this ordering; otherwise the algorithms will have
_undefined behaviour_. Yet, that is not the whole story: In _Adiar_ a set of
nodes are stored in a `node_file` in which all these nodes are stored in
_reverse_ of the ordering above.

#### Node Stream

One can then read from a `node_file` by use of the `node_stream<bool>` class,
where the boolean template argument specifies whether the content should be
reversed (default is an in-order traversal of the nodes, as described
[above](#nodes-and-pointers)). This class attaches to the `node_file` (and the
[BDD](/bdd.md) wrapping class) on construction and detaches again on
deconstruction. The class provides the following member functions

- `void reset()`

  Reset the node_stream back to its beginning.
  
- `bool can_pull()`

  Return whether there is a next element to pull.

- `const node pull()`

  Get the next node from the stream and move to the next.

- `const node peek()`

  Get the next node from the stream without moving to the next.

#### Node Writer

To follow the ordering in a `node_file` one has to write nodes bottom-up and in
reverse for each layer with respect to the _id_. One can write nodes to the file
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

### Assignments and Labels

Some functions take a list of [assignments](#assignment) as input or return them
as an output. To create such input, resp. traverse such output, one can use the
`assignment_writer`, resp. `assignment_stream`. Other functions work on lists of
[labels](#nodes-and-pointers) for which we provide the `label_writer` and
`label_stream`.

These writers provide the same interface as the `node_writer`. They also have
sanity checks on the ordering of their elements when pushing elements. The
streams also provide the same member functions as above with the addition of the
`attach(x_file)`, `attached()`, and `detach()` member functions.


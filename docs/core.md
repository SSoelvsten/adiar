# Core

**Table of Contents**

- [Nodes and Pointers](#nodes-and-pointers)
- [Assignment](#nodes-and-pointers)
- [Files](#files)


## Nodes and Pointers
A non-sink node is uniquely identified by two values: its _label_ and its _id_.
The prior is the variable it represents, whereas _id_ is a unique number
specific to the numbers _label_. Together they create a unique identifier _uid_
of a node, that is supposed to reflect the following total ordering of nodes.

<p style="text-align: center;">
  _n_ < _m_ := _n_._label_ < _m_._label_ || (_n_._label_ = _m_._label_ && _n_._id_ < _m_.id)
</p>

These _uid_s can be stored within a single unsigned 64-bit integer, which then
acts as a "pointer" to the node and can be constructed as follows.

- `uid_t create_node_uid(label_t label, id_t id)`

- `ptr_t create_node_uid(label_t label, id_t id)`

T retrieve values of a _uid_ one may use one of the following functions

- `label_t label_of(uid_t u)`
- `id_t id_of(uid_t u)`

Here `label_t`, `id_t`, `uid_t`, and `ptr_t` are aliases for an unsigned 64-bit
integer. The _uid_ constructed (and the algorithms by extension) only works as
intended, if the _label_, resp. _id_, is less or equal to `MAX_LABEL`, resp.
`MAX_ID`.

A unique identifier for a sink is recognised by a single bit-flag within the
64-bit number of the identifer. One can create, read from, and manipulate
sink-identifiers by using the following functions.

- `ptr_t create_sink_ptr(bool v)`
  Creates the identifier/pointer to a sink with a given value

- `bool value_of(ptr_t u)`
  Extracts the value of the sink from the 

- `uid_t negate(ptr_t u)`

At this point a node in _Adiar_ is the following combination 3 unsigned 64-bit
numbers

```c++
struct node { uint64_t uid; uint64_t low; uint64_t high; };
```

For which the operators `<` , `>`, `==`, and `!=` have been defined to reflect
the ordering based on the _uid_ discussed above.

- `node_t create_node(label_t label, id_t id, uid_t low, uid_t high)`

  Creates a node, given a _label_ and an _id_ for the specific node, together
  with the identifier for its low and high child. We also provide variants of
  this function, where `low` and `high` provided are themselves nodes.

- `label_t label_of(node_t n)`

  Extract the _label_ from within the _uid_ of a non-sink node.

- `id_t id_of(node_t n)`

  Extract the _id_ from within the _uid_ of a non-sink node.

- `node_t create_sink(bool value)`

  Create a sink node with a specific boolean value.

- `bool is_sink(node_t n)`

  Asserts whether the node is a sink or an internal node.

- `bool value_of(node_t n)`

  Extract the value of the given sink node.

- `node_t negate(node_t n)` (operator `!`)

  Negates the content node. If the node is a sink, then its value is negated,
  otherwise if it has a pointer to a sink, then that pointer is negated.


## Assignment

An assignment to a variable depends on the _label_ of the variable together with
the boolean _value_ it is assigned to.

```c++
struct assignment { label_t label; bool value; };
```

## Files
All algorithms of _Adiar_ rely on having the BDDs stored on disk with the
specific exploitable ordering mentioned above (c.f. [Nodes and
Pointers](#nodes-and-pointers)). To construct a BDD by hand, one has to
explicitly follow this ordering; otherwise the algorithms will have _undefined
behaviour_.

In _Adiar_ a set of nodes are stored in a `node_file`, in which all these nodes
are stored with respect to the reverse of the ordering above. That is, one has
to write nodes bottom-up in reverse for each layer with respect to the _id_. One
can write nodes to the file by use of the `node_writer` object. One can either
construct a `node_writer` by itself and then attach it to a file or construct it
attached to a specific `node_file`.

One can use the `node_writer` using its following member functions

- `void push(node_t n)` (operator `<<`)

  Push a single node to the `node_file`. This also applies a few sanity checks
  on the provided input, based on the ordering.

- `void attach(node_file f)`

  Attaches the `node_writer` to a given `node_file`.

- `bool attached() const`

  Whether the current `node_writer` currently is attached to a `node_file`.

- `void detach()`

  Detaches the `node_writer` from its current `node_file`, if any.

One cannot have multiple `node_writers` attached to the same `node_file`, but it
is more important to point out, that one also has to detach the `node_writer`
before anything can be read from the `node_file` or any streams canbe attached
to it from within the _Adiar_ algorithms. So, remember to either detach it
explicitly or have the `node_writer` destructed before calling any such
functions.

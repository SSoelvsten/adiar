COOM is a library for manipulating _Ordered Boolean Decision Diagrams_ even when
these grow bigger than the memory one has available. To achieve this, all
operations are not implemented recursively with a global memoization table, but
instead they are done as streaming algorithms that exploit the sorting of the
graph on disk to delay recursion with _Time-Forward Processing_.

**Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

# Table of Content

- [**Installation**](/install.md)
- **Data types**
  - [**Nodes and Arcs**](/data.md)
  - [**Streams**](/streams.md)
- **Functions**
  - [**Core**](/core.md)
  - [**Operators**](/operators.md)
    - [**Apply**](/operators.md#apply)
    - [**Counting**](/operators.md#count)
    - [**Evaluate**](/operators.md#evaluate)
    - [**Negation**](/operators.md#negate)
    - [**Restrict**](/operators.md#restrict)
  - [**Builders**](/builders.md)
- [**N-Queens Example**](/example.md)

# License
The software and documentation files in this repository are provided under the
[MIT License](https://github.com/SSoelvsten/coom/blob/master/LICENSE.md).

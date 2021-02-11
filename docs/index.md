_Adiar_ is a library for manipulating Decision Diagrams even when these grow
bigger than the memory one has available. To achieve this, all operations are
not implemented recursively with a global memoization table, but instead they
are created as streaming algorithms that exploit the sorting of the graph on
disk to delay recursion with Time-Forward Processing.

**Maintainer:** [Steffan Sølvsten](mailto:soelvsten@cs.au.dk)

# Table of Content

- [**Getting Started**](/getting_started.md)
  - [**Dependencies**](/getting_started.md#dependencies)
  - [**Building with CMake**](/getting_started.md#building-with-cmake)
  - [**Usage**](/getting_started.md#usage)

- [**BDD**](/bdd.md)
  - [**Basic Constructurs**](/bdd.md#basic-constructors)
  - [**Basic Manipulation**](/bdd.md#basic-manipulation)
  - [**Counting Operations**](/bdd.md#counting-operations)
  - [**Other Functions**](/bdd.md#other-functions)
  - [**DOT Output**](/bdd.md#dot-output)

- [**Core**](/core.md)
    - [**Data types**](/core.md#data-types)
        - [**Nodes and Pointers**](/core.md#nodes-and-pointers)
        - [**Assignments**](/core.md#assignments)
    - [**Files**](/core.md#files)
        - [**Nodes**](/core.md#nodes)
        - [**Assignments and Labels**](/core.md#assignments-and-labels)

- [**N-Queens Example**](/example.md)
  - [**Computing the set of all solutions**](/example.md#computing-the-set-of-all-solutions)
  - [**Printing each solution**](/example.md#printing-each-solution)

# License
The software and documentation files in this repository are provided under the
[MIT License](https://github.com/SSoelvsten/adiar/blob/master/LICENSE.md).

\page page__builder Manual Construction

In some cases, one may already know the shape of a `adiar::bdd` and/or as a
`adiar::zdd` for a more complex function. In those cases, it is much cheaper to
construct them by hand than to manipulate logic formulas

[TOC]

Builder
========================

The `adiar::builder` class allows one to construct decision diagrams. To this
end, they provide “pointers” to constructed nodes such that they can be
referenced by other nodes.

The templated argument carries with it the logic related to each type of
decision diagram, but we provide type definitions to hide it away

| Decision Diagram | Builder              | Pointer          |
|------------------|----------------------|------------------|
| `adiar::bdd`     | `adiar::bdd_builder` | `adiar::bdd_ptr` |
| `adiar::zdd`     | `adiar::zdd_builder` | `adiar::zdd_ptr` |

Example
========================

Consider the following BDD consisting of 3 nodes that represents the formula
\f$ (x_0 \land x_1) \lor x_2 \f$.

\dot
digraph builder_example {
  n0 [label=<x<sub>0</sub>>, shape=circle];
  n1 [label=<x<sub>1</sub>>, shape=circle];
  n2 [label=<x<sub>2</sub>>, shape=circle];

  t0 [label=0, shape=box];
  t1 [label=1, shape=box];

  n0 -> n2 [style=dashed]
  n0 -> n1 [style=solid]

  n1 -> n2 [style=dashed]
  n1 -> t1 [style=solid]

  n2 -> t0 [style=dashed]
  n2 -> t1 [style=solid]
  
  { rank=same; n0 }
  { rank=same; n1 }
  { rank=same; n2 }
  { rank=same; t0 t1 }
}
\enddot

This can be created manually using an `adiar::bdd_builder` (see `adiar::builder`
for all its member functions) as follows.

```cpp
bdd_builder b;

const bdd_ptr p2 = b.add_node(2, false, true);
const bdd_ptr p1 = b.add_node(1, p2, true);
const bdd_ptr p0 = b.add_node(0, p2, p1);

bdd example_a = b.build();
```

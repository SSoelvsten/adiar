# BDD

A Binary Decision Diagram (BDD) represents a boolean function

<p style="text-align: center;">
  {0,1}ⁿ → {0,1}
</p>

The `bdd` class takes care of reference counting and optimal garbage collection
of the underlying files (c.f. [Files](/core.md#files)). To ensure the most
disk-space is available, try to garbage collect the `bdd` objects as quickly as
possible and/or minimise the number of lvalues of said type.

**Table of Contents**

- [Basic Constructors](#basic-constructors)
- [Basic Manipulation](#basic-manipulation)
- [Counting Operations](#counting-operations)
- [Other Functions](#other-functions)
- [DOT Output](#dot-output)

## Basic Constructors

To construct a more complex but well-structured `bdd` than what follows below,
create a `node_file` and write the nodes bottom-up with a `node_writer` as
described in Section [Files](/core.md#files). The `bdd` object can then be
copy-constructed from the `node_file`.

- `bdd bdd_sink(bool)`

  Create a sink-only BDD with the provided boolean value. Alternatives are
  `bdd_true()` and `bdd_false()` or to assign a _bool_ directly to a BDD.
  
- `bdd bdd_ithvar(label_t)`

  Create a BDD representing the literal with the given _label_
  
- `bdd bdd_nithvar(label_t)`

  Create a BDD representing the negation of the literal with the given _label_
  
- `bdd bdd_and(const label_file)`

  Construct a BDD representing the _and_ of all the literals with the provided
  labels. The given `label_file` must be sorted in increasing order.

- `bdd bdd_or(const label_file)`

  Construct a BDD representing the _or_ of all the literals with the provided
  labels. The given `label_file` must be sorted in increasing order.

- `bdd bdd_counter(label_t min_label, label_t max_label, uint64_t threshold)`

  Construct a BDD that is true if exactly `threshold` many of the variables in
  the interval [`min_label`; `max_label`] are true.

## Basic Manipulation

- `bdd bdd_apply(bdd, bdd, bool_op)`

  Construct a bdd representing the `bool_op` applied onto the two given BDDs.
  For each operator, we provide the following alias functions.
  
  - `bdd bdd_and(bdd f, bdd g)` (operator `&`)
  
    Same as `bdd_apply(f, g, and_op)` and computes `f /\ g`.
  
  - `bdd bdd_nand(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, nand_op)` and computes `~(f /\ g)`.
  
  - `bdd bdd_or(bdd f, bdd g)` (operator `|`)

    Same as `bdd_apply(f, g, or_op)` and computes `f \/ g`.

  - `bdd bdd_nor(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, nor_op)` and computes `~(f \/ g)`.
  
  - `bdd bdd_xor(bdd f, bdd g)` (operator `^`)
  
    Same as `bdd_apply(f, g, xor_op)` and computes `f + g`.

  - `bdd bdd_xnor(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, xor_op)` and computes `~(f + g)`.
  
  - `bdd bdd_imp(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, imp_op)` and computes `f => g`.
  
  - `bdd bdd_invimp(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, invimp_op)` and computes `f <= g`.
  
  - `bdd bdd_equiv(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, equiv_op)` and computes `f = g` which is equivalent
    to `~(f + g)`.
    
  - `bdd bdd_diff(bdd f, bdd g)`
  
    Same as `bdd_apply(f, g, diff_op)` and computes `f /\ ~g`.
  
  - `bdd bdd_less(bdd f, bdd g)`
    
    Same as `bdd_apply(f, g, less_op)` and computes `~f /\ g`.

- `bdd bdd_ite(bdd f, bdd g, bdd h)`

  Return the BDD representing `f ? g : h`. In other BDD packages such a function
  is good for manually constructing a BDD bottom-up, but for those purposes one
  should here instead use the [`node_writer`](/core.md#files) class.

- `bdd bdd_not(bdd f)` (operator: `~`)

  Return the BDD representing `~f`.

- `bdd bdd_restrict(bdd f, assignment_file)`

  Return the BDD representing `f[xₛ / v, ..., xₜ / v]` for `{s ; vₛ}`, ...,
  `{t ; vₜ}` in `assignment_file`. The `assignment_file` must be in
  increasing order wrt. the label.

- Variable quantification

  - `bdd bdd_exists(bdd, label_t)`

     Existential quantification (or) of the variable with the given label.
     
  - `bdd bdd_exists(bdd, label_file)`
  
     Existential quantification (or) of all variables in the `label_file` in the
     very order these variables are provided.

  - `bdd bdd_forall(bdd, label_t)`

     Forall quantification (and) of the variable with the given label.
     
  - `bdd bdd_forall(bdd, label_file)`
  
     Forall quantification (and) of all variables in the `label_file` in the
     very order these variables are provided.


## Counting Operations

- `uint64_t bdd_nodecount(bdd)`

   The number of nodes (not counting sink nodes) in the BDD.

- `uint64_t bdd_varcount(bdd)`

  The number of variables present in the BDD.

- `uint64_t bdd_pathcount(bdd)`

  Count all unique (but not necessarily disjoint) paths that satisfies the
  given sink predicate (default is only to count _true_ sinks).

- `uint64_t bdd_satcount(bdd f, size_t varcount)`

  Count the number of satisfying assignments (i.e. the number of `x` such that
  `f(x) = 1`), given the total number of expected variables. The default value
  for `varcount` is to be the number of variables occuring in the given BDD,
  i.e. `bdd_varcount(f)`.

- `uint64_t bdd_satcount(bdd, min_label, max_label)`

  Count the number of satisfying assignments to variables in the interval
  [`min_label`; `max_label`].

## Other Functions

- `bool bdd_eval(bdd f, assignment_file x)`

  Return `f(x)`, i.e. the evaluation of the given BDD for `f` according to the
  assignment `x`.

- `assignment_file bdd_satmin(bdd f)`

  Return the _lexicographically smallest_ `x` such that `f(x) = 1`. The
  variables mentioned in `x` are for all levels in the given BDD.

- `assignment_file bdd_satmax(bdd f)`

  Return the _lexicographically largegst_ `x` such that `f(x) = 1`. The
  variables mentioned in `x` are for all levels in the given BDD.

- `bool is_sink(bdd f, sink_pred)`

  Whether the BDD for `f` only consists of a sink satisfying the given sink
  predicate, i.e. `f` is a constant function. By default the predicate for _any_
  kind of sink is used.

- `label_t min_label(bdd)`

  Return the smallest _label_ in the BDD, i.e. the label of the root.

- `label_t max_label(bdd)`

  Return the largest _label_ in the BDD, i.e. the label of the deepest node.

## DOT Output

- `void output_dot(bdd, std::string filename)`

  Prints the bdd to a dot file with the filename provided.

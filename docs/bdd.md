# BDD

The `bdd` class takes care of reference counting and optimal garbage collection
of the underlying files (c.f. [Files](/core.md#files)). To ensure the most
disk-space is available, try to garbage collect the `bdd` objects as quickly as
possible and/or minimise the number of lvalues with said type.

**Table of Contents**

- [Basic Constructors](#basic-constructors)
- [Basic Manipulation](#basic-manipulation)
- [Other functiosn](#other-functiosn)

## Basic Constructors

- `bdd bdd_sink(bool)`

  Create a sink-only BDD with the provided boolean value. Alternatives are
  `bdd_true()` and `bdd_false()`.
  
- `bdd_ithvar(label_t)`

  Create a BDD representing the literal with the given _label_
  
- `bdd_nithvar(label_t)`

  Creates a BDD representing the negation of the literal with the given _label_
  
- `bdd_and(const label_file)`

  Construct a BDD representing the _and_ of all the literals with the provided
  labels. The given `label_file` must be sorted in increasing order.

- `bdd_or(const label_file)`

  Construct a BDD representing the _or_ of all the literals with the provided
  labels. The given `label_file` must be sorted in increasing order.

- `bdd_counter(label_t min_label, label_t max_label, uint64_t threshold)`
  Construct a 

## Basic Manipulation

- `bdd bdd_apply(bdd, bdd, bool_op )`

  Constructs a bdd representing the `bool_op` applied onto the two given _BDD_s.
  For each operator, we provide a function as a shortcut.
  
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

- `bdd bdd_not(bdd f)` (operator: `~`)

  Returns the BDD representing `~f`.

- `bdd bdd_restrict(bdd, assignment_file)`

  Computes the BDD representing `bdd` with the nodes with a _label_ within
  `assignment_file` restricted to be the associated _value_ in the same file.
  The `assignment_file` must be in sorted order by the label.

- Variable quantification

  - `bdd bdd_exists(bdd, label_t)`

     Existential quantification (or) of the variable with the given label.
     
  - `bdd bdd_exists(bdd, label_file)`
  
     Existentially quantification (or) all variables in `label_file` in the very
     order these variables are provided.

  - `bdd bdd_forall(bdd, label_t)`

     Forall quantification (and) of the variable with the given lable.
     
  - `bdd bdd_forall(bdd, label_file)`
  
     Forall quantification (and) all variables in `label_file` in the very order
     these variables are provided.
     
  - `bdd bdd_unique(bdd, label_t)`

     Unique quantification (xor) of the variable with the given lable.
     
  - `bdd bdd_unique(bdd, label_file)`
  
     Unique quantification (xor) all variables in `label_file` in the very order
     these variables are provided.

## Other functiosn

- `is_sink(bdd)`

  Whether the sink only consists of a sink.

- `min_label(bdd)`

- `max_label(bdd)`

- `bool bdd_eval(bdd, assignment_file)`

  Evaluate the BDD corresponding to the assignment provided. Returns the value
  of the sink reached.

- `uint64_t bdd_nodecount(bdd)`

  The number of nodes (not counting sink nodes) in the BDD.

- `uint64_t bdd_varcount(bdd)`

  The number of variables present in the BDD.

- `uint64_t bdd_pathcount(bdd, sink_pred)`

  Count all unique (but not necessarily disjoint) paths that satisfies the given
  sink predicate (default is only to count _true_ sinks).

- `uint64_t bdd_satcount(bdd, label_t min_label, label_t max_label, sink_pred)`

  Count all assignments to variables in the interval `[min_label, max_label]`
  that satisfies the given sink predicate (default is only to count _true_
  sinks)

- `uint64_t bdd_satcount(bdd, sink_pred)`

  Same as the `bdd_satcount` above with `min_label`, resp. `max_label`, set to
  `min_label(bdd)`, resp. `max_label(bdd)`.

- `uint64_t bdd_satcount(bdd, size_t varcount, sink_pred)`

  Count all assignments to variables, given the total number of expected
  variables.

## DOT Output

- `void output_dot(bdd, std::string filename)`

  Prints the bdd to a dot file with the filename provided.

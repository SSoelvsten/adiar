# v1.0.1

**Date: 6th of September, 2021**

## Bug fixes

- `bdd_apply`:
  - Fix tuples have their ties on `std::min` or `std::max` not broken correctly. This resulted in that the same recursion request could potentially be handled multiple times independently, since all its "calls" ended up interleaving with other tied requests in the priority queues.
  - Slightly improve performance of some boolean operators. Most likely this is negligible.
- `bdd_counter`:
  - Fix returns trivially false cases as *true*. 
- *CMake* 
  - Now Adiar compiles with C++17 regardless of its parent project. This allows the user to omit the use of `set_target_properties(<target> PROPERTIES CXX_STANDARD 17)` in their own CMake settings.

## Changes

- `adiar_init`:
  - It now takes its memory argument in *bytes* rather than in *MiB*.
- *Equality Checking* (`==`): Improved performance from its prior *O(N<sup>2</sup> log N<sup>2</sup>)* time comparison.
  - If both given BDDs are *canonical* (as defined in the [documentation](https://ssoelvsten.github.io/adiar/core/files#a-note-on-equality-checking)) and have the same value in their *negation flag*, then equality checking is done with a simple (and much faster) linear scan.
  - In all other cases the prior time-forward processing algorithm is used. But this one has been improved to be an *O(N log N)* time comparison algorithm. That is, equality checking is not only a constant improvement computing `~(f ^ g) == bdd_true()` but it is provably faster (both in terms of time and I/Os).

# v1.0.0

**Date: 25th of April, 2021**

## Binary Decision Diagrams

`bdd` class to hide away management of files and running the _reduce_ algorithm. This takes care of reference counting and optimal garbage collection.

### Constructors
- `bdd_sink(bool v)` (and `bdd_true()` and `bdd_false()` as alternatives)
- `bdd_ithvar(label_t i)` and `bdd_nithvar(label_t i)`
- `bdd_and(label_file ls)` and `bdd_or(label_file ls)` to construct an *and*/*or* chain.
- `bdd_counter(label_t min_label, label_t max_label, label_t threshold)` to construct whether exactly *threshold* many variables in the given interval are true.

Furthermore, the `node_writer` class is also provided as a means to construct a BDD manually bottom-up.

### Basic Manipulation
- `bdd_apply(bdd f, bdd g, bool_op op)` to combine two BDDs with a binary operator (also includes aliases for every possible *op*)
- `bdd_ite(bdd f, bdd g, bdd h)` to compute the if-then-else
- `bdd_not(bdd f)` to negate a bdd
- `bdd_restrict(bdd f, assignment_file as)` to fix the value of one or more variables
- `bdd_exists(bdd f, label_t i)` and `bdd_forall(bdd f, label_t i)` to existentially or forall quantify a single variable (also includes versions with the second argument being multiple labels in a `label_file`).

### Counting Operations
- `bdd_nodecount(bdd f)` the number of (non-leaf) nodes.
- `bdd_varcount(bdd f)` the number of levels present (i.e. variables).
- `bdd_pathcount(bdd f)` the number of unique paths to the *true* sink.
- `bdd_satcount(bdd f, size_t varcount)` the number of satisfying assignments.

### Input variables
- `bdd_eval(bdd f, assignment_file x)` computes *f(x)*.
- `bdd_satmin(bdd f)`, resp. `bdd_satmax(bdd f)`, to find the lexicographical smallest, resp. largest, satisfying assignment.

### Other Functions
- `output_dot(bdd f, std::string filename)` to output a visualizable *.dot* file.

## License
Adiar 1.0.0 is distributed under the MIT license. But, you should notice that it uses the TPIE library underneath, which is licensed under LGPL v3, and so by extension any binary file of Adiar is covered by the very same license.

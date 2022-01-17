# v1.1.0

**Date: 25th of January, 2022**

## Zero-suppressed Decision Diagrams

Adds support for *zero-suppressed* decision diagrams with the `zdd` class. All operations on *ZDD*s are based on the family of sets semantics as in the original paper by Minato.

### Constructors
- `zdd zdd_sink(bool v)` (and `zdd_empty()` and `zdd_null()` as alternatives)
- `zdd zdd_ithvar(label_t i)`
- `zdd zdd_vars(label_file vars)`,  `zdd zdd_singletons(label_file vars)`, and `zdd zdd_powerset(label_file vars)` to respectively construct an *and*, *or*, and the *don't care* chain.
- `zdd zdd_sized_set<pred_t>(label_file vars, k, pred)` to construct the sets of variables in *vars* whose size satisfies the given predicate in relation to *k*.

### Basic Manipulation
- `zdd zdd_binop(zdd A, zdd B, bool_op op)` to apply a binary operator to two families of sets (also includes aliases for the _or_, _and_, and _diff_ operators).
- `zdd zdd_change(zdd A, label_file vars)` to compute the symmetric difference.
- `zdd zdd_complement(zdd A, label_file dom)` to construct the complement.
- `zdd zdd_expand(zdd A, label_file vars)` to expand the domain with new variables.
- `zdd zdd_offset(zdd A, label_file vars)` to compute the subset without the given variables.
- `zdd zdd_onset(zdd A, label_file vars)`  to compute the subset with the given variables.
- `zdd zdd_project(zdd A, label_file is)` to project onto a (smaller) domain.

### Counting Operations
- `uint64_t zdd_nodecount(zdd A)` the number of (non-leaf) nodes.
- `uint64_t zdd_varcount(zdd A)` the number of levels present (i.e. variables).
- `uint64_t bdd_size(bdd A)` the number of elements in the family of sets.

### Predicates
- `bool zdd_equal(zdd A, zdd B)` to check for set equality.
- `bool zdd_unequal(zdd A, zdd B)` to check set inequality.
- `bool zdd_subseteq(zdd A, zdd B)` to check for weak subset inclusion.
- `bool zdd_subset(zdd A, zdd B)` to check for strict subset inclusion.
- `bool zdd_disjoint(zdd A, zdd B)` to check for the sets being disjoint.

### Set Elements
- `bool zdd_contains(zdd A, label_file a)`
- `std::optional<label_file> zdd_minelem(zdd A)`
- `std::optional<label_file> zdd_maxelem(zdd A)`

### Other Functions
- `output_dot(bdd f, std::string filename)` to output a visualizable _*.dot* file.
- `zdd zdd_from(bdd f, label_file dom)` and `bdd bdd_from(zdd f, label_file dom)` to convert between *BDD*s and *ZDD*s interpreted in the given domain.

## Statistics

Compile Adiar with `ADIAR_STATS` or `ADIAR_STATS_EXTRA` to gather statistics about the execution of the internal algorithms. With `ADIAR_STATS` you only gathers statistics that introduce a small constant time overhead to every operation. `ADIAR_STATS_EXTRA` also gathers much more detailed information, such as the bucket-hits of the levelized priority queue, which does introduce a linear-time overhead to every operation.

- `stats_t adiar_stats()` to obtain a copy of the raw data values.
- `void adiar_printstat(std::ostream)` to print all statistics to an output stream.

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

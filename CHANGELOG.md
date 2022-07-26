# v1.2.0

**Date: 26th of July, 2022**

## Performance

This new release's primary focus is to drastically improve performance for smaller instances.To this end, we still use the very same algorithms, but if the input is small enough then we soundly can use purely internal memory auxiliary data structures within each algorithm. Prior to this version we could only guarantee up to a 3x performance difference compared to other BDD libraries when the largest constructed decision diagram is 9.5 GiB or larger. Now, we are able to now guarantee the same when the largest is only a few hundred KiB or larger.

## Domain

One may now set a *label_file* as the global domain over which one works.

- `adiar_set_domain(label_file dom)`
  sets the global domain variable.
- `adiar_has_domain()`
  checks whether a global domain already is set.
- `adiar_get_domain()`
  provides the current *label_file* that acts as the global domain (assuming `adiar_has_domain()` evaluates to `true`).

## Binary Decision Diagrams

### New Features

- `bdd_builder`
  is a new class to replace using the *node_writer* directly. This allows you to much more naturally construct BDDs bottom-up by hiding away several details. Furthermore, it makes use of exceptions rather than immediately terminating assertions.
- `bdd_from(zdd A)`
  converts from a ZDD to a BDD using the global domain.
- `bdd_equal(bdd f, bdd g)`
  is an alternative to using the `==` operator.
- `bdd_unequal(bdd f, bdd g)`
  is an alternative to using the `!=` operator.
- `bdd_varprofile(bdd f)`
  obtains a *label_file* containing all of the variables present in a BDD.

### Bug Fixes

- Results from `bdd_nithvar(label_t i)` and `bdd_ithvar(label_t i)`
  are now marked as *canonical* and so can be used with the linear-scan equality checking.
- Fixed the reduction phase may use 2 MiB more memory than is available.

## Zero-suppressed Decision Diagrams

### New Features

- `zdd_builder`
  is a new class to replace using the *node_writer* directly. This allows you to much more naturally construct ZDDs bottom-up by hiding away several details. Furthermore, it makes use of exceptions rather than immediately terminating assertions.
- `zdd_complement(zdd A)`
  Complementation within the global domain.
- `zdd_from(bdd f)`
  Converts from a BDD to a ZDD using the global domain.
- `zdd_varprofile(zdd A)`
  Obtain a *label_file* containing all of the variables present in a ZDD.

### Bug Fixes

- `zdd_ithvar(label_t i)`
  Is now marked as *canonical* and so can be used with the linear-scan equality checking.
- Fixed the reduction phase may use 2 MiB more memory than is available.
- DOT files are now with the terminals properly printed as *Ø* and *{Ø}*.

## Statistics

### New Features

- Statistics have been heavily extended with information on how often each type of auxiliary data structures (internal or external) have been used.
- All statistics variables are now fixed-precision numbers (using the [CNL library](https://github.com/johnmcfarlane/cnl)) making sure there are no overflows in the provided numbers.
- `adiar_statsreset()`
  resets all statistics values back to 0.

### Bug Fixes

- Fixed fine grained statistics (*ADIAR_STATS_EXTRA*) are turned on if only coarse-grained statistics (*ADIAR_STATS*) was desired.

## Deprecations

The word *sink* has been replaced with the word *terminal* that is more commonly used in the context of decision diagrams.

- *adiar/data.h*
  - `create_sink_uid(bool val)` -> `create_terminal_uid(bool val)`
  - `create_sink_ptr(bool val)` -> `create_terminal_ptr(bool val)`
  - `create_sink(bool val)` -> `create_terminal(bool val)`
- *adiar/bdd.h*
  - `is_sink(bdd f)` -> `is_terminal(bdd f)`
  - `bdd_sink(bool val)` -> `bdd_terminal(bool val)`
- *adiar/zdd.h*
  - `is_sink(zdd A)` -> `is_terminal(zdd A)`
  - `zdd_sink(bool val)` -> `zdd_terminal(bool val)`

## Breaking Changes

The *terminal predicates* `is_any`, `is_true` and `is_false` with the prior `is_sink(zdd A, pred)` functions were too complicated. The above performance improvements allows us for a much simpler (and faster) implementation. Deprecation was not possible due to name conflicts with their replacements below.

- *adiar/data.h*
  - `is_sink(ptr_t p)`, `is_true(ptr_t p)`, and `is_false(ptr_t p)`.
- *adiar/bdd.h*
  - `is_sink(bdd f)`, `is_true(bdd f)` and `is_false(bdd f)`.
- *adiar/zdd.h*
  - `is_sink(zdd A)`, `is_null(zdd A)` and `is_empty(zdd A)`.

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

# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate research projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Future Work](#future-work)
    - [Implementation of missing BDD algorithms](#implementation-of-missing-bdd-algorithms)
        - [Data type conversions](#data-type-conversions)
        - [Projection](#projection)
        - [Set manipulation](#set-manipulation)
        - [Composition](#composition)
        - [Advanced satisfiability functions](#advanced-satisfiability-functions)
        - [Coudert's and Madre's BDD functions](#couderts-and-madres-bdd-functions)
    - [Optimising the BDD](#optimising-the-bdd)
        - [Complement Edges](#complement-edges)
    - [Extensions](#extensions)
        - [Proof Logging](#proof-logging)
        - [Multi-Terminal Binary Decision Diagrams](#multi-terminal-binary-decision-diagrams)
        - [Zero-suppressed Decision Diagrams](#zero-suppressed-decision-diagrams)
        - [Multi-valued Decision Diagrams](#multi-valued-decision-diagrams)
        - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)
    - [Optimising the current algorithms](#optimising-the-current-algorithms)
        - [Levelized Files](#levelized-files)
        - [Non-comparison based sorting on numbers](#non-comparison-based-sorting-on-numbers)
        - [From _recursive_ algorithm to _time-forward processing_ and back again](#from-recursive-algorithm-to-time-forward-processing-and-back-again)
    - [References](#references)

<!-- markdown-toc end -->


## Implementation of missing BDD algorithms
There are still many features for BDDs not yet addressed in the _Adiar_ library,
and valuable additions to the current project can be made in implementing these.
All of these of course should be made in the style of _Time-Forward Processing_
like the rest.

The features are sorted based on the difficulty deriving their design and their
implementation. The first few already include a description and good ideas of
how to approach the implementation.

### Data type conversions

If one can convert to the `bdd` data type from an `assignment_file` and a
`label_file`, then the output of `bdd_satmin`, `bdd_satmax`, etc. can be used
elsewhere. Likewise, if we can convert a `bdd` back into an `assignment_file`
or `label_file` then we can use `bdd`'s as input in `bdd_restrict`, `bdd_exists`,
and `bdd_forall`. That would help bridge the gap between this and other packages.

Similarly, the `bdd_and` and `bdd_or` functions in `bdd/build.cpp` should maybe
be extended to also take an array, `std::vector`, or even make them variadic.

The simple way to do this would be by _O(N)_ time and _(N/B)_ I/O algorithms that
use _O(N)_ more space on disk. I would expect though, that with some templating
it should be possible to reuse the original files and convert the types on-the-fly.

### Projection

`bdd_project` is the dual to `bdd_exists`, i.e. only keep the variables in the
given cube and existentially quantify all other variables. The simplest way to
do so may be to filter out levels in the _meta_ file that are not in the given
_label_ file and then just call `bdd_exists` on that. Alternatively, one can
extend the `bdd_quantify` algorithm to flip the check on a label being within
the _label_ file.

### Set manipulation

Verification relies heavily on manipulation of sets. For this, we need multiple
functions, most of which are aliases for other algorithms. See the documentation
of Sylvan for an overview of this.

Furthermore, one may want to convert back and from `std::set<label_t>` and
similar data structures.

Finally, one may want to look into Coudert and Madre's _Meta-products_
representation [[Coudert92](#references)] of sets in BDDs and the functions
related to it.

### Composition
The _Composition_ (`bdd_compose`) of two OBDDs _f_ and _g_ for some label
_i ∊ [n]_ is _f ∘<sub>i</sub> g (x)_ and is to be interpreted as
_f(x<sub>1</sub>, ..., x<sub>i-1</sub>, g(x<sub>1</sub>, ..., x<sub>n</sub>), x<sub>i+1</sub>, ..., x<sub>n</sub>)_.

This can be implemented with a single sweep through _f_ and _g_ by using the
ideas in the _quantification_ and the _if-then-else_ algorithms. A priority
queue contains requests on triples `t1`, `t2`, `t3` where `t2` and `t3` are
nodes from _f_ and `t1` is from _g_. Most optimisations and prunings used for
both the _if-then-else_ and the _quantification_ algorithm then would apply
here.

### Advanced satisfiability functions
The number of satisfiable assignments can be very large (even larger than
2<sup>64</sup> at times). Hence, the BDD package BuDDy also provides a
`bdd_satcountln` function, that outputs the logarithm of the number of
satisfiable assignments. The current `count` function should be changed to
also support this. To this end, the use of template functions for a
compile-time strategy should be replaced with lambda functions instead (see
`bdd_satmin` and `bdd_satmax` functions for inspiration).

Furthermore, currently we only provide functions to obtain the lexicographically
smallest or largest assignment. A `bdd_satall` function with a callback to
iterate over all assignments would be needed. One could go for outputting
assignments in lexicographical order, but that may cost extra in I/Os. One
may instead go for minimizing the distance travelled when picking a child.

### Coudert's and Madre's BDD functions
In [[Coudert90](#references)] was proposed three functions, that are of interest
for the field of verification. Inspired by the naming in BuDDy the three functions
are:

- `bdd_simplify`: Their _Restrict_ function
- `bdd_constrain`: Their _Constrain_ function
- `bdd_expand`: Their _Expand_ function


## Optimising the BDD

### Complement Edges
Currently, we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique. In the
recursive algorithms, one can even expect a factor two decrease in the
algorithms execution time [[Brace90](#references)].


## Extensions

### Proof Logging
A problem in SAT solving is trusting the solver when it claims a formula to be
_unsatisfiable_. To this end, the solver can create a _extended resolution
proof_, though doing so is not a trivial task. Yet, the nature of the BDD
operations in fact naturally give rise to these very proofs, as the algorithms
in [[Bryant21](#references)] do.

The underlying idea in [[Bryant21](#references)] is to store within each node
the associated extension variable _u_ and the four proof steps that encode the
formula '_u_ <-> ITE(_x_,_u_<sub>1</sub>,_u_<sub>0</sub>)'.

- To integrate these into _Adiar_ it seems best to place them in a separate file
  that maps each node to these five numbers. Most importantly, this allows one
  to turn the feature off to safe on memory and computation time.

The algorithms are then changed as follows

- During the Apply of BDDs for two functions _f_ and _g_, all nodes in the
  intermediate output can remember what pair of variables (_u_,_v_) is
  associated with them.

- The Reduce then puts the output of Apply back in canonical form. Here, we
  would try to collapse the (_u_,_v_) tuples back into a single variable.
  
  - From the output of Apply we can retrieve the original (_u_,_v_) and their
    defining proof steps.
  
  - Within the priority queue the variable _w_<sub>i</sub> for each child is
    forwarded together with its defining clauses OPL, OPH (ANDL and ANDH in
    [[Bryant21](#references)], but we can generalise it to any binary operator).
    This is merely three more integers.
  
  - We can use the idea of _taint tracking_ to merge the (_u_,_v_) variables.
    From bottom-up we remember whether the node was like ...
    
    1. ... in neither f or g
    2. ... in f
    3. ... in g
    4. ... in both f and g.
    
    These taints can be created in the Apply. Then, it is propagated upwards and
    updated for each node based on the given taint of its resolved children.
    When these cases are labelled in binary starting from 0 then the resulting
    case is derived merely with the & bit-operator. When the taint of a node is
    resolved, then we should be able to output the new clauses to the Proof
    Checker, if any. In the case of _d._ with two different extension variables
    we would need to connect them.

  - An open question is still how to notice whether a variable goes out of
    scope. Likely, this is possible within the Reduce algorithm after having
    sorted all remaining nodes by their children.

Since most of the work is placed within the Reduce algorithm, then almost all
other algorithms (such as quantification) can also immediately create the
relevant proof.


### Multi-Terminal Binary Decision Diagrams
One can easily extend the proposed representation of sink nodes to encompass
non-boolean values, such as integers or floats. Thereby, the algorithms
immediately yield an I/O efficient implementation of the _Multi-Terminal Binary
Decision Diagrams_ (MTBDD) of [[Fujita97](#references)].

### Zero-suppressed Decision Diagrams
A Zero-suppressed Decision Diagram [[Minato93,Minato01](#references)] is a binary
decision diagram, which is very compresed when representing sparse sets of bit
vectors. This has been shown to be great for solving NP-Complete problems and
symbolic model checking algorithms on sparse sets of states.

To achieve this, ZDDs make use of a different reduction rule than BDDs to do so.
So, to implement them we need to:

- Generalize the _Reduce_ with a strategy pattern with lambdas.

- Repurpose/Generalize current algorithms wherever possible
 
  - `zdd_empty`: Same as `bdd_false`

  - `zdd_null`/`zdd_base`: Same as `bdd_true`

  - `zdd_ithvar`: Same as `bdd_ithvar`

  - `zdd_union`, `zdd_intsec`, `zdd_diff`: A generalized `bdd_apply` algorithm with
    _or_, _and_ and _diff_ as the operators. The recursion request generation for ZDDs
    need to take into account the operator and the reduction rule to be optimal. So, a
    it is needed to add a strategy pattern in that spot (and to make Apply work with
    `nil` inside of the request tuples?).

  - `zdd_onset`/`zdd_subset1` and `zdd_offset`/`zdd_subset0`: Can be done with `bdd_restrict`

  - `zdd_count`: Same as `bdd_pathcount`
  
- Implement other 'new' and more complex algorithms.

  - `zdd_change`: Should be possible to do in a single bottom-up sweep, though one
    needs to remove/add nodes based on the reduction rules above the given label.
  
  - Unate Cube operations `zdd_prod`, `zdd_div`, `zdd_mod` [[Minato01](#references)]:
    These seem to again do a double-recursion similar to `bdd_exists` and `bdd_forall`.
    So, we can probably reuse the same technique to make this more efficient.

- Finally, the functions for reasoning about state-transition systems into ZDDs, such
  as `bdd_ite`, `bdd_exists` and `bdd_relprod` needs to be translated. See
  [[Hajighasemi14](#references)] for a recursive version of these.


### Multi-valued Decision Diagrams
By solely using an edge-based representation of the data-structure one can also
implement a _Multi-valued Decision Diagram_ (MDD) of
[[Kam98](#references)]. This allows one to succinctly encode a function
from a non-boolean domain. Switching to the edge-based representation will lead
to rewriting almost all algorithms, but we may look into recreating the _List
Decision Diagrams_ of [[Dijk16](#references)] to circumvent this.

### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Meinel94](#references)] may also be possible to
implement in the setting of Time-forward processing used here.


## Optimising the current algorithms
There are quite a few avenues of trying to shave off a few significant constants
in the running time on the current algorithms. Some suggestions below also make
the GPU an intriguing subject for a possible heavy improvement in the running
time.

### Levelized Files
The node-based and arc-based representations currently use a single file for the
entire data structure. If this file is split up per level, then one can
aggressively garbage collect each level while the algorithms are running. This
can safe concurrent disk usage and so allow computation on even bigger instances
before running out of disk space.

### Non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe one will not gain too much due to the _O(sort(N))_ I/O lower bound.

### From _recursive_ algorithm to _time-forward processing_ and back again
Most implementations, such as the ones in [[Brace90, Dijk16](#references)], make
use of a _unique node table_, which is a hash-table in which all BDDs exist.
This has the benefit of allowing one to reuse common subtrees across BDDs (which
saves space linear in the number of concurrent BDDs in use) and the recursive
algorithms run _2_ or even _4_ times faster than the current algorithms (when
they don't outgrow the main memory).

_TPIE_ provides a hash table, so one can look into one of the following two

1. When a BDD outgrows the main memory, we may be able to convert to the current
   time-forward processing algorithms instead of crashing (which the other BDD
   libraries do). When a stream-based BDD grows small enough again, then we may
   be able to place it back in the _unique node table_.

2. If one can keep using the same node identifiers in the table, then one may
   also use both data structures at the same time. Some parts of the BDD is in
   main memory, while others are in the stream.

Based on the memory usage I've witnessed during benchmarking, I think the first
option is the most promising.

See also the discussion in issue [#98](https://github.com/SSoelvsten/adiar/issues/98)

## References

- [[Brace90](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=114826)]
  Karl S. Brace, Richard L. Rudell, and Randal E. Bryant. “_Efficient
  implementation of a BDD package_”. In: _27th ACM/IEEE Design Automation
  Conference_. pp. 40 – 45 (1990)

- [[Bryant21](https://arxiv.org/abs/2105.00885)]
  Randal E. Bryant, Marijn J. H. Heule. “_Generating Extended Resolution Proofs
  with a BDD-Based SAT Solver (Extended Paper)_”. In: _arXiv_. (2021)

- [[Coudert90](http://www.ocoudert.com/papers/pdf/iccad90.pdf)]
  Olivier Coudert and Jean Christophe Madre. “_A Unified Framework for the
  Formal verification of sequential circuits_”. In: _Computer-Aided Design /
  IEEE International Conference_. (1990)

- [[Coudert92](https://www.researchgate.net/profile/Olivier-Coudert/publication/221059871_Implicit_and_Incremental_Computation_of_Primes_and_Essential_Primes_of_Boolean_Functions/links/556f37fd08aeccd777410eec/Implicit-and-Incremental-Computation-of-Primes-and-Essential-Primes-of-Boolean-Functions.pdf)]
  Olivier Coudert and Jean Christophe Madre. “_Implicit and Incremental Computation
  of Primes and Essential Implicant Primes of Boolean Functions_”. In: _Proc.
  ACM/IEEE 29th DAC_. (1992)

- [[Dijk16](https://link.springer.com/content/pdf/10.1007/s10009-016-0433-2.pdf)]
  Tom van Dijk, Jaco van de Pol. “_Sylvan: multi-core framework for decision
  diagrams_”. In: _International Journal on Software Tools for Technology
  Transfer_. (2016)

- [[Fujita97](https://link.springer.com/article/10.1023/A:1008647823331#citeas)]
  M. Fujita, P.C. McGeer, J.C.-Y. Yang . “_Multi-Terminal Binary Decision
  Diagrams: An Efficient Data Structure for Matrix Representation_”. In: _Formal
  Methods in System Design_. (2012)

- [[Hajighasemi14](https://essay.utwente.nl/66388/)]
  Maryam Hajighasemi. “_Symbolic Model Checking using Zero-suppressed Decision
  Diagrams_”. Master's Thesis, University of Twente (2014)

- [Kam98]
  Timothy Kam, Tiziano Villa, Robert K. Brayton, and L. Sangiovanni-vincentelli
  Alberto. “_Multi-valued decision diagrams: Theory and applications_”. In:
  _Multiple- Valued Logic 4.1_ (1998)

- [Meinel94]
  J. Gergov and C. Meinel. “_Efficient analysis and manipulation of OBDDs can
  be extended to FBDDs_”. (1994)

- [[Minato93](https://dl.acm.org/doi/pdf/10.1145/157485.164890)]
  S. Minato. “_Zero-suppressed BDDs for set manipulation in combinatorial
  problems_”. In: _DAC '93: Proceedings of the 30th international Design
  Automation Conference_ (1993)

- [[Minato01](https://eprints.lib.hokudai.ac.jp/dspace/bitstream/2115/16895/1/IJSTTT3-2.pdf)]
  S. Minato. “_Zero-suppressed BDDs and their applications_”. In: _International
  Journal on Software Tools for Technology Transfer, 3_ (2001)

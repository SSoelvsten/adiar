# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate research projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Future Work](#future-work)
    - [Implementation of missing BDD algorithms](#implementation-of-missing-bdd-algorithms)
        - [Projection](#projection)
        - [Composition](#composition)
        - [Coudert's and Madre's Restrict](#couderts-and-madres-restrict)
        - [Variable reordering](#variable-reordering)
    - [Optimising the BDD](#optimising-the-bdd)
        - [Complement Edges](#complement-edges)
    - [Extensions](#extensions)
        - [Multi-Terminal Binary Decision Diagrams](#multi-terminal-binary-decision-diagrams)
        - [Zero-suppressed Decision Diagrams](#zero-suppressed-decision-diagrams)
        - [Multi-valued Decision Diagrams](#multi-valued-decision-diagrams)
        - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)
    - [Optimising the current algorithms](#optimising-the-current-algorithms)
        - [Non-comparison based sorting on numbers](#non-comparison-based-sorting-on-numbers)
        - [From _recursive_ algorithm to _time-forward processing_ and back again](#from-recursive-algorithm-to-time-forward-processing-and-back-again)

<!-- markdown-toc end -->


## Implementation of missing BDD algorithms
There are still many features for BDDs not yet addressed in the _Adiar_ library,
and valuable additions to the current project can be made in implementing these.
All of these of course should be made in the style of _Time-Forward Processing_
like the rest.

The features are sorted based on the difficulty deriving their design and their
implementation. The first three already include a description and good ideas of
how to approach the implementation.

### Projection

This is the dual to `bdd_exists`, i.e. only keep the variables in the given cube
and existentially quantify all other variables. The simplest way to do so may be
to filter out levels in the _meta_ file that are not in the given _label_ file
and then just call `bdd_exists` on that.

### Composition
The _Composition_ of two OBDDs _f_ and _g_ for some label _i ∊ [n]_ is
_f ∘<sub>i</sub> g (x)_ and is to be interpreted as _f(x<sub>1</sub>, ...,
x<sub>i-1</sub>, g(x<sub>1</sub>, ..., x<sub>n</sub>), x<sub>i+1</sub>, ...,
x<sub>n</sub>)_.

This can be implemented with a single sweep through _f_ and _g_ by using the
ideas in the _quantification_ and the _if-then-else_ algorithms. A priority
queue contains requests on triples `t1`, `t2`, `t3` where `t2` and `t3` are
nodes from _f_ and `t1` is from _g_. Most optimisations and prunings used for
both the _if-then-else_ and the _quantification_ algorithm then would apply
here.

### Coudert's and Madre's Restrict
The current _Restrict_ algorithm is the basic algorithm of Bryant, but one has
been proposed in [[Coudert90](README.md#references)] that is very different and
may be used in Verification. They also proposed algorithms with the name
_Constrain_ and _Expand_.

### Variable reordering
Currently, _Adiar_ only uses a static ordering of the variables, but since the
size of the BDD is heavily influenced by the order chosen then many BDD
libraries provide variable reordering algorithms, or do these themselves
behdind the scenes.

How one can rephrase these algorithms within the design of _Adiar_ will probably
require quite a bit of creativity, understanding of the I/O model and also of
the original variable reordering algorithms.


## Optimising the BDD

### Complement Edges
Currently, we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique. In the
recursive algorithms, one can even expect a factor two decrease in the
algorithms execution time [[Brace90](README.md#references)].


## Extensions

### Multi-Terminal Binary Decision Diagrams
One can easily extend the proposed representation of sink nodes to encompass
non-boolean values, such as integers or floats. Thereby, the algorithms
immediately yield an I/O efficient implementation of the _Multi-Terminal Binary
Decision Diagrams_ (MTBDD) of [[Fujita97](README.md#references)].

### Zero-suppressed Decision Diagrams
A Zero-suppressed Decision Diagram
([ZDD](https://en.wikipedia.org/wiki/Zero-suppressed_decision_diagram)) is a
binary decision diagram, which is very compresed when representing sparse sets
of bit vectors. This has been shown to be great for solving NP-Complete problems
and symbolic model checking algorithms on sparse sets of states.

To achieve this, ZDDs make use of a different reduction rule than BDDs to do so.
So, to implement them we need to:

- Generalize the _Reduce_ and _Apply_ algorithms (strategy pattern with lambdas?)
  to implement all basic operations.
- Implement other 'new' and more complex algorithms.

### Multi-valued Decision Diagrams
By solely using an edge-based representation of the data-structure one can also
implement a _Multi-valued Decision Diagram_ (MDD) of
[[Kam98](README.md#references)]. This allows one to succinctly encode a function
from a non-boolean domain. Switching to the edge-based representation will lead
to rewriting almost all algorithms, but we may look into recreating the _List
Decision Diagrams_ of [[Dijk16](README.md#references)] to circumvent this.

### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Meinel94](README.md#references)] may also be possible to
implement in the setting of Time-forward processing used here.


## Optimising the current algorithms
There are quite a few avenues of trying to shave off a few significant constants
in the running time on the current algorithms. All suggestions below also make
the GPU an intriguing subject for a possible heavy improvement in the running
time.

### Non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe one will not gain too much due to the _O(sort(N))_ I/O lower bound.

### From _recursive_ algorithm to _time-forward processing_ and back again
Most implementations, such as the ones in [[Brace90,
Dijk16](README.md#references)], make use of a _unique node table_, which is a
hash-table in which all BDDs exist. This has the benefit of allowing one to
reuse common subtrees across BDDs (which saves space linear in the number of
concurrent BDDs in use) and the recursive algorithms run _2_ or even _4_ times
faster than the current algorithms (when they don't outgrow the main memory).

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

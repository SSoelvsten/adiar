# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate research projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Future Work](#future-work)
    - [Implementation of missing BDD algorithms](#implementation-of-missing-bdd-algorithms)
        - [If-Then-Else](#if-then-else)
        - [Function composition](#function-composition)
        - [Coudert's and Madre's Restrict](#couderts-and-madres-restrict)
        - [Variable reordering](#variable-reordering)
    - [Optimising the OBDD](#optimising-the-obdd)
        - [Complement Edges](#complement-edges)
    - [Extensions](#extensions)
        - [Multi-Terminal Binary Decision Diagrams](#multi-terminal-binary-decision-diagrams)
        - [Multi-valued Decision Diagrams](#multi-valued-decision-diagrams)
        - [Zero-suppressed Decision Diagrams](#zero-suppressed-decision-diagrams)
        - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)
        - [From _recursive_ algorithm to _time-forward processing_ and back again](#from-recursive-algorithm-to-time-forward-processing-and-back-again)
    - [Optimising the current algorithms](#optimising-the-current-algorithms)
        - [Non-comparison based sorting on numbers](#non-comparison-based-sorting-on-numbers)
        - [Parallelisation](#parallelisation)
            - [Parallel Layer-aware priority queue](#parallel-layer-aware-priority-queue)
            - [Reduction Rule 2 within the merge sorting](#reduction-rule-2-within-the-merge-sorting)
            - [Distribute nodes and information (in order) to worker-threads](#distribute-nodes-and-information-in-order-to-worker-threads)

<!-- markdown-toc end -->


## Implementation of missing BDD algorithms
There are still many features for BDDs not yet addressed in the _COOM_ library,
and valuable additions to the current project can be made in implementing these.
All of these of course should be made in the style of _Time-Forward Processing_
like the rest.

The features are sorted based on the difficulty deriving their design and their
implementation.

### If-Then-Else
The _ITE_ operator takes three OBDDs _f_, _g_, and _h_ and constructs an OBDD
which resembles the _(f ∧ g) ∨ (¬f ∧ h)_ formula. That is, if _f_ is true,
then output the value of _g_; otherwise output the value of _h_. one should be
able to reproduce the ideas in _Apply_ to do a simultaneous sweep over all three
OBDDs simultaneously in one go and gain a major performance gain over
constructing the above formula with `coom::apply` and `coom:negate`.

Yet, there are multiple edge-cases, in which one can do much, much better than
the above.

- If _f_ is a sink, it is trivial

- If _g_ and _h_ are sinks, it's also either _f_, _~f_, or a collapse to a sink.

- The labels of _f_ are 'above' _g_ and _h_. At this point, one can change the
  pointers of _f_ to point to the root of _g_ and _h_, where the ids of _h_ are
  decreased to not conflict with _g_.
  
  - If _g_ or _h_ also is separate from the other, then one can do it all
    without reduction by merely copying the nodes of _g_ and _h_.

There may be many more interesting and common cases, that may run much faster.

One can also encode many other interesting operators within the ITE operator,
where it might be worth investigating the performance difference between the use
of _ITE_ rather than the other implementation of the algorithm

| Operator    | Operator (formula) | ITE form                         |
|-------------|--------------------|----------------------------------|
| Composition | f(x, g(y), z)      | ITE(g(x), f(x, 1, z), f(x, 0, z) |
| Existence   | ∃y : f(x, y, z)    | ITE(f(x, 1, z), 1, f(x, 0, z))   |
| Forall      | ∀y : f(x, y, z)    | ITE(f(x, 1, z), f(x, 0, z), 0)   |

### Function composition
The _Composition_ of two OBDDs _f_ and _g_ for some label _i ∊ [n]_ is
_f ∘<sub>i</sub> g (x)_ and is to be interpreted as _f(x<sub>1</sub>, ...,
x<sub>i-1</sub>, g(x<sub>1</sub>, ..., x<sub>n</sub>), x<sub>i+1</sub>, ...,
x<sub>n</sub>)_.

### Coudert's and Madre's Restrict
The current _Restrict_ algorithm is the basic algorithm of Bryant, but one has
been proposed in [[Coudert90](README.md#references)] that is very different and
may be used in Verification. They also proposed algorithms with the name
_Constrain_ and _Expand_.

### Variable reordering
Currently, _COOM_ only uses a static ordering of the variables, but since the
size of the BDD is heavily influenced by the order chosen then many BDD
libraries provide variable reordering algorithms, or do these themselves
behdind the scenes.

How one can rephrase these algorithms within the design of _COOM_ will probably
require quite a bit of creativity, understanding of the I/O model and also of
the original variable reordering algorithms.


## Optimising the OBDD

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

### Multi-valued Decision Diagrams
By solely using an edge-based representation of the data-structure one can also
implement a _Multi-valued Decision Diagram_ (MDD) of
[[Kam98](README.md#references)]. This allows one to succinctly encode a function
from a non-boolean domain. Switching to the edge-based representation will lead
to rewriting almost all algorithms, but we may look into recreating the _List
Decision Diagrams_ of [[Dijk16](README.md#references)] to circumvent this.


### Zero-suppressed Decision Diagrams
A Zero-suppressed Decision Diagram
([ZDD](https://en.wikipedia.org/wiki/Zero-suppressed_decision_diagram)) is a
binary decision diagram, which is very compresed when representing sparse sets
of bit vectors. They make use of different reduction rules 

- Generalize the _Reduce_ algorithm
- Implement (Time-forward processing) algorithms for all the set operations.


### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Meinel94](README.md#references)] may also be possible to
implement in the setting of Time-forward processing used here.


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

See also the discussion in issue [#98](https://github.com/SSoelvsten/coom/issues/98)

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

### Parallelisation
The idea of Time-Forward-Processing is inherently sequential, but can we
parallelise parts of the algorithm? One may want to look into what possibilities
we have in parallel access to the underlying data structures and algorithms:

- [X] Parallel traversal of the same file stream

- [X] Parallelisable sorting algorithms (Sorting in TPIE is parallelised).
  
- [ ] Parallelisable I/O efficient priority queue, such as the one in
      [[Sitchinava12](README.md#references)].


#### Parallel Layer-aware priority queue

Already just allowing the priority queue to be sorted on a different thread than
where all other computation happens may improve performance.

1. **Layer i → layer j > i+1**

    Since the layer-aware priority queue knows which layer is the current and
    which one is the next, then the priority queue may on a separate thread
    prepare all requests send to layer _i+1_ from a layer prior to _i_ ahead of
    time. It may place these requests in sorted order in a separate list, which
    may then be implicitly merged when popping elements for layer _i+1_.


#### Reduction Rule 2 within the merge sorting

The biggest bottleneck is the priority queue (see above suggestion) and the
sorting of nodes within each layer of the _Reduce_ step. The computations
involved with Reduction Rule 1 are so few, that one cannot win from
parallelising it. The other list of possibly to-be reduced by Reduction Rule 2
nodes start out being sorted by their label and id to then go through two
sortings:

1. By their children to make the children-related nodes direct neighbours.

2. Back to their label and id to put them back in order with the transposed
   graph of where to forward information for later.

Between step _1_ and _2_ the list is split into two in a linear sweep: The list
of nodes to keep and the ones to remove in favour of another one kept. One may
include the linear sweep inside the merge procedure of the first parallelised
sorting to immediately generate these two lists and with that hopefully improve
the running time.


#### Distribute nodes and information (in order) to worker-threads

Let the main thread merely distribute the work: The file-stream is a work-queue
synchronised with the priority queue for other processes to take their jobs. The
main thread also is the only one allowed to pop from the priority queue. One may
also need a process to collect, manage and sort the output of all the workers or
force them to output in the right order using another priority queue.

The basic idea would then be to let a worker thread pop the next node from the
file stream and extracts from the priority queue the forwarded information they
need. They then process their information, push new information to be forwarded
into the priority queue and return the to-be outputted node or arcs.

Since each layer is independent, then all processes only need to wait for each
other to finish when going from one layer to another. With
[semaphores](https://en.wikipedia.org/wiki/Semaphore_(programming)) one can both
implement a fast and hardware-supported lock on popping the jobs from the file
stream and a barrier for synchronisation of threads when crossing from one layer
to the next.

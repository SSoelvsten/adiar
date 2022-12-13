# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate and graduate projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Additional BDD algorithms](#additional-bdd-algorithms)
    - [Set manipulation](#set-manipulation)
    - [Boolean Optimisation](#boolean-optimisation)
    - [Advanced satisfiability functions](#advanced-satisfiability-functions)
    - [Coudert's and Madre's BDD functions](#couderts-and-madres-bdd-functions)
- [Additional Features](#additional-features)
    - [Attributed edges](#attributed-edges)
    - [Hash Values](#hash-values)
    - [Proof Logging](#proof-logging)
    - [Boolean Vectors](#boolean-vectors)
- [Other Decision Diagrams](#other-decision-diagrams)
    - [Chained Decision Diagrams](#chained-decision-diagrams)
    - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)
    - [Multi-Terminal Binary Decision Diagrams](#multi-terminal-binary-decision-diagrams)
    - [Multi-valued Decision Diagrams](#multi-valued-decision-diagrams)
    - [Quantum Multiple-valued Decision Diagrams](#quantum-multiple-valued-decision-diagrams)
- [Dealing with Tiny BDDs](#dealing-with-tiny-bdds)
- [Shared Decision Diagrams](#shared-decision-diagrams)
- [Parallelization](#parallelization)
    - [Superscalarity and Pipelining in CAL](#superscalarity-and-pipelining-in-cal)
    - [Pipelining](#pipelining)
    - [Per-level Multi-threading](#per-level-multi-threading)
    - [Delayed Pointer Processing](#delayed-pointer-processing)
- [References](#references)

<!-- markdown-toc end -->


## Additional BDD algorithms
There are still many algorithms for BDDs not yet present in the _Adiar_ library,
and valuable additions to the current project can be made in implementing these.
All of these of course should be made in the style of _Time-Forward Processing_
like the rest.

The features are sorted based on the difficulty deriving their design and their
implementation. The first few already include a description and good ideas of
how to approach the implementation.

### Set manipulation

Verification relies heavily on manipulation of sets. For this, we need multiple
functions, most of which are aliases for other algorithms. See the documentation
of Sylvan for an overview of this. More interesting is Coudert and Madre's
_Meta-products_ representation [[Coudert92](#references)] of sets in BDDs and the
functions related to it.

### Boolean Optimisation

Donald Knuth solved the NP-complete combinatorial _travelling salesman_ problem (and
many others) to optimality in a graph of 50 nodes in one of his
[lectures](https://youtu.be/SQE21efsf7Y?t=3776) and also in *The Art of Programming
Volume 4A* on page 208 - 214. Given Adiar already being competetive for combinatorial
problems, this seems like an interesting way to go.

We should be able to derive the best (i.e. minimal or maximal) solution in O(sort(N))
I/Os with a similar algorithm as for Counting paths. The question then is, how does
one (a) maintain the entire path(s) of possible best solutions traversed or (b)
reconstruct the best path? I think (b) is the most promising by using O(N) extra space
and time after the initial sweep.

Alternatively, one may want to look into recreating the Counting algorithm bottom-up
and then generalising it for this application.

### Advanced satisfiability functions
The number of satisfiable assignments can be very large (even larger than
2<sup>64</sup> at times). Hence, the BDD package BuDDy also provides a
`bdd_satcountln` function, that outputs the logarithm of the number of
satisfiable assignments.

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


## Additional Features

### Attributed edges
The primary goal of using a decision diagram is to represent a complicated
boolean formula / set of elements in very little space. This is done by
removing as many "redundant" nodes and by sharing as many subtrees as possible.
By encoding information inside of the arcs one can further increase the
possibility to share subtrees. With *attributed edges* we add a single boolean
value inside of each arc which has different semantics for BDDs and ZDDs. For
BDDs we can expect on average a 7% factor decrease in the size of the BDD, and
also potentially up to a factor two speedup in the algorithms execution time
[[Brace90](#references)]!

**Semantics and Attributed-Edge Rule**

In other implementations with this optimisation only the *false* terminal exists;
the *true* terminal is the attribution flag set on the *false* sink. 

- **Binary Decision Diagrams**:

  The subtree for a function *f* is treated as *¬f*, i.e. flip the values of
  all its sinks. The bottom of page 42 in [[Brace90](#references)] shows how
  the attributed edge can be flipped on a single node. By convetion, one
  always picks the left member of each equivalent pair.

- **Zero-suppressed Decision Diagrams**:
 
  A subtree for the set *A* is treated as also including the empty set Ø
  (i.e. null combination vector "00...0"). Figure 10 in
  [[Minato93](#references),[Minato01](#references)] shows how to flip the
  attributed edge on a single node.

**Encoding Attributed Edges**

Our bit representation of _unique identifiers_ already has a single bit-flag,
which is currently unused in a _node_ and on the _target_ of an _arc_. These are
currently reserved for implementation of this very feature. The main question
about the encoding is whether the "only false sink" idea benefits our
representation? If we do choose to use it, then all binary operators in
*adiar/bool_op.h* need to **not** unflag the sink but rather manipulate the flag
itself.

**Adding Attributed Edges to Reduce**

- When outputting nodes for a level: Identify what case applies and whether
  in-going arcs should be flipped. This boolean value is sent through the second
  sorter to the forwarding phase.
  
- During the forwarding phase: The *attribute* flag of the forwarded edge is set
  based on (a) whether it should be flipped and (b) what is its value in the
  internal arcs stream.

- If the root should have its in-going arc flipped then place that value in the
  *negate* flag  of the resulting *node_file*.

One also needs to look at whether this addition breaks the idea of the
*canonical* flag on the *node_file*. That is, either find a counter-example or
prove that the same linear-scan equality checking algorithm still works. In
fact, if the linear-scan still works then this will asymptotically improve our
equality checking to be O(N/B) in all cases!

**Adding Attributed Edges to all other Algorithms**

All other algorithms now also need to take this flag into account.

- *Substitute*: Keep the flag as it was forwarded from the parent

- *Traverse*: Allow the visitor to know the flag on the in-going arc (at first,
  this is the negation flag of the *BDD* and then whether the flag was set on the
  chosen out-going arc in the prior step).

I have not figured out the rest of the functions, but one may be able to look at
the Sylvan implementation [[Dijk16](#references)] as a guide.


### Hash Values
This approach is based on a suggestion by Bryant for how to improve the
performance of equality checking. His idea is based on [[Blum80](#references)].
His intention was to obtain an O(N/B) I/O comparison at price of increased
memory, but the equality checking in
[#127](https://github.com/SSoelvsten/adiar/pull/127) obtains the same bound
without any increase in size by exploiting a feature of our _Reduce_ algorithm.
Yet, this idea of hashing could be useful for further improving the speed of our
equality checking in the negative cases.

Let _p_ be a prime number (though the math may work out even when doing all
computations with the non-prime p = 2<sup>k</sup>, i.e. by abusing the overflow
of unsigned integers). Consider a hash function _H_ (all numbers computed modulo
_p_) defined as follows

- Leaves hash to their value, i.e. H(0) = 0 and H(1) = 1

- Variables x<sub>i</sub> hash to a random value in [0;_p_)

- Internal nodes has as follows:
  H((x<sub>i</sub>), v<sub>0</sub>, v<sub>1</sub>) =
  H(x<sub>i</sub>) H(v<sub>1</sub>) + (1 - H(x<sub>i</sub>) H(v<sub>0</sub>))

Then the probability of two different BDDs share the same hash value is 1/_p_.

Notice, we only care about the hash value at the root, so we do not need to
store the hash value within each and every node. Instead, similar to
_canonicity_ in [#127](https://github.com/SSoelvsten/adiar/pull/127), we can
store the hash of the root (and its negation) as two numbers in the `node_file`
and merely propagate the hash values in the priority queue of `reduce`.


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

### Boolean Vectors

By placing BDDs (or ZDDs) in a _vector_ of length _b_ we can represent a *b*-bit
binary integer. These can be used to implement the logic for boolean vectors in
an SMT solver. The following description is based off the implementation for
_unsigned_ boolean vectors in the BDD library BuDDy; it would also be of interest
to have _signed_ boolean vectors with two's complement.

We can allow the user to choose whether to use BDDs or ZDDs to represent each bit
by templating the _bvec_ class and all the functions. If they choose to use ZDDs,
then we do need to have the domain defined to make bitwise negation make any
sense. It might make sense to start with BDDs only and then template it later.

**Constructors**

We want to be able to construct a _bvec_ in a few ways

- `bvec_true()`, and `bvec_false()` for the all-1 and all-0 vectors.
- `bvec_const(i)` to construct a _bvec_ that represents the constant integer *i*
- `bvec_vars(vs)` to construct a _bvec_ where the *i*th bit is the *i*th variable
  in *vs*. Alternatively, *vs* can be a function *index -> variable label*.

**Operators**

The following functions return a new _bvec_.

- Bit-wise operations such as bit-wise AND, OR, and XOR are quite easy to
  implement, since one merely needs to apply that operator per index.
 
  `bvec_and(x,y)` (`x & y`), `bvec_or(x,y)` (`x | y`), `bvec_xor(x,y)` (`x ^ y`), `bvec_not(x)` (`~ y`)
  
- Much more intersting are arithmetic operations, since the output bits are
  dependant on multiple input bits.
  
  - With Decision Diagrams on either side:
  
    `bvec_add(x,y)` (`x + y`), `bvec_sub(x,y)` (`x - y`), `bvec_mul(x,y)` (`x * y`), `bvec_div(x,y)` (`x / y`),
    
    `bvec_shr(x,y)` (`x >> y`), `bvec_shl(x,y)` (`x << y`)
  
  - Where the latter value *i* is a fixed *b*-bit number:
  
    `bvec_mulfixed(x,i)` (`x * i` and `i * x`), `bvec_divfixed(x,i)` (`x / i`)
    
    `bvec_shrfixed(x,i)` (`x >> i`), `bvec_shlfixed(x,i)` (`x << i`)

- We would also want some extra functions:

  - `bvec_truncate(x,b)` or `bdd_coerce(x,b)` to change *x* to be *b* bits. If it
    is made longer, then it is padded with extra *zeros*.

**Comparators**

The following functions return a single decision diagram (not a vector of
decision diagrams) which contains the desired result of a comparison of two
_bvec_s. The type is either a  _bdd_ or _zdd_ depending on what was the
templated value underneath.

`bvec_eq(x,y)`, `bvec_neq(x,y)` `bvec_lt(x,y)`, `bvec_le(x,y)`, `bvec_gt(x,y)`, `bvec_ge(x,y)`.

## Other Decision Diagrams

### Chained Decision Diagrams

Decision diagrams achieve their practical performance by also encoding
information inside of their arcs. Specifically, _Binary_ and _Zero-suppressed_
Decision Diagrams encode the information inside of skipping a level; in the
former skipped levels represent a _don't care_ chain while in the latter they
represent an _or_ chain. This makes each of them good at solving a problem,
depending on the nature of said problem. Yet, for _n_ variables a BDD can be
_n/2_ times larger than the same ZDD and vica versa [[Bryant18](#references)].

Independently Van Dijk [[Dijk17](#references)] and Bryant [[Bryant18](#references)]
came up with ideas of how to get the best of both worlds.

- **Tagged Decision Diagrams** [[Dijk17](#references)]

  Every outgoing arc of a node _n_ does not only identify the next node _m_,
  but also a variable _x_ in between _n.label_ and _m.label_ (inclusive). All
  nodes before _x_ are treated by one rule (e.g. the BDD rule) and all others
  afterwards with a second rule (e.g. the ZDD rule).
  
  - This works with [complement edges](#attributed-edges)

  - One of the reduction rules replaces it with a (possibly new) node that is
    not one of its children. It seems unlikely we can get this to work in Adiar.

- **Chained Decision Diagrams** [[Bryant18](#references)]

  No extra information is encoded inside of an edge, but instead every node now
  has a _begin_ variable _t_ (top) and _end_ variable _b_ (bottom). That is,
  every node now can be writtes as (_t_ : _b_, _low_, _high_). To encode all of
  this we probably need to switch to 128 bit numbers instead of 64 bit.
  
  - This does not change anything about how the _Reduce_ algorithm needs to work!

  - Most algorithms have already been generalised to work for BDDs and ZDDs by
    use of some hooks, e.g. the _cofactor_ function which they also modify. One
    should be able to build on top of this to 

  - They did not use complement edges for the sake of the experiment. But, there
    is no reason why they are not applicable to CZDDs. How they can be used with
    CBDDs without conflicting with the _or_ chain within a node may require some
    extra thought.

Yet, in both of their experiments, they had a hard time finding any cases where
ZDDs did not outperform BDDs, CBDDs. CZDDs are on-par (or better) than ZDDs. So,
currently the only motivation we have for this (except for: "because we can")
is to remove the need for the user to think about what representation to use.

### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Gergov94](#references)] may also be possible to
implement in the setting of Time-forward processing used here.

### Multi-Terminal Binary Decision Diagrams
One can extend the proposed representation of sink nodes to encompass non-boolean
values, such as integers or floats. To this end, one should template all structs
and functions in _data.h_ with the type to interpret the bit values in the leafs.
This templating should then be lifted to all _file_ types, the _decision diagram_
types, and finally all algorithms. This lifting may result in having to expose
internal algorithms and restructure the project. Yet, this templating will
immediately yield an I/O efficient implementation of _Multi-Terminal Binary
Decision Diagrams_ (MTBDD) of [[Fujita97](#references)].

### Multi-valued Decision Diagrams
TPIE allows one to use a custom _serializer_ and _deserializer_ for their
`tpie::file_stream` and `tpie::merge_sorter`. With this, one can change the
`adiar::node` struct to be of variable size such that they can describe the
_Multi-valued Decision Diagram_ (MDD) of [[Kam98](#references)]. Alternatively,
one can create the "union" of `adiar::uid_t` and `adiar::ptr_t` such that
consecutive (64-bit) elements describe a single node.

Another approach for MDDs would be to look into recreating the _List Decision
Diagrams_ of [[Dijk16](#references)] to keep the decision diagram binary.

### Quantum Multiple-valued Decision Diagrams
Assuming Adiar's supports [MDDs](#multi-valued-decision-diagrams), then one can
add support for _Quantum Multiple-valued Decision Diagram_ (QMDD)
[[Miller06](#references)]. Alternatively, if Adiar's nodes are changed such
their out-degree is a template parameter then one can do without all the
headache that MDDs may bring.

The main addition is to annotate each arc with a complex number (i.e. two
doubles) and then incorporate the logic within Adiar's algorithm templates.

## Shared Decision Diagrams

**NOTE:** This is best done in a completely new repository, where some files
(e.g. `data.*`, `tuple.*`, `levelized_priority_queue.*`) are moved over.

Equality Checking in _Adiar_ has an _O(sort(N))_ worst case I/O complexity,
which in most practical cases is going to be an _2 N/B_ complexity. Since the
decision diagrams are stored in separate files, it is unlikely that one can
improve this further (both in the constant and in the asymptotics). Question is,
whether on can have a _shared_ node table while still being able to deploy the
time-forward processing applied here - in this case equality checking would be
decreased to _O(1)_ as desired. In many ways, one can take inspiration from the
work of [[Ochi93, Ashar94, Sanghavi96](#references)], but circumvent the
asymptotic bad performance by not using a hash-table for every level.

Let _T_ be the number of elements in the common node data structure.

- The _Apply_ should then use at most _O(T/B + sort(N<sub>f</sub>
  N<sub>g</sub>))_ I/Os in the worst case.

  Here, "possibly new nodes" that are pairs of existing nodes are placed in a
  separate output and the following _Reduce_ would bottom-up identify the
  already existent nodes and add new nodes. The _Apply_ procedure can even
  shortcut computation for some recursion requests, (_v_, _w_), when _v_ = _w_.

- The _Reduce_ becomes much more complicated, since one has to check whether the
  resolved node is a duplicate or should be added to the nodes on said level.
  The most vital question to resolve to this end is what the common _unique node
  data structure_ should be?

  - The most likely good way to do so is with a list of nodes sorted
    lexicographically by their children. This way, duplicates can be found in a
    single scan. One needs to be sure to keep it sorted while adding new nodes,
    but this should be possible as a merge of the two sorted lists; one may even
    be able to do this as an in-place merge on the very _unique nodes_ list.

  - A B-tree may also be applicable to this use case.

  If it is a mere list of nodes, then garbage collection is a simple top-down
  mark-and-sweep using time-forward processing that uses _O(sort(T))_ number of
  I/Os.

The benefit of all this work would be to decrease equality checking to _O(1)_ in
the worst case and to decrease the overall memory occupied, when one has a lot
of concurrent decision diagrams alive. Yet, this does also happen at the cost of
having to read a much larger input. Only a practical evaluation can gauge
whether it is of benefit.

## Dealing with Tiny BDDs

With Adiar v1.2 we have been able to decrease the threshold as to when Adiar is
viable in comparison to conventional depth-first BDD packages. Yet, this does
not remove the major relative performance decrease for tiny BDDs. Interestingly,
the threshold we derived in our experiments is somewhat below a threshold in the
CAL BDD package [[Sanghavi96](#references)]: if below this size, it switches to
the conventional depth-first algorithms instead. Hence, it is likely that we
have hit a lower bound on when Adiar's time-forward processing is viable.

For an end-user, it is of interest to have Adiar be efficient across the board.
That is, we want to lower this threshold down to the smallest possible BDD. To
this end, we will have to (similar to CAL) switch to depth-first algorithms.
But, instead of implementing a whole new BDD pacakge we ought to interop with
another BDD package! Specifically, I have the following two in mind:

- *BuDDy:* This BDD package has a very simple and elegant design and throughout
  all benchmarks it was the fastest. Its design should make it quite easy to
  integrate it with Adiar's algorithms. Its only problems are, that it is not
  thread-safe nor supports ZDDs. The latter can be fixed, but the prior may
  be impossible.

- *Sylvan:* This BDD package provides support for BDDs and ZDDs and also
  thread-safety. Yet, using its pointers to BDDs may be considerably more
  complicated. Furthermore, its use of attributed edges may break with Adiar.

Here, we need to address the following three issues.

1. Run the equivalent algorithm in the other BDD package, if
   - The inputs are all BDDs in the other BDD package.
   - The size of all inputs is below the 2<sup>19</sup> threshold of CAL.
   - The worst-case output size fits within the unique node table assuming no
     nodes can be reused. This may be complicated in practice, as garbage
     collection may provide the space needed.

2. In Adiar's top-down algorithms a node uid of the input either points to a
   node in one of Adiar's files or a node in the unique node table.
   - Here the `node_stream` class simulates a stream by having the `.seek(...)`
     function in *O(1)* time do a lookup in the unique node table.
   - For the priority queues to work, we need to add a wrapper on the nodes,
     such that they can also carry around the level.

3. In Adiar's Reduce algorithm, if the output fits within the unique node table
   then the `node_writer` should create the entire BDD within the unique node
   table.

A lot of this can be achieved similar to the templating done for v1.2 itself.

## Parallelization

Currently, Adiar only makes use of *1* processor thread and is able to get close
to other conventional BDD packages running sequentially. With some fiddling this
can be improved such that Adiar is multi-threaded to the same degree as the
BeeDeeDee package: each operation is still sequential on a thread, but multiple
threads can in parallel run their own BDD operations.

Yet, BDD packages like Sylvan or PJBDD support true multi-threading, i.e.
multiple BDD operations can run in parallel and each operation also is
parallelized. The question is, how far can we get Adiar to the same? Below are
sketches for possible directions to go with the aim of multi-threading.

**NOTE:** All suggestions below that involve multi-threading are dependent on
thread-safety of TPIE. Hence, to be able to parallelize Adiar, we first need to
be sure to make Adiar thread-safe and double-check the necessary parts of TPIE
is too.

### Superscalarity and Pipelining in CAL

The Breadth-first BDD package CAL [[Sanghavi96](#references)] supports
parallelization in a slightly different way: the CPU still only uses a single
core, but it solves multiple BDD operations at the same time. These ideas can
maybe be reapplied directly in the context of Adiar to improve the performance
in practice; or more likely they can serve as inspiration for other avenues of
parallelization.

**Superscalarity**

In [[Sanghavi96](#references)] "superscalarity" is the idea of running
*independent* operations within a single sweep. Since this can be achieved in
Adiar with actual thread-safety, this may only becomes relevant if Adiar
switches to using a [shared forest of BDD nodes](#shared-decision-diagrams).

**Pipelining**

In [[Sanghavi96](#references)] "pipelining" is the idea of running *dependent*
operations within a single sweep. They base this off of the following observations

1. There is a one-to-one correspondence between requests and the nodes in the
   unreduced BDD..
2. Operands can also be applied on unreduced BDDs to create another valid
   unreduced BDD.
3. Handling each request *R* is a local operation: *R* is based on one or more
   nodes at a level split into *R<sup>low</sup>* and *R<sup>high</sup>*. That
   is, an operation (even one dependent on another) can progress, assuming both
   the *low* and *high* arcs are available (see **2**).

With minor tweaks, these observations also do apply for Adiar. Hence, one can
have one top-down sweep right behind another.

### Pipelining

The observations and idea for *pipelining* in [[Sanghavi96](#references)] (see
[above](#superscalarity-and-pipelining-in-cal)) has also independently sprung to
my mind: in practice, the Reduce algorithm only removes very few nodes and hence
does not do much except safe 1/4 of space. Furthermore, from an
automata-language perspective there is no difference between an unreduced OBDD
and its equivalent ROBDD (this is the same as observation **2** above) and so we
can run a top-down apply operation directly on unreduced BDDs.

**Top-down Operations: Levelized Pipe**

Hence, we can skip the Reduce step and have one operation start processing level
*i* of another operation when all arcs with source at level *i* have been
processed. To this end, we need to replace Adiar's *writer* and *stream* classes
with some kind of a *pipe*. To Adiar's algorithms, nothing really changes: they
still push and pull from these sequential streams. Hence, most of this can be
achieved with further templating Adiar's *internal/* folder.

The pipe on the other hand needs to orchestrate what is the *safe to read* parts
of the unreduced output still under construction. Since the top-down algorithms
work on nodes but they produce *arcs*, we need to convert them on-the-fly back
to be nodes. Here, we can use a *sorter* similar as in the levelized priority
queue. A level is safe to read, when the sorter for level *i* includes all of
the arcs corresponding to requests made at level *i*. The number of requests is
directly given as part of the `level_info` in the meta data.

This effectively splits the queue in two: (1) the ready sorters that can be used
by the target operation at the end of the pipe and (2) the sorters still waiting
for some elements by the source operation at the start of the pipe. All of this
can be boiled down into a single semaphore `levels_ready`: for (1) it is
decremented each time the target operation proceeds and for (2) it is
incremented each time a level is fully finished.


**Negation Operation**

Negation can be stored inside this computation tree as a "complement edge". If
the computation result is negated, then this merely is a flag on the pipe to do
so on the fly (similar to what we do now).


**Bottom-up Reduce Operation**

The final BDD still has to be an ROBDD to safe on space. Some operations, e.g.
Equality Checking, may also take advantage of the canonical form of Adiar's
ROBDDs.

Yet, even in all other cases it might be valuable to not pipe two top-down
sweeps together in favour of decreasing the size of the unreduced OBDD. Here,
one can make a prediction of how many nodes will be removed with a Reduce. When
this estimate meets a certain threshold, then one can place a Reduce computation
inside of the pipe to keep the disk usage close to what otherwise would be used.


**Processing/Scheduling Tree**

It probably is necessary to first precompute some (read-only) binary tree that
describes the operations and the base cases.

Part of this tree should also be whether the operation should be parallelised or
not, because every thread should have some disjoint amount of TPIEs memory. So,
one needs to find a scheduling on the tree, such that all concurrent threads
have 128+ MiB of their own, and which balances the number of threads with the
amount of the memory they have.

As part of this scheduling, one may want to use the numbers in
[[Sanghavi96](#references)] as a guide.


### Per-level Multi-threading

There are a few avenues where Adiars algorithms can be parallelised.

- The levelized priority queue can be fully replaced with per-level sorting
  algorithms, which can be parallelised.

Furthermore, within a single level, each recursion request is independent of all
other requests. Hence, while the *time-forward processing* technique is
sequential with respect to the levels, we can maybe parallelize it for each
level.

Specifically, the recursion requests from the levelized priority queue can be
distributed to worker threads. Each worker has its own sequential reading of the
input stream and will only deal with requests *ahead* of itself. There are two
possible directions to take with this worker thread (the second of which sounds
the most interesting and promising):

1. Each worker places the request back into a per-level priority queue. This
   safes on memory but there is likely a lot of contention on these per-level
   priority queues.
   
   - This may lead to some workers being blocked, since they are ahead of the
     per-level priority queue. One solution is for them to keep processing based
     on requests in the levelized priority queue, even if it is ahead of the
     per-level priority queue (as some other thread must be behind it).

2. Each worker has its own per-level priority queue(s) to defer some requests.
   This decreases contention on shared data and so benefits of parallelization
   are more likely.
   
   - It might be possible to have no coordinating *main* thread but only a
     parallelized levelized priority queue and a thread safe writer. If this is
     the case, then a singly-threaded version essentialy is just a single worker
     thread!
     
     This also leads to no threads being blocked. Each thread is still behind
     the levelized priority queue and can take from that or its own per-level
     priority queue.
   
   - To decrease the contention on the sorters in the levelized priority queue,
     each worker thread can push to its own block rather than a shared one. This
     block is then sorted and pushed to disk as a base case.

### Delayed Pointer Processing

In [[Arge10](#references)] the *delayed pointer processing* technique was
described as a way to design graph algorithms that are both I/O-efficient and
parallelized.

**NOTE:** I (Steffan Sølvsten) have not read said paper in detail, so I cannot
guarantee the technique applies to Adiar.

## References

- [[Arge10](https://ieeexplore.ieee.org/abstract/document/5470440)]
  Arge, Lars, Michael T. Goodrich, and Nodari Sitchinava. “_Parallel external
  memory graph algorithms_”. In: IEEE International Symposium on Parallel &
  Distributed Processing (IPDPS). (2010).


- [[Blum80](https://www.sciencedirect.com/science/article/pii/S0020019080900782)]
  Manuel Blum, Ashok K. Chandra, and Mark N.Wegman. “_Equivalence of free
  boolean graphs can be decided probabilistically in polynomial time_”. In:
  _27th ACM/IEEE Design Automation Conference_. pp. 40 – 45 (1990)

- [[Brace90](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=114826)]
  Karl S. Brace, Richard L. Rudell, and Randal E. Bryant. “_Efficient
  implementation of a BDD package_”. In: _Information Processing Letters 10.2_.
  (1980)

- [[Bryant18](https://link.springer.com/chapter/10.1007/978-3-319-89960-2_5)]
  Randal E. Bryant. “_Chain Reduction for Binary and Zero-Suppressed Decision
  Diagrams_”. In: _Tools and Algorithms for the Construction and Analysis of
  Systems_. (2018)

- [[Bryant21](https://arxiv.org/abs/2105.00885)]
  Randal E. Bryant, Marijn J. H. Heule. “_Generating Extended Resolution Proofs
  with a BDD-Based SAT Solver (Extended Paper)_”. In: _arXiv_. (2021)

- [[Cheong94](https://ieeexplore.ieee.org/document/629886)]
  Pranav Ashar and Matthew Cheong. “_Efficient breadth-first manipulation of
  binary decision diagrams_”. In: _Proceedings of the
  1994 IEEE/ACM International Conference on Computer-Aided Design_. (1994)

- [[Coudert90](https://link.springer.com/chapter/10.1007/978-1-4615-0292-0_4)]
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

- [[Dijk17](https://ieeexplore.ieee.org/abstract/document/8102248)]
  Tom van Dijk, Robert Wille, Robert Meolic. “_Tagged BDDs: Combining reduction
  rules from different decision diagram types_”. In: _Formal Methods in Computer
  Aided Design_. (2017)

- [[Drechsler01](https://ieeexplore.ieee.org/document/905674)]
  Rolf Drechsler Wolfgang Günther. “_Using lower bounds during dynamic BDD
  minimization_”. In: _IEEE Transactions on Computer-Aided Design of Integrated
  Circuits and Systems _. (2001)

- [[Fujita97](https://link.springer.com/article/10.1023/A:1008647823331#citeas)]
  M. Fujita, P.C. McGeer, J.C.-Y. Yang. “_Multi-Terminal Binary Decision
  Diagrams: An Efficient Data Structure for Matrix Representation_”. In: _Formal
  Methods in System Design_. (2012)

- [[Gergov94](https://citeseer.ist.psu.edu/viewdoc/citations;jsessionid=EFE9D48EA6750B06CD2FB09B23D5CF2E?doi=10.1.1.45.5821)]
  J. Gergov and C. Meinel. “_Efficient analysis and manipulation of OBDDs can
  be extended to FBDDs_”. (1994)

- [[Hajighasemi14](https://essay.utwente.nl/66388/)]
  Maryam Hajighasemi. “_Symbolic Model Checking using Zero-suppressed Decision
  Diagrams_”. Master's Thesis, University of Twente (2014)

- [Kam98]
  Timothy Kam, Tiziano Villa, Robert K. Brayton, and L. Sangiovanni-vincentelli
  Alberto. “_Multi-valued decision diagrams: Theory and applications_”. In:
  _Multiple- Valued Logic 4.1_ (1998)

- [[Lind-Nielsen99](http://www.itu.dk/research/buddy/)]
  Jørn Lind-Nielsen. “_BuDDy: A binary decision diagram package_”. Technical
  report, _Department of Information Technology, Technical University of
  Denmark_, 1999.

- [[Miller06](https://ieeexplore.ieee.org/document/1623982)]
  D. Michael Miller and Mitchell A. Thornton. “_QMDD: A Decision Diagram
  Structure for Reversible and Quantum Circuits_”. In: _36th International
  Symposium on Multiple-Valued Logic (ISMVL'06)_ (2006)

- [[Minato93](https://dl.acm.org/doi/pdf/10.1145/157485.164890)]
  S. Minato. “_Zero-suppressed BDDs for set manipulation in combinatorial
  problems_”. In: _DAC '93: Proceedings of the 30th international Design
  Automation Conference_ (1993)

- [[Minato01](https://eprints.lib.hokudai.ac.jp/dspace/bitstream/2115/16895/1/IJSTTT3-2.pdf)]
  S. Minato. “_Zero-suppressed BDDs and their applications_”. In: _International
  Journal on Software Tools for Technology Transfer, 3_ (2001)

- [[Ochi93](https://www.computer.org/csdl/proceedings-article/iccad/1993/00580030/12OmNAXglQz)]
  Hiroyuki Ochi, Koichi Yasuoka, and Shuzo Yajima. “_Breadth-first manipulation
  of very large binary-decision diagrams_”. In: _Proceedings of 1993
  International Conference on Computer Aided Design (ICCAD),_ (1993)

- [[Rudell93](https://ieeexplore.ieee.org/document/580029)]
  Richard Rudell. “_Dynamic variable ordering for ordered binary decision
  diagrams_”. In: _ Proceedings of 1993 International Conference on Computer
  Aided Design_ (1993)

- [[Sanghavi96](https://link.springer.com/article/10.1007/s002360050083)
  Jagesh V. Sanghavi, Rajeev K. Ranjan, Robert K. Brayton, and Alberto
  Sangiovanni-Vincentelli. “_High performance BDD package by exploiting
  memory hierarchy_”. In: _Proceedings of the 33rd Annual Design
  Automation Conference_ (1996)

- [[Somenzi15](https://docplayer.net/34293942-Cudd-cu-decision-diagram-package-release-3-0-0.html)]
  Fabio Somenzi. “_CUDD: CU decision diagram package, 3.0_”. University of
  Colorado at Boulder (2015)

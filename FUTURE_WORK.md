# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate and graduate projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Future Work](#future-work)
    - [Missing BDD algorithms](#missing-bdd-algorithms)
        - [Set manipulation](#set-manipulation)
        - [Linear Optimisation](#linear-optimisation)
        - [Advanced satisfiability functions](#advanced-satisfiability-functions)
        - [Coudert's and Madre's BDD functions](#couderts-and-madres-bdd-functions)
    - [Additional features](#additional-features)
        - [Attributed edges](#attributed-edges)
        - [Hash Values](#hash-values)
        - [Proof Logging](#proof-logging)
        - [Dynamic Variable Reordering](#dynamic-variable-reordering)
    - [Other Decision Diagrams](#other-decision-diagrams)
        - [Multi-Terminal Binary Decision Diagrams](#multi-terminal-binary-decision-diagrams)
        - [Multi-valued Decision Diagrams](#multi-valued-decision-diagrams)
        - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)
        - [Shared Decision Diagrams](#shared-decision-diagrams)
    - [Optimising the current algorithms](#optimising-the-current-algorithms)
        - [Levelized Files](#levelized-files)
        - [Levelized Parallel Computation](#levelized-parallel-computation)
        - [From _recursion_ to _time-forward processing_ and back again](#from-recursion-to-time-forward-processing-and-back-again)
    - [References](#references)

<!-- markdown-toc end -->


## Missing BDD algorithms
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

### Linear Optimisation

Given a linear cost function on the input variables for a BDD or ZDD one should
be able to derive the value of the best (i.e. minimal or maximal) solution in
O(sort(N)) I/Os with a similar algorithm as for Counting paths.

The question then is, how does one (a) maintain the entire path traversed or (b)
reconstruct said path?

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


## Additional features

### Attributed edges
Currently, we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique. In the
recursive algorithms, one can even expect a factor two decrease in the
algorithms execution time [[Brace90](#references)].

Our bit representation of _unique identifiers_ already has a single bit-flag,
which is currently unused in a _node_ and on the _target_ of an _arc_. These are
currently reserved for implementation of this very feature, meaning we primarily
are lacking the additional logic in all of the algorithms.


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


### Dynamic Variable Reordering

Many other BDD packages provide dynamic variable reordering (e.g. BuDDy
[[Lind-Nielsen99](#references)] and CUDD [[Somenzi15](#references)]), where the
order of the variables are changed with the goal to decrease the size of the
decision diagram in question. Finding the optimal variable order is an
NP-complete problem, so all packages either use some notion of a local search
procedure to find a _somewhat_ good ordering or some other heuristics.

**Reordering a single BDD (Sifting)**

The most famous reordering procedure is Rudell's sifting algorithm
[[Rudell93](#references)]. This is based on the observation by prior authors
that two adjacent variables can be swapped without affecting any nodes of any
other variable in the DAG. To allow this swapping in _Adiar_, one will need to
first look into the [Levelized Files](#levelized-files) refactor below. After
this is done, then the sifting algorithm can be mapped to the decision diagrams
of _Adiar_.

Since the _meta file_ in _Adiar_ also provides the size of each level, then one
can use the lower bounds in [[Drechsler01](#references)] to prune the search of
the sifting algorithm.

**Matching ordering between BDDs**

Implementing the sifting algorithm would suffice for other BDD packages, but not
for _Adiar_, since (almost) all decision diagrams are stored independently. So,
one may end up with two decision diagrams using two different variable orders π
and π'. To make _apply_ and other non-unary algorithms work with such decision
diagrams they have to be put onto the same variable order. A few algorithms have
been designed for specifically such cases. If one or more of these are
translated into an I/O efficient version, then one can put the given decision
diagrams on a common order before running the desired non-unary operation.

The most promising seems to be the level-by-level algorithm of Savický and
Wegener [[Savický97](#references)]. This is an iterative version of the
recursive algorithms in [[Meinel94](#references)] and [[Bern05](#references)],
which heightens my confidence in highlighting this very procedure. For
simplicity one may assume that the new desired ordering is the identity. Let _f_
be the original BDD.

1. Start with a recursion request for the root _x_<sub>1</sub>.

2. For every level, merge recursion requests that induce the same graph in the
   original BDD.

  - Obtain an assignment σ that reflects a path going to each node. Here, one
    needs to traverse the under-construction BDD back up to the root. To this
    end, the [Levelized Files](#levelized-files) may again make all of this
    easier.

  - For two different requests to the same level with assignments σ and σ' check
    whether they induce the same graph, i.e. check whether _Restrict(f, σ) =
    Restrict(f, σ')_. If they induce the same graph, then merge these two
    requests.

    In the paper they do this by constructing a _keyword_ string that describes
    the DFS walk of the restricted BDD. Essentially, we can think of them having
    computed the two restricts. They only ever have two of them present at any
    moment to safe on space, and instead recompute keywords if needed. So, it
    makes much more sense to compute whether they induce the same subgraph with
    an `bdd_restrict_isomorphic` algorithm taking the logic of `bdd_restrict`
    with the optimisations of `is_isomorphic`.

3. Create new recursion requests for internal-node children and immediately
   output sink-children.

Due to _2._ and _3._ one does in fact not need to use `reduce`. Instead one can
immediately convert the `arc_file` to a `node_file` (though maybe each level
should be sorted to make it _canonical_). Yet, I wonder whether it in practice
is not better to leave it to the current Reduce implementation to remove
duplicate subtrees (which skips most of the work in _2._) even though it breaks
the worst-case guarantees.

## Other Decision Diagrams

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
consecutive (64-bit) elements describe a single node. Another approach for MDDs
would be to look into recreating the _List Decision Diagrams_ of
[[Dijk16](#references)] to keep the decision diagram binary.

### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Gergov94](#references)] may also be possible to
implement in the setting of Time-forward processing used here.

### Shared Decision Diagrams

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

### Levelized Parallel Computation
A few experiments show, that the Reduce algorithm in practice removes very few
redundant nodes. So, the Reduce only really puts it on canonical form and saves
on space. Should this premise be true even for larger cases, then we may be able
to obtain a major boost in performance by computing operations in parallel.

For example, consider a `bdd_apply(bdd_apply(...), ...)`. The outer Apply can
already start processing nodes on level _i_ when the inner one is at a level
_j_>_i_. This puts all Applys (and all other single-sweep BDD operations!) in a
_levelized lockstep_


**Levelized Pipe**

To this end, the outer Apply needs a _pipe_ that feeds it the _safe to read_
parts of the output of the inner Apply. From the inner Applys perspective, it
needs as an argument an output handler (i.e. a _pipe_):

1. If the output is supposed to be a reduced file, then they are pushed as before
   to two different files. Then these can be Reduced.

2. If the output is supposed to be piped directly into another `bdd_apply` then
   we place the inserted arcs into the relevant `tpie::merge_sorters` in a
   queue. This is in many ways akin to the levelized priority queue.

   - We also remember the number of elements pushed to each sorter. This way we
     can compare with the meta data to see whether a level has all the expected
     elements or not.

     This effectively splits the queue in two: (1) the ready sorters that can be
     used by the Apply at the end of the pipe and (2) the sorters still waiting
     for some elements by the Apply at the start of the pipe.

     One can store the size of part (1) in a single number `levels_ready`.

   - Due to this levelized lockstep, most synchronisation between threads can be
     done with _semaphores_ inside the pipe. For example, when going from one
     level to another, one decreases its `levels_ready` semaphore; if it turns
     negative, then it is because the next sorter is part of half (2) rather
     than (1).

     This semaphore ensures that the thread of the receiving Apply is only ever
     _runnable_ if it has something to process.

   - The meta data itself needs to be stored in a thread shareable file, i.e.
     one should use the `tpie::file` and `tpie::file::stream` classes with a
     semaphore or a simple lock.

3. If the input is a base case (e.g. `bdd_ithvar(i)`), then some BDD
   construction algorithms could be done top-to-bottom rather than bottom-up. If
   the [levelized files](#levelized-files) above are added too, then this makes
   a level available before the underlying algorithm has finished constructing
   all nodes.

   This kind of a pipe may not be worth the effort though, since most of these
   BDDs are smaller than a few thousand nodes (i.e. less than a few kB).

**Processing/Scheduling Tree**

It probably is necessary to first precompute some (read-only) binary tree that
describes the operations and the base cases.

Part of this tree should also be whether the operation should be parallelised or
not, because every thread should have some disjoint amount of TPIEs memory. So,
one needs to find a scheduling on the tree, such that all concurrent threads
have 128+ MiB of their own, and which balances the number of threads with the
amount of the memory they have.


**Piped Apply**

The Apply itself works as before with time-forward processing. The main
difference is, that its two arguments may not be a fully computed BDD in a file,
but rather some set of operations instead (i.e. a node in a processing tree).

For each of these uncomputed results we create a pipe and feed it to a newly
spawned thread. The Apply itself then runs as usual by pulling from the
streams/pipes and processing the nodes and then pushing to its own output pipe.
To this end, one can make the `node_stream` change behaviour based on what kind
of pipe it was given at construction time.


**Negation**

Negation can be stored inside this computation tree as a "complement edge". If
the computation result is negated, then this merely is a flag on the pipe to do
so on the fly (similar to what we do now).


**Expected Gain of Reduce**

Based on experiments or even prior runs of Reduce, one can make a prediction of
how many nodes will be removed with the Reduce. When this estimate meets a
certain threshold, then one can place a Reduce computation inside of the pipe to
keep the disk usage close to what otherwise would be used.


### From _recursion_ to _time-forward processing_ and back again
As mentioned in issue [#98](https://github.com/SSoelvsten/adiar/issues/98) the
current set of algorithms are very slow for very small instances. This is in parts
due to the nature of the TPIE data structures. So, some of it can be regained by
switching to internal memory data structures, but some slowdown may just be inherent
to the very nature of our iterative algorithms.

Most other implementations (e.g. [[Brace90, Dijk16, Lind-Nielsen99, Somenzi15](#references)]),
make use of a _unique node table_, which is a hash-table in which all BDDs
exist. This has the benefit of allowing one to reuse common subtrees across BDDs
(which saves space linear in the number of concurrent BDDs in use) and the
recursive algorithms run for very small instances by several orders of
magnitudes faster than _Adiar_.

**Bridging the gap**

By bridging the gap between the two methods, we get the best of both world. To do
this, parts of the given memory is not used for TPIE, but rather for a separate
unique node table and a cache. For small amounts of memory half of what is given
can be dedicated to these internal memory decision diagrams.

A BDD is then either (1) a TPIE file or (2) a pointer to a node in the hash table.

All BDD operations then use O(1) time and I/Os to switch between different versions.

  - When an algorithm is only given decision diagrams in internal memory, then
    the classical recursive algorithms are used.
    
  - If both decision diagrams are in external memory, then the current algorithms
    are used.
    
  - If the given decision diagrams are mixed, then the external memory algorithms
    are used. To this end, the traversal of the internal memory decision diagram
    has to be simulated, which can be done with a sorted set or priority queue.
    
    The `tpie::internal_priority_queue` may be of interest in this case. Most of
    this can also be handled at compile time with templating.
    
As described in [#98](https://github.com/SSoelvsten/adiar/issues/98), we can
easily compute an upper bound on the output size. With this, we can figure out,
whether the output can be within the _unique node table_ or has to be placed in
an external memory file.

The Reduce algorithm can be templated such that it either outputs to a TPIE file
or uses the _find-or-insert_ function to place its output. This templating also
includes changing what is forwarded in its priority queue from a _uid_ to an
actual pointer into the hash table.

**Alternative**

Instead of reimplementing an entire hash table based approach, one may want to
look into using another BDD library underneath. The Sylvan library can first
really be used, when [attributed edges](#attributed-edges) as described above
has already been added; until then BuDDy and CUDD are possible options.


## References

- [[Bern05](https://link.springer.com/chapter/10.1007/3-540-60045-0_36)]
  Jochen Bern, Christoph Meinel, Anna Slobodová. “_Global rebuilding of OBDDs
  avoiding memory requirement maxima_”. In: _Computer Aided Verification_ (2005)

- [[Blum80](https://www.sciencedirect.com/science/article/pii/S0020019080900782)]
  Manuel Blum, Ashok K. Chandra, and Mark N.Wegman. “_Equivalence of free
  boolean graphs can be decided probabilistically in polynomial time_”. In:
  _27th ACM/IEEE Design Automation Conference_. pp. 40 – 45 (1990)

- [[Brace90](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=114826)]
  Karl S. Brace, Richard L. Rudell, and Randal E. Bryant. “_Efficient
  implementation of a BDD package_”. In: _Information Processing Letters 10.2_.
  (1980)

- [[Bryant21](https://arxiv.org/abs/2105.00885)]
  Randal E. Bryant, Marijn J. H. Heule. “_Generating Extended Resolution Proofs
  with a BDD-Based SAT Solver (Extended Paper)_”. In: _arXiv_. (2021)

- [[Cheong94](https://ieeexplore.ieee.org/document/629886)]
  Pranav Ashar and Matthew Cheong. “_Efficient breadth-first manipulation of
  binary decision diagrams_”. In: _Proceedings of the
  1994 IEEE/ACM International Conference on Computer-Aided Design_. (1994)

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

- [[Meinel94](https://link.springer.com/chapter/10.1007/3-540-58338-6_98)]
  Christoph Meinel, Anna Slobodová. “_On the complexity of constructing optimal
  ordered binary decision diagrams_”. In: _International Symposium on
  Mathematical Foundations of Computer Science_ (1994)

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

- [[Savický97](https://link.springer.com/article/10.1007/s002360050083)
  Petr Savický, Ingo Wegener. “_Efficient algorithms for the transformation
  between different types of binary decision diagrams_”. In: _Acta Informatica
  volume 34_ (1997)

- [[Somenzi15](https://docplayer.net/34293942-Cudd-cu-decision-diagram-package-release-3-0-0.html)]
  Fabio Somenzi. “_CUDD: CU decision diagram package, 3.0_”. University of
  Colorado at Boulder (2015)

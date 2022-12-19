# Future Work

Random notes one possible (but not definitely planned) future work.

**Table of Contents**

- [Shared Decision Diagrams](#shared-decision-diagrams)
- [Parallelization](#parallelization)
    - [Pipelining](#pipelining)
    - [Per-level Multi-threading](#per-level-multi-threading)
    - [Delayed Pointer Processing](#delayed-pointer-processing)
- [References](#references)

## Shared Decision Diagrams

**NOTE:** This is best done in a completely new repository, where internal data
types and data structures are moved over.

Equality Checking in *Adiar* has an *O(sort(N))** worst case I/O complexity,
which in most practical cases is going to be an *2 N/B* complexity. Since the
decision diagrams are stored in separate files, it is unlikely that one can
improve this further (both in the constant and in the asymptotics). Question is,
whether on can have a *shared* node table while still being able to deploy the
time-forward processing applied here - in this case equality checking would be
decreased to *O(1)** as desired. In many ways, one can take inspiration from the
work of [[Ochi93, Ashar94, Sanghavi96](#references)], but circumvent the
asymptotic bad performance by not using a hash-table for every level.

Let *T* be the number of elements in the common node data structure.

- The *Apply* should then use at most *O(T/B + sort(N<sub>f</sub>
  N<sub>g</sub>))** I/Os in the worst case.

  Here, "possibly new nodes" that are pairs of existing nodes are placed in a
  separate output and the following *Reduce* would bottom-up identify the
  already existent nodes and add new nodes. The *Apply* procedure can even
  shortcut computation for some recursion requests, (*v*, *w*), when *v* = *w*.

- The *Reduce* becomes much more complicated, since one has to check whether the
  resolved node is a duplicate or should be added to the nodes on said level.
  The most vital question to resolve to this end is what the common *unique node
  data structure* should be?

  - The most likely good way to do so is with a list of nodes sorted
    lexicographically by their children. This way, duplicates can be found in a
    single scan. One needs to be sure to keep it sorted while adding new nodes,
    but this should be possible as a merge of the two sorted lists; one may even
    be able to do this as an in-place merge on the very *unique nodes* list.

  - A B-tree may also be applicable to this use case.

  If it is a mere list of nodes, then garbage collection is a simple top-down
  mark-and-sweep using time-forward processing that uses *O(sort(T))** number of
  I/Os.

The benefit of all this work would be to decrease equality checking to *O(1)** in
the worst case and to decrease the overall memory occupied, when one has a lot
of concurrent decision diagrams alive. Yet, this does also happen at the cost of
having to read a much larger input. Only a practical evaluation can gauge
whether it is of benefit.

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

## References

- [[Arge10](https://ieeexplore.ieee.org/abstract/document/5470440)]
  Arge, Lars, Michael T. Goodrich, and Nodari Sitchinava. “*Parallel external
  memory graph algorithms*”. In: IEEE International Symposium on Parallel &
  Distributed Processing (IPDPS). (2010).

- [[Ochi93](https://www.computer.org/csdl/proceedings-article/iccad/1993/00580030/12OmNAXglQz)]
  Hiroyuki Ochi, Koichi Yasuoka, and Shuzo Yajima. “*Breadth-first manipulation
  of very large binary-decision diagrams*”. In: *Proceedings of 1993
  International Conference on Computer Aided Design (ICCAD),** (1993)

- [[Sanghavi96](https://link.springer.com/article/10.1007/s002360050083)
  Jagesh V. Sanghavi, Rajeev K. Ranjan, Robert K. Brayton, and Alberto
  Sangiovanni-Vincentelli. “*High performance BDD package by exploiting
  memory hierarchy*”. In: *Proceedings of the 33rd Annual Design
  Automation Conference* (1996)

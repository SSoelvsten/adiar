# Future Work
In the following we list multiple avenues for optimisations and extensions which
may constitute interesting undergraduate research projects.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Future Work](#future-work)
    - [Optimising the current algorithms](#optimising-the-current-algorithms)
        - [Non-comparison based sorting on numbers](#non-comparison-based-sorting-on-numbers)
        - [Layer-aware priority queue](#layer-aware-priority-queue)
        - [Parallelisation](#parallelisation)
            - [Parallel Layer-aware priority queue](#parallel-layer-aware-priority-queue)
            - [Reduction Rule 2 within the merge sorting](#reduction-rule-2-within-the-merge-sorting)
            - [Distribute nodes and information (in order) to worker-threads](#distribute-nodes-and-information-in-order-to-worker-threads)
    - [Optimising the OBDD](#optimising-the-obdd)
        - [Complement Edges](#complement-edges)
    - [Extensions](#extensions)
        - [Non-boolean Decision Diagrams](#non-boolean-decision-diagrams)
        - [Free Boolean Decision Diagrams](#free-boolean-decision-diagrams)

<!-- markdown-toc end -->


## Optimising the current algorithms
There are quite a few avenues of trying to shave off a few significant constants
in the running time on the current algorithms. Both suggestions below also make
the GPU an intriguing subject for a possible heavy improvement in the running
time.

### Non-comparison based sorting on numbers
The sorting in multiple variables has already been reduced to a simple sorting
on a single 64-bit key in the representation of nodes and arcs. It should be
possible to exploit this with a radix sort for an _O(N)_ time complexity, though
maybe one will not gain too much due to the _O(sort(N))_ I/O lower bound.


### Layer-aware priority queue

The primary bottleneck of the algorithms are the priority queues. All algorithms
(except for _Reduce_) both _push_ and _pop_ from their priority queue in a mixed
order. Yet, all algorithms are layer-by-layer and most elements that are pushed
are not to be used until after the whole layer has been dealt with. We may
postpone dealing with the pushed elements until after all computations are done
within the same layer.

The priority queue may either know the current layer being processed by being
told so by the main algorithm or just look into its internal data structures to
derive it. Knowing we are at layer _i_ we may then split the priority queue into
three different sub-structures.

1. **Layer i → Layer j > i+1**

    This is information we need in an arbitrary long time into the future, so we
    have to deal with these by an internal priority queue as before. We may
    choose to postpone pushing elements into it until all the next layer is
    reached by means of an intermediate FIFO queue.

2. **Layer i → Layer i+1**

    When information is pushed from layer _i_ to _i+1_ one may then just sort
    those nodes separately after all layer computations are done and then merge
    the two sorted lists implicitly when popping elements later.

    Assuming most requests are from layer _i_ to _i+1_, then based on
    [[Mølhave12](/README.md#references)] this will hopefully constitute a major
    improvement in the running time, since a priority queue is much slower at
    sorting elements than the `merge_sorter`.

3. **Layer i → Layer i**

    Since the priority queue is keeping track of which layer one is at, then
    when information is sent across to a later node on the same layer, then this
    can be dealt with by a second priority queue.

We may then _pop_ from our layer-aware priority queue by an implicit merge of
these three data structures above.

The third data structure only is needed for algorithms such as _Apply_, where
nodes are sent as _data_ across the layer. In the first two cases this field is
occupied merely by a dummy _NIL_ variable. That is, the first two data
structures do not need this field that takes up _3·64_ bits of data, and we can
make the total memory footprint much smaller and with it allow many more
elements to be closer to the processor.


### Parallelisation
The idea of Time-Forward-Processing is inherently sequential, but can we
parallelise parts of the algorithm? One may want to look into what possibilities
we have in parallel access to the underlying data structures and algorithms:

- Parallel traversal of the same file stream

- Parallelisable sorting algorithms (Sorting in TPIE is parallelised).
  
- Parallelisable Cache-oblivious priority queue, such as the one in
  [[Sitchinava12](#references)].

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


## Optimising the OBDD

### Complement Edges
Currently, we do not support complement edges, though one can expect about a 7%
factor decrease in the size of the OBDD from using said technique. In the
recursive algorithms, one can even expect a factor two decrease in the
algorithms execution time [[Brace90](#references)].


## Extensions

### Non-boolean Decision Diagrams
One can easily extend the proposed representation of sink nodes to encompass
non-boolean values, such as integers or floats. Thereby, the algorithms
immediately yield a Cache-oblivious implementation of the _Multi-Terminal Binary
Decision Diagrams_ (MTBDD) of [[Fujita97](#references)]. By solely using an
edge-based representation of the data-structure one can also implement a
_Multi-valued Decision Diagram_ (MDD) of [[Kam98](#references)]. The latter may
result in major rewrites of the algorithms.

### Free Boolean Decision Diagrams
One can remove the restriction of ordering the decision diagram to then
potentially compress the data structure even more. These Free Binary Decision
Diagrams (FBDD) of [[Meinel94](#references)] may also be possible to implement
in the setting of Time-forward processing used here.

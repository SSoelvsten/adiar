# Talent Track Project: Cache-oblivious OBDD
The goal is to implement I/O efficient algorithms to manipulate Ordered Boolean
Decision Diagrams, which is a fundamental data structure used in SAT Solvers,
Symbolic Model Checkers, and much more. During this project, they'll learn about
the data structure, the I/O model and the I/O technique called _Time Forward
Processing_.

- The data structure itself is never treated as part of the curriculum of any
  course.

- The I/O model and _Time Forward Processing_ is treated as part in the latter
  half of the Master's Course "_Computational Geometry_".


<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Talent Track Project: Cache-oblivious OBDD](#talent-track-project-cache-oblivious-obdd)
    - [Initial Reading](#initial-reading)
    - [Implementation](#implementation)
        - [Evaluate ★](#evaluate-)
        - [Restrict ★★](#restrict-)
        - [Count Paths ★★](#count-paths-)
        - [Apply ★★★](#apply-)
        - [Reduce ★★★](#reduce-)
    - [Theoretical Exercises](#theoretical-exercises)
    - [Statistics](#statistics)
    - [Solving problems with BDDs](#solving-problems-with-bdds)
        - [The N-Queens problem](#the-n-queens-problem)
        - [SAT Solver](#sat-solver)

<!-- markdown-toc end -->



## Initial Reading
The following material gives a basic understanding of the data structure.

- _Logic in Computer Science_:
  Huth and Ryan. page 358 - 390 (or at least to 374)

The following material gives a basic understanding of the I/O model and the
design of I/O algorithms.

- _I/O-Efficient Algorithms and Data Structures_:
    Lecture slides by Rolf Fagerberg:
  
    https://imada.sdu.dk/~rolf/Edu/DM207/E11/Slides/introduction.pdf
  
- _I/O Efficient Algorithms and Data Structures_:
    Lecture by Lars Arge.
  
    - Part 1+2 : https://www.youtube.com/watch?v=py4z_v9dfzQ
    - Part 3+4 : https://www.youtube.com/watch?v=KZua1GbIGr8

Extra reading for the interested:

- _Sorting - Upper and Lower bounds_:
    Lecture slides by Rolf Fagerberg:
  
    https://imada.sdu.dk/~rolf/Edu/DM207/E11/Slides/sorting.pdf

- _External Memory Geometric Data Structures_:
    Lars Arge.
  
    https://imada.sdu.dk/~rolf/Edu/DM808/F08/Handouts/ionotes.pdf

- _The input/output complexity of sorting and related problems_:
    Alok Aggarwal, Jeffrey S. Vitter.
  
    https://dl.acm.org/doi/10.1145/48529.48535

## Implementation
What implementation milestones to solve and in what order is chosen by the
students themselves. The order below reflects the expected difficulty from
lowest to highest.

### Evaluate ★
Reading:

- _COOM: Cache-oblivious OBDD Manipulation_: Steffan Sølvsten,
    - Start of section 3 (1.5 pages)
    - Section 4.3 and 4.4 (1 page)

Learning Outcome:

- Set up and get the project running
    - Using Git and Linux
- Understand the layout of our implementation of the data structure.
- TPIE:
    - file stream "iterator".


### Restrict ★★
Reading:

- _COOM: Cache-oblivious OBDD Manipulation_: Steffan Sølvsten,
    - Section 3.2 (1.5 pages)

Learning Outcome:

- Simple Time Forward Processing on a top-down algorithm.
- TPIE:
    - file stream "iterator".
    - Priority queue


### Count Paths ★★
Reading:

- No reading, so they have to come up with the solution themselves.

Learning Outcome:

- Simple Time Forward Processing on a `________` algorithm.
- TPIE:
    - file stream "iterator".
    - Priority queue


### Apply ★★★
Reading:

- _COOM: Cache-oblivious OBDD Manipulation_: Steffan Sølvsten,
    - Section 3.3 (3.5 pages)
    - Section 4.2 (0.2 pages)
  
Learning Outcome:

- More advanced data juggling with Time Forward Processing on a top-down
  algorithm.
    - Keeping the priority queue in sync with reading two separate file streams.
- TPIE:
    - file stream "iterator".
    - Priority queue


### Reduce ★★★
Reading:

- _COOM: Cache-oblivious OBDD Manipulation_: Steffan Sølvsten,
    - Section 3.1 (2.5 pages)
    - Section 4.1 (1 page)

Learning Outcome:

- Time Forward Processing (I/O Algorithm technique) on a bottom-up algorithm.
    - Handle more than one node at a time.
- TPIE:
    - Creating and managing temporary file streams
    - Merge Sorter and Priority Queue (and when to use which)


## Theoretical Exercises
There are a few open questions still on the theoretical side, which may inform
some details in the implementation.

- Is there a non-trivial worst-case upper bound on the number of elements in the
  priority queues?


## Statistics
Gather statistics about multiple facets of the algorithm. The design document
  (_COOM: Cache-oblivious OBDD Manipulation_: Steffan Sølvsten) contains
  multiple statistics worth exploring.

## Solving problems with BDDs
The OBDD data structure is commonly used to solve multiple problems. One may
want to try and implement these as _examples_ of using COOM. Then one can
benchmark how fast they can solve a problem for different inputs and input
sizes, to then maybe set up the same experiments on an installation of
[Sylvan](https://github.com/utwente-fmt/sylvan) to do competitive benchmarks.

### The N-Queens problem
One can find descriptions of how to solve the N-Queens Problem with an OBDD the
following places:

- https://github.com/MartinFaartoft/n-queens-bdd/blob/master/report.tex

That is, using the five algorithms above, one can solve the N-Queens problem.
For each level of recursion one uses an separate and unique OBDD.

One can find the supposed output for _n_ between 5 and 21
[here](http://www.ic-net.or.jp/home/takaken/e/queen/).


### SAT Solver
Based on [Integrating CNF and BDD Based SAT
Solvers](https://my.ece.utah.edu/~kalla/papers/hldvt03.pdf) provides an overview
of how to do a SAT Solver using BDDs. These algorithms also only use the five
above.

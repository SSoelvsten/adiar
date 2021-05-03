# Cite

To cite this project, please consider to cite the most appropriate papers listed
below, sorted by day of publication.

## [Efficient Binary Decision Diagram Manipulation in External Memory](https://arxiv.org/abs/2104.12101)

**Contents**

- Theoretical presentation of the techniques used
  - The underlying _total_ and _topological_ ordering
  - Use of both _node-based_ and _arc-based_ representations
  - Description of _Apply_ and _Reduce_, including input-dependant optimisations
    and proofs of their I/O and time complexities.
  - Description of all the other standard BDD algorithms
  - The _Levelized Priority Queue_
  - Memory layout of the unique identifier

- Presentation of _Adiar_
  - Overview on how to get started
  - `bdd` and `__bdd` objects, including garbage collection
  - List of all functions in
    [v1.0.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.0).

- Experimental Evaluation
  - Performance of _Adiar_ across the memory barrier
  - Performance improvements on top of Arge's work
  - Comparison with _Sylvan_ and _BuDDy_ BDD packages.

**BibTeX**

```
@Misc{Soelvsten:2021:arXiv,
  title         = {Efficient Binary Decision Diagram Manipulation in External Memory}, 
  author        = {Steffan Christ Sølvsten
               and Jaco van de Pol
               and Anna Blume Jakobsen
               and Mathias Weller Berg Thomasen},
  year          = {2021},
  eprint        = {2104.12101},
  archivePrefix = {arXiv},
  primaryClass  = {cs.DS},
}
```

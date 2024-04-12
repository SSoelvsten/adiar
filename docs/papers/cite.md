\page page__cite Citing this Project

Please consider to cite one or more of the following *relevant* papers, if you
use Adiar in some of your academic work. By relevant, we ask you to cite the
paper(s) that "generated" the cited knowledge.

[TOC]

Lars Arge
========================

Adiar is based on the theoretical work of Lars Arge back in the 90s.

- Lars Arge.
  “[*The I/O-complexity of Ordered Binary-Decision Diagram Manipulation*](https://link.springer.com/chapter/10.1007/BFb0015411)”.
  In: *International Symposium on Algorithms and Computation* (ISAAC). (1995)
  ```bibtex
  @InProceedings{arge1995:ISAAC,
    title     = {The {I/O}-complexity of {O}rdered {B}inary-{D}ecision {D}iagram manipulation},
    author    = {Arge, Lars},
    editor    = {Staples, John
             and Eades, Peter
             and Katoh, Naoki
             and Moffat, Alistair},
    booktitle = {Sixth International Symposium on Algorithms and Computation},
    year      = {1995},
    publisher = {Springer Berlin Heidelberg},
    address   = {Berlin, Heidelberg},
    series    = {Lecture Notes in Computer Science},
    volume    = {1004},
    pages     = {82--91},
    isbn      = {978-3-540-47766-2},
    doi       = {10.1007/BFb0015411},
  }
  ```

- Lars Arge.
  “[*The I/O-complexity of Ordered Binary-Decision Diagram Manipulation*](https://tidsskrift.dk/brics/issue/view/2576)”.
  In: *BRICS RS Preprint*. (1996)
  ```bibtex
  @InProceedings{arge1996:BRICS,
    author    = {Arge, Lars},
    title     = {The {I/O}-{C}omplexity of {O}rdered {B}inary-{D}ecision {D}iagram manipulation},
    booktitle = {BRICS RS preprint series},
    volume    = {29},
    year      = {1996},
    publisher = {Department of Computer Science, University of Aarhus},
    doi       = {10.7146/brics.v3i29.20010},
  }
  ```


Adiar
========================

We have, for your convenience, grouped all papers based on the semantic version
of Adiar. Depending on what feature of Adiar that you are referring to, then
pick the most relevant paper(s) to cite.

v1.0
------------------------

With [v1.0.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.0) (and its
patch [v1.0.1](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.1)) we
provide an implementation of Arge's algorithms and also add non-trivial
optimisations, extensions, and new theoretical contributions to make it usable
in practice.

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “[*Adiar: Binary Decision Diagrams in External Memory*](https://link.springer.com/chapter/10.1007/978-3-030-99527-0_16)”.
  In: *Tools and Algorithms for the Construction and Analysis of Systems* (TACAS). (2022)
  ```bibtex
  @InProceedings{soelvsten2022:TACAS,
    title         = {{A}diar: {B}inary {D}ecision {D}iagrams in {E}xternal {M}emory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2022},
    booktitle     = {Tools and Algorithms for the Construction and Analysis of Systems},
    editor        = {Fisman, Dana
                 and Rosu, Grigore},
    pages         = {295--313},
    numPages      = {19},
    publisher     = {Springer},
    series        = {Lecture Notes in Computer Science},
    volume        = {13244},
    isbn          = {978-3-030-99527-0},
    doi           = {10.1007/978-3-030-99527-0\_16},
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “[*Efficient Binary Decision Diagram Manipulation in External Memory*](https://arxiv.org/abs/2104.12101)”.
  In: *arXiv preprint*. (2021)
  ```bibtex
  @Misc{soelvsten2021:arXiv,
    title         = {{E}fficient {B}inary {D}ecision {D}iagram {M}anipulation in {E}xternal {M}emory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2021},
    archivePrefix = {arXiv},
    eprint        = {2104.12101},
    primaryClass  = {cs.DS},
    numPages      = {38},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2104.12101},
  }
  ```

v1.1
------------------------

With [v1.1.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.1.0) we add
support for *Zero-suppressed Decision Diagrams* to *Adiar*.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Adiar 1:1: Zero-suppressed Decision Diagrams in External Memory*](https://link.springer.com/chapter/10.1007/978-3-031-33170-1_28)”.
  In: *NASA Formal Methods* (NFM). (2023)
  ```bibtex
  @InProceedings{soelvsten2023:NFM,
    title         = {{A}diar 1.1: {Z}ero-suppressed {D}ecision {D}iagrams in {E}xternal {M}emory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2023},
    booktitle     = {NASA Formal Methods},
    editor        = {Rozier, Kristin Yvonne
                 and Chaudhuri, Swarat},
    pages         = {464--471},
    numPages      = {8},
    publisher     = {Springer},
    series        = {Lecture Notes in Computer Science},
    volume        = {13903},
    doi           = {10.1007/978-3-031-33170-1\_28},
  }
  ```

v1.2
------------------------

With [v1.2.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.0) (and its
patches [v1.2.1](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.1),
[v1.2.2](https://github.com/SSoelvsten/adiar/releases/tag/v1.2.2)) we introduce
the notion of an *i-level cut* of a graph and use these to derive sound upper
bounds on the size of the data structures. This drastically decreases the
threshold as to when *Adiar*'s running time is only a small constant factor
slower than other implementations.

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Predicting Memory Demands of BDD Operations Using Maximum Graph Cuts*](https://link.springer.com/chapter/10.1007/978-3-031-45332-8_4)”.
  In: *Automated Technology for Verification and Analysis* (ATVA). (2023)
  ```bibtex
  @InProceedings{soelvsten2023:ATVA,
    title     = {{P}redicting {M}emory {D}emands of {BDD} {O}perations {U}sing {M}aximum {G}raph {C}uts},
    author    = {S{\o}lvsten, Steffan Christ
             and van de Pol, Jaco},
    booktitle = {Automated Technology for Verification and Analysis},
    year      = {2023},
    editor    = {Andr{\'e}, {\'E}tienne
             and Sun, Jun},
    pages     = {72--92},
    numPages  = {21},
    publisher = {Springer},
    series    = {Lecture Notes in Computer Science},
    volume    = {14216},
    isbn      = {978-3-031-45332-8},
    doi       = {10.1007/978-3-031-45332-8\_4}
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol.
  “[*Predicting Memory Demands of BDD Operations Using Maximum Graph Cuts (Extended Paper)*](https://arxiv.org/abs/2307.04488)”.
  In: *arXiv preprint*. (2023)
  ```bibtex
  @Misc{soelvsten2023:arXiv,
    title         = {{P}redicting {M}emory {D}emands of {BDD} {O}perations using {M}aximum {G}raph {C}uts (Extended Paper)},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco},
    year          = {2023},
    archivePrefix = {arXiv},
    eprint        = {2307.04488},
    primaryClass  = {cs.DS},
    numPages      = {25},
    howPublished  = {arXiv},
    url           = {https://arxiv.org/abs/2307.04488},
  }
  ```

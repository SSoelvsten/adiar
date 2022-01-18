---
layout: default
title: Citing this project
nav_order: 7
description: "Citations"
permalink: /cite
---

# Citing this project
{: .no_toc }

Please consider to cite one or more of the following papers, if you use Adiar in
some of your academic work.
{: .fs-6 .fw-300 }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}


## Lars Arge

Adiar is based on the theoretical work of Lars Arge back in the 90s.

- Lars Arge.
  “[*The I/O-complexity of Ordered Binary-Decision Diagram Manipulation*](https://link.springer.com/chapter/10.1007/BFb0015411)”.
  In: *International Symposium on Algorithms and Computation* (ISAAC). (1995)

  ```bibtex
  @InProceedings{arge1995:ISAAC,
    title     = {The I/O-complexity of Ordered Binary-Decision Diagram manipulation},
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
    title     = {The I/O-Complexity of Ordered Binary-Decision Diagram},
    booktitle = {BRICS RS preprint series},
    volume    = {29},
    year      = {1996},
    publisher = {Department of Computer Science, University of Aarhus},
    doi       = {10.7146/brics.v3i29.20010},
  }
  ```


## Adiar

We have, for your convenience, grouped all papers based on the semantic version
of Adiar. Depending on what feature of Adiar that you are referring to, then
pick the most relevant paper(s) to cite.

### v1.0

With [v1.0.0](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.0) (and its
patch [v1.0.1](https://github.com/SSoelvsten/adiar/releases/tag/v1.0.1)) we
provide an implementation of Arge's algorithms and also add non-trivial
optimisations, extensions, and new theoretical contributions to make it usable
in practice.

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “*Adiar: Binary Decision Diagrams in External Memory*”.
  In: *Tools and Algorithms for the Construction and Analysis of Systems* (TACAS). (2022)
  
  ```bibtex
  @InProceedings{soelvsten2022:TACAS,
    title         = {Adiar: Binary Decision Diagrams in External Memory},
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2022},
    booktitle     = {Tools and Algorithms for the Construction and Analysis of Systems},
    editor        = {Dana Fisman
                 and Grigore Rosu},
    numPages      = {19},
    publisher     = {Springer},
    address       = {Berlin, Heidelberg},  
    series        = {Lecture Notes in Computer Science},
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “[*Efficient Binary Decision Diagram Manipulation in External Memory*](https://arxiv.org/abs/2104.12101)”.
  In: *arXiv preprint*. (2021)
  ```bibtex
  @Misc{soelvsten2021:arXiv,
    title         = {Efficient Binary Decision Diagram Manipulation in External Memory}, 
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

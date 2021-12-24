---
layout: default
title: Citing this project
nav_order: 6
description: "Citations"
permalink: /cite
---

# Citing Adiar
{: .no_toc }

Please consider to cite one or more of the following papers, if you use Adiar in
some of your academic work.
{: .fs-6 .fw-300 }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}


## Lars Arge

Adiar is based on the theoretical work of Lars Arge back in the 90s.

- Lars Arge.
  “[_The I/O-complexity of Ordered Binary-Decision Diagram Manipulation_](https://link.springer.com/chapter/10.1007/BFb0015411)”.
  In: _International Symposium on Algorithms and Computation_ (ISAAC). (1995)

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
    pages     = {82--91},
    isbn      = {978-3-540-47766-2}
  }
  ```

- Lars Arge.
  “[_The I/O-complexity of Ordered Binary-Decision Diagram Manipulation_](https://tidsskrift.dk/brics/issue/view/2576)”.
  In: _BRICS RS Preprint_. (1996)

  ```bibtex
  @InProceedings{arge1996:BRICS,
    author    = {Arge, Lars},
    title     = {The I/O-Complexity of Ordered Binary-Decision Diagram},
    booktitle = {BRICS RS preprint series},
    year      = {1996},
    publisher = {Department of Computer Science, University of Aarhus},
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
  “_Adiar: Binary Decision Diagrams in External Memory_”.
  In: _Tools and Algorithms for the Construction and Analysis of Systems_ (TACAS). (2022)
  
  ```bibtex
  @InProceedings{soelvsten2022:TACAS,
    title         = {Adiar: Binary Decision Diagrams in External Memory},
    author        = {S{\o}lvstxen, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2022},
    booktitle     = {Tools and Algorithms for the Construction and Analysis of Systems},
    numpages      = {20},
    publisher     = {Springer Berlin Heidelberg},
    address       = {Berlin, Heidelberg},
  }
  ```

- Steffan Christ Sølvsten, Jaco van de Pol, Anna Blume Jakobsen, Mathias Weller Berg Thomasen.
  “_[Efficient Binary Decision Diagram Manipulation in External Memory](https://arxiv.org/abs/2104.12101)_”.
  In: _arXiv preprint_. (2021)
  ```bibtex
  @Misc{soelvsten2021:arXiv,
    title         = {Efficient Binary Decision Diagram Manipulation in External Memory}, 
    author        = {S{\o}lvsten, Steffan Christ
                 and van de Pol, Jaco
                 and Jakobsen, Anna Blume
                 and Thomasen, Mathias Weller Berg},
    year          = {2021},
    eprint        = {2104.12101},
    archivePrefix = {arXiv},
    primaryClass  = {cs.DS}
    numpages      = {36},
  }
  ```

---
layout: default
title: Memory Mode
nav_order: 2
parent: Settings
description: "Memory Mode of Auxiliary Data Structures"
permalink: settings/memory_mode
---

# Memory Mode
{: .no_toc }

*Adiar*'s Algorithms delay recursion through use of auxiliary data structures.
These auxiliary data structures can be optimised for *internal* memory, and so
have a high performance on very small instances, or they can be designed for
*external* memory such that they can handle decision diagrams much larger than
the available memory.
{: .fs-6 .fw-300 }

What type of auxiliary data structure is used is encapsulated in the following
`memory_mode` enum in the `adiar::memory` namespace which can have one of the
following three values

| Enum Value | Effect                                         |
|------------|------------------------------------------------|
| `AUTO`     | Automatically decide on type of memory to use. |
| `INTERNAL` | Always use *internal* memory.                  |
| `EXTERNAL` | Always use *external* memory.                  |

If you want to force *Adiar* to a specific memory mode then you should set the
global variable `adiar::memory::mode` to one of the above three values. For
example, one can force *Adiar* always use internal memory with the
following piece of code.
```cpp
adiar::memory::mode = adiar::memory::INTERNAL
```

| ! WARNING !                                                                  |
|:----------------------------------------------------------------------------:|
| **Using `INTERNAL` may lead to crashes if an input or output is too large!** |

By default `adiar::memory::mode` is set to `AUTO`. That is, *Adiar* decides
whether to use internal or external memory for each algorithm based on meta
information about the inputs.


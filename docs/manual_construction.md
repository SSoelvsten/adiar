---
layout: default
title: Manual Construction
nav_order: 4
description: "Manual Construction of BDDs and ZDDs"
permalink: /manual_construction
has_children: true
---

# Manual Construction
{: .no_toc }

In some cases, one may already know the shape of the
[BDDs](data_structures/bdd.md) and/or as [ZDDs](data_structures/zdd.md) for a
more complex function. In those cases, it is much cheaper to construct them by
hand than to manipulate logic formulas
{: .fs-6 .fw-300 }

To this end, one can construct these decision diagrams bottom-up in *Adiar* by
use of the provided [builders](manual_construction/builder.md). Expert users
that need a few more ounces of performance out of such a manual construction
constructions, on the other hand, may want to
[write](manual_construction/node_writer.md) directly to the raw files
underneath.

---
layout: default
title: Statistics
nav_order: 4
description: "Gathering statistics"
permalink: /statistics
---

# Statistics
{: .no_toc }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

## Activating Gathering of Statistics
Statistics are by default **not** gathered due to a concern of the performance
of *Adiar*. That is, the logic related to updating the statistics is only run
when one or more of the following *CMake* variables are set to *ON*.

- `ADIAR_STATS`

  Only gather statistics that introduce a small constant time overhead to every
  operation.

- `ADIAR_STATS_EXTRA`

  Also gathers statistics that introduce a linear-time overhead to all
  operations.

## Obtaining Statistics

### `stats_t adiar_stats()`

Obtain a copy of the raw data values. To see all available values in *stats_t*,
please consult its declaration in *<adiar/statistics.h>*.

### `void adiar_printstat(std::ostream os)`

Pretty prints the statistics in `adiar_stats()` to the output stream *os*.

### `void adiar_printstat()`

Pretty prints the statistics in `adiar_stats()` to the standard output.

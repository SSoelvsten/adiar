---
layout: default
title: Domain
nav_order: 1
parent: Settings
description: "Domain of All Variables"
permalink: settings/domain
---

# Domain
{: .no_toc }

Some operations depend on a domain of all variables for the problem being
modelled. For simplicity, this can be set once.
 {: .fs-6 .fw-300 }

---

### `void adiar_set_domain(label_file dom)`
{: .no_toc }

Sets the global domain of variables to the
[labels](../data_structures/labels_and_assignments#labels) in the given
[`label_file`](../data_structures/labels_and_assignments.md#files).

### `bool adiar_has_domain()`
{: .no_toc }

Whether a global domain already is set.

### `label_file adiar_get_domain()`
{: .no_toc }

Retrieve the [`label_file`](../data_structures/labels_and_assignments.md#files)
that represents the current domain.

\page page__basic Basic Usage

[TOC]

Creating BDDs
===============================

Constants
-------------------------------

The Boolean constants *true* and *false* can be obtained directly with the
`adiar::bdd_true` and `adiar::bdd_false` functions.

```cpp
adiar::bdd top = adiar::bdd_true();
```

Alternatively, boolean constants can also be created directly from a value of
type `bool`.

```cpp
adiar::bdd bot = false;
```

Variables
-------------------------------

For a single variable, e.g. x<sub>i</sub>, parse the label *i* to
`adiar::bdd_ithvar`.

```cpp
adiar::bdd x0 = adiar::bdd_ithvar(0);
adiar::bdd x1 = adiar::bdd_ithvar(1);
adiar::bdd x2 = adiar::bdd_ithvar(2);
```

For the negation of a single variable, use `adiar::bdd_nithvar`

Combining BDDs
===============================

Negation
-------------------------------

Use `adiar::bdd_not` or the `~` operator. For example, `res` below is equivalent
to `adiar::bdd_nithvar` of *0*.

```cpp
adiar::bdd res = adiar::bdd_not(x0);
```

```cpp
adiar::bdd res = ~x0;
```

Binary Operators
-------------------------------

Use `adiar::bdd_apply` with an `adiar::bool_op`.

```cpp
adiar::bdd f = adiar::bdd_apply(x0, x1, adiar::xor_op);
```

Alternatively, there is a specializations of `adiar::bdd_apply` for each
`adiar::bool_op` and for *and*, *or*, and *xor* one can also use the `&`, `|`,
and `^` operators.

```cpp
adiar::bdd f = adiar::bdd_xor(x0, x1);
```
```cpp
adiar::bdd f = x0 ^ x1;
```
```cpp
adiar::bdd f = bot;
f ^= x0;
f ^= x1;
```

If-Then-Else Operator
-------------------------------

Use `adiar::bdd_ite`.

```cpp
adiar::bdd _ = adiar::bdd_ite(x0, x1, x2);
```

In other BDD packages, this function is good for manually constructing a BDD
bottom-up. But, here you should use `adiar::bdd_builder` instead (see \ref
page__builder).

Restricting Variables
-------------------------------

Use `adiar::bdd_restrict`. For example, `res` below is equivalent to
`~x1`.

```cpp
adiar::bdd _ = adiar::bdd_restrict(f, i, true);
```

Specifically to restrict the *top* variable, you can use `adiar::bdd_low` and
`adiar::bdd_high`.

See also the \ref page__functional tutorial for better ways to use this
operation.

Quantification
-------------------------------

Use `adiar::bdd_exists` and `adiar::bdd_forall`. For example, `res` below is
equivalent to `adiar::bdd_true()`.

```cpp
adiar::bdd _ = adiar::bdd_exists(f, i);
```

See also the \ref page__functional tutorial for better ways to use these
operations.

Satisfying Assignments
===============================

To get the number of satisfying assignments, use `adiar::bdd_satcount`. Its
second (optional) argument is the total number of variables (including the
possibly suppressed ones in the BDD).

```cpp
size_t varcount = 3;
size_t _ = adiar::bdd_satcount(f, varcount); // 4
```

To get a cube of the *lexicographical minimal* or *maximal* assignment, use
`adiar::bdd_satmin` and `adiar::bdd_satmax` respectively.

```cpp
adiar::bdd f_min = adiar::bdd_satmin(f); // ~i & j
adiar::bdd f_max = adiar::bdd_satmax(f); // i & ~j
```

Predicates
===============================

Boolean Constants
-------------------------------

Use `adiar::bdd_isconst` to check whether a BDD is a constant Boolean value and
`adiar::bdd_istrue` and `adiar::bdd_isfalse` to check for a specific boolean
value.

```cpp
bool _ = adiar::bdd_isconst(top); // true
bool _ = adiar::bdd_isconst(x0);  // false
```

```cpp
bool _ = adiar::bdd_istrue(top);  // true
bool _ = adiar::bdd_istrue(x0);   // false
```

```cpp
bool _ = adiar::bdd_isfalse(top); // false
bool _ = adiar::bdd_isfalse(bot); // true
```

Single Variables
-------------------------------

Use `adiar::bdd_isvar`, `adiar::bdd_isithvar`, and `adiar::bdd_isnithvar` to
check whether a BDD represents a formula of exactly one variable.

```cpp
bool _ = adiar::bdd_isvar(top);     // false
bool _ = adiar::bdd_isvar(x0);      // true
bool _ = adiar::bdd_isvar(~x0);     // true
```

```cpp
bool _ = adiar::bdd_isithvar(top);  // false
bool _ = adiar::bdd_isithvar(x0);   // true
bool _ = adiar::bdd_isithvar(~x0);  // false
```

```cpp
bool _ = adiar::bdd_isnithvar(top); // false
bool _ = adiar::bdd_isnithvar(x0);  // false
bool _ = adiar::bdd_isnithvar(~x0); // true
```

Cubes
-------------------------------

To check whether a BDD represents a cube, i.e. where all the variables in its
*support* have a fixed value, use `adiar::bdd_iscube`.

```cpp
bool _ = adiar::bdd_iscube(f);     // false
bool _ = adiar::bdd_iscube(f_min); // true
```

Equality
-------------------------------

Use `adiar::bdd_equal` and `adiar::bdd_unequal` or the `==` and `!=` operators.

```cpp
adiar::bdd f2 = x1 ^ x0;

bool _ = adiar::bdd_equal(f,f2); // true
bool _ = f == f2;                // true
```

BDD Information
===============================

Counting Operations
-------------------------------

Use `adiar::bdd_nodecount` to get the number of BDD nodes.

```cpp
size_t _ = adiar::bdd_nodecount(f); // 3
```

Use `adiar::bdd_varcount` to get the number of variables present in the BDD.

```cpp
size_t _ = adiar::bdd_varcount(f); // 2
```

Use `adiar::bdd_pathcount` to get the number of paths to *true*.

```cpp
size_t _ = adiar::bdd_pathcount(f); // 2
```

Support
-------------------------------

The *top*, *minimal*, and *maximal* variable label is can be obtained with
`adiar::bdd_topvar`, `adiar::bdd_minvar`, and `adiar::bdd_maxvar`, respectively.

```cpp
adiar::bdd::label_type _ = adiar::bdd_topvar(f); // i
adiar::bdd::label_type _ = adiar::bdd_minvar(f); // i
adiar::bdd::label_type _ = adiar::bdd_maxvar(f); // j
```

Graphical Output
-------------------------------

The BDD can be exported to the *DOT* format with `adiar::bdd_printdot`. The
second argument can either be an output stream, e.g. `std::cout`, or a filename.

```cpp
adiar::bdd_printdot(f, "./f.dot");
```

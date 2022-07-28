---
layout: default
title: Labels and Assignments
nav_order: 3
parent: Data Structures
description: "Labels and Assignments"
permalink: data_structures/labels_and_assignments
---

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Labels

An input variable *x<sub>i</sub>* is identified by its label *i* which in
*Adiar* is defined as the following type.

```cpp
typedef uint32_t label_t;
```

Please note, that while `label_t` is an unsigned 32-bit integer, not all bits
are used. Specifically, *Adiar* does not support any labels larger than the
`MAX_LABEL` value (*2<sup>24</sup>-1*).

### Example

The set of all even labels between *0* and *42* (inclusive) is constructed with
the following piece of code.
```cpp
label_file labels;

{ // Garbage collect writer to free write-lock
  label_writer lw(labels);
  
  for (label_t l = 0; l <= 42; l = l+2) {
    lw << l;
  }
}
```
and it can then be read again as follows
```cpp
label_stream ls(labels);

while (ls.can_pull()) {
  std::cout << "  " << ls.pull() << std::endl;
}
```
For more details on the `label_file`, `label_stream`, and
`label_writer` see the section on [Files](#files) below.

## Assignments

An assignment maps a *label* to a boolean *value* and so they can be represented
as a pairing of the two.

```cpp
struct assignment { label_t label; bool value; };

typedef assignment assignment_t
```

The operators `<` and `>` are defined such that they enforce labels are ordered
ascendingly. The `==`, and `!=` operators check both values. Finally, the `!`
operator negates the *value* and leaves the *label* unchanged.

### `assignment_t create_assignment(label_t label, bool value)`

Create an assignment given a label and value.

### `label_t label_of(assignment_t assignment)`
{: .no_toc }

Obtain the label of the variable to-be assigned a value.

### `bool value_of(assignment_t assignment)`
{: .no_toc }

Obtain the value to-be assigned to the variable with the label.

### Example 1: Encoding a Total Assignment into a Function

Consider the following truth assignment

| Variable      | Value |
|:-------------:|:-----:|
| x<sub>0</sub> | ⊤     |
| x<sub>1</sub> | ⊥     |
| x<sub>2</sub> | ⊥     |
| x<sub>3</sub> | ⊤     |

Assuming the domain is only [0; 3] then we can encode it with the following
lambda function.

```cpp
assignment_func assignment = [](label_t i) { return i == 0 || i == 3 };
```

### Example 2: Encoding a Partial Assignment into a File

One can encode the very same truth table above in `assignment_file` by use of
the `assignment_writer` shown below.
```cpp
assignment_file assignment;

{ // Garbage collect writer to free write-lock
  assignment_writer aw(assignment);

  aw << create_assignment(0, true )
     << create_assignment(1, false)
     << create_assignment(2, false)
     << create_assignment(3, true )
  ;
}
```
Notice, the scope around the `assignment_writer` to detach it early. One can
then read this assignment and print out the above table as follows
```cpp
assignment_stream<> as(assignment);
  
std::cout << "| var | value |" << std::endl;
std::cout << "|-----|-------|" << std::endl;

while (as.can_pull()) {
  assignment_t a = as.pull();
  std::cout << "| x" << a.label << "  | " << a.value << "     |" << std::endl;
}
```
For more details on the `assignment_file`, `assignment_stream`, and
`assignment_writer` see the section on [Files](#files) below.

## Files

Both a `label_file` and an `assignment_file` are instances of a `simple_file<T>`
which one can write to the end of and can read in either direction using their
respective *writer* and *stream* classes.

---

### `file_stream<typename T, bool REVERSE>` class

This templated class provides a read-only access to a file with a single
*reading direction*. If the boolean `REVERSE` is set to *true* (default
*false*), then one reads the file in the reverse direction.

---

### Constructors
{: .no_toc }

### `file_stream<T, REVERSE>::file_stream<>()`
{: .no_toc }

Construct the stream class, attached to nothing.

### `file_stream<T, REVERSE>::file_stream<>(const simple_file<T>)`
{: .no_toc }

Construct the stream and have it attach to the given `simple_file`.

---

### Member Functions
{: .no_toc }

and it has the following member functions.

### `bool file_stream<T, REVERSE>::can_pull()`
{: .no_toc }

Whether there is an element to pull in the desired direction.

### `T file_stream<T, REVERSE>::pull()`
{: .no_toc }

Get the element from the stream in the desired direction and move to the next.

### `T file_stream<T, REVERSE>::peek()`
{: .no_toc }

Get the element from the stream in the desired direction without moving to the
next.

### `void file_stream<T, REVERSE>::reset()`
{: .no_toc }

Reset the stream back to its beginning of the current file. If `reverse` is
true, then it resets back to the end.

### `void file_stream<T, REVERSE>::attach(simple_file<T> f)`
{: .no_toc }

Attach the stream to the given file.

### `bool file_stream<T, REVERSE>::attached()`
{: .no_toc }

Whether the stream is attached to a file.

### `void file_stream<T, REVERSE>::detach()`
{: .no_toc }

Detach the stream from its current file, if any.

---

### `simple_file_writer<typename T, typename Comp>` class

This allows one to push elements to the end of a file. The `Comp` template
argument is a comparator to enforce some desired structure of the file. This is
not important for the `label_writer` (`no_ordering<T>`), but for the
`assignment_writer` this enforces an ascending order
(`std::less<assignment_t>`).

---

### Constructors
{: .no_toc }

### `simple_file_writer<T, Comp>::simple_file_writer<>()`
{: .no_toc }

Construct it attached to nothing.

### `simple_file_writer<T, Comp>::simple_file_writer<>(const simple_file<T>)`
{: .no_toc }

Construct it attached to the given *simple_file*.

---

### Member Functions
{: .no_toc }

### `void simple_file_writer<T, Comp>::push(T t)` (operator `<<`)
{: .no_toc }

Push an *element* to the end of the file and check it is provided in-order.

### `void simple_file_writer<T, Comp>::unsafe_push(T t)`
{: .no_toc }

Push an *element* to the end of the file.

### `void simple_file_writer<T, Comp>::sort()`
{: .no_toc }

Sort the content of the attached file to be in the correct order based on
`Comp`. This is not useful in a `label_writer` but in the `assignment_writer`
this can be used to repair .

### `void simple_file_writer<T, Comp>::attach(simple_file<T> f)`
{: .no_toc }

Attach the writer to the given file.

### `bool simple_file_writer<T, Comp>::attached()`
{: .no_toc }

Whether the writer is attached to a file.

### `void simple_file_writer<T, Comp>::detach()`
{: .no_toc }

Detach the writer from its current file, if any.


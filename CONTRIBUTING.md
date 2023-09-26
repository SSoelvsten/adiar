# Adiar Contribution Guidelines

Thank you for spending your valuable time contributing to this research project!
Below you find information relevant for helping you to get started with
contributing to Adiar.

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Adiar Contribution Guidelines](#adiar-contribution-guidelines)
    - [New to Git and Programming?](#new-to-git-and-programming)
    - [Getting Started](#getting-started)
    - [Git Flow](#git-flow)
        - [Commits and Messages](#commits-and-messages)
        - [Linear History](#linear-history)
        - [Branch Names](#branch-names)
    - [Code Formatting Guidelines](#code-formatting-guidelines)
    - [Design Principles](#design-principles)
        - [§1 Functional-First](#1-functional-first)
            - [§1.1 Public API is Functional](#11-public-api-is-functional)
            - [§1.2 Data Types are Immutable](#12-data-types-are-immutable)
            - [§1.3 Imperative Exceptions](#13-imperative-exceptions)
        - [§2 Run-time over Compilation-time](#2-run-time-over-compilation-time)
            - [§2.1 Do Not Expose Templates](#21-do-not-expose-templates)
        - [§3 Naming Scheme](#3-naming-scheme)
        - [§4 No Almost Always Auto!](#4-no-almost-always-auto)
        - [§5 Documentation Comments Everywhere](#5-documentation-comments-everywhere)
        - [§6 Test Everything Thoroughly](#6-test-everything-thoroughly)

<!-- markdown-toc end -->

## New to Git and Programming?

If you are new to using Git, please read the following by GitHub

- [Finding ways to contribute to open source on GitHub](https://docs.github.com/en/get-started/exploring-projects-on-github/finding-ways-to-contribute-to-open-source-on-github)
- [Set up Git](https://docs.github.com/en/get-started/quickstart/set-up-git)
- [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow)
- [Collaborating with pull requests](https://docs.github.com/en/github/collaborating-with-pull-requests)


## Getting Started

See the [README](./README.md) for information on all dependencies and how to
build *Adiar* and run its unit tests.

### Visual Studio Code

If you use Visual Studio Code as your editor, please have the following three
*extensions* installed.

| Name                         | ID                        |
|------------------------------|---------------------------|
| **CMake Tools**              | ms-vscode.cmake-tools     |
| **C/C++**                    | ms-vscode.cpptools        |
| **EditorConfig for VS Code** | EditorConfig.EditorConfig |

The following two are also recommended:

| Name                  | ID                           |
|-----------------------|------------------------------|
| **C/C++ Themes**      | ms-vscode.cpptools-themes    |
| **Better C++ Syntax** | jeff-hykin.better-cpp-syntax |

## Git Flow

### Commits and Messages

- Commit Messages are written in imperative, their first word is capitalized,
  and they are written in a way that is pretty in ASCII (not only Markdown),
  e.g. "*Turn 'tuple' into a class*".
- Only **one** change per commit, i.e. a commit message that includes the word
  "and" should be avoided as much as is possible.
- Unit tests for a feature are included in the commit that adds said feature.
  They do not need to be mentioned as part of the commit message.
- Each commit should be possible to build and run *test/* and *src/main.cpp*.
  If you forgot to stage a change or introduced a bug, please *rebase* your
  branch and add the *fixup*.

### Linear History

The *main* branch is kept with a linear history. Hence, all pull requests are
always *rebased* on-top of main.

### Branch Names

If you are a collaborator on the main repository (*github.com/SSoelvsten/adiar/*),
your branches should try to use the following prefixes in their name.

- **bug/\***:
  Branches that resolve a bug.

- **bdd/\***, **internal/\***, **statistics/\*** **zdd/\***:
  Feature branches are prefixed with the primary area of contribution.

- **docs/\***:
  Changes, fixes and additions to the documentation, i.e. Markdown and
  Doxygen files and documentation comments in the *src/* folder.

## Code Formatting Guidelines

Most of Adiar has been developed with the
[Spacemacs](https://www.spacemacs.org/) extension of the Emacs editor. Hence,
code in Adiar is indented as dictated by Emacs (version 28 or later).

The most basic parts of these formatting rules are also reflected in the
*editorconfig* that most editors can be set up to follow.

## Design Principles

### §1 Functional-First

The algorithms of Adiar lend themselves very well to a functional style: if the
priority queues are abstracted to ordered sets then the algorithm can be
rephrased as very simple tail-recursive operations on lists. This has the vital
benefit of decreasing the complexity of the code base and with it the number of
errors.

Hence, all code should be written in a functional style insofar it does not
negatively impact performance. We trust the compiler to find the last few
optimisations.

#### §1.1 Public API is Functional

- Member access (and its derived state) are available through functions.
- Manipulation of decision diagrams are handled through functions that have no
  (observable) side-effects on the input.

#### §1.2 Data Types are Immutable

The internal data types, e.g. *ptr* and *node*, are all treated as immutable
values and should if possible be marked as `const`.

- Member access (and its derived state) are available through member functions.
- Manipulation of data types are handled through functions that have no
  side-effects and instead return a new separate instance of the changed object.

#### §1.3 Imperative Exceptions

For the sake of performance, the only exception to this principle is the
interaction with (1) files, (2) data structures, e.g. the priority queues and the
input streams, and (3) the call stack.

### §2 Run-time over Compilation-time

A primary goal of Adiar is high-performance computing. Hence, we want to
sacrifice the compilation time in favour of improving the running time, e.g.
polymorphism is achieved with templates rather than virtual functions.

Yet, the code should still be kept maintainable, i.e. with separated
responsibilities and without any code duplication. Hence, the code base ought to
be modularised with a [policy-based
design](https://en.wikipedia.org/wiki/Modern_C%2B%2B_Design#Policy-based_design).

#### §2.1 Do Not Expose Templates

This principle leads to the creation of templates only intended for use within
Adiar. None of these should (if possible) be exposed in a header file to the end
user, but instead be compiled into a non-templated function within a relevant
CPP source file.

### §3 Naming Scheme

The codebase uses [snake case](https://en.wikipedia.org/wiki/Snake_case) as much
as possible. This nicely aligns Adiar's codebase with the *std* (and the
*tpie*) namespace that the developers are used to anyway. Furthermore, this way
of writing might be easier to read.

- For *namespaces*, *classes*, *variables*, and *functions* use `snake_case`.
  The only other exceptions are *enum* values and the public API (see below).

  - Preprocessing variables use *SNAKE_CASE*.

  - We cannot use *snake_case* or *SNAKE_CASE* for enum values since that may
    clash with keywords or preprocessing variables. Hence, we have settled on
    `Snake_Case`.

  - Yet, in the public *BDD* API we run into some problems. What we really want
    is to have the `bdd` and `zdd` "class" prefix all functions as a namespace,
    e.g. we would write `adiar::bdd h = adiar::bdd::and(f,g)`. Yet we cannot do
    so since `and`, `or`, and `xor` are keywords in C++.

    So we need some alternative. Yet, each is a compromise in some way.
    Currently, we have chosen to stick with a naming scheme that aligns with the
    C API of *BuDDy* and *Sylvan* and does not clash too much with the use of
    *snake_case*:

    All functions of the public API are of the form `{prefix}_{functionname}`,
    e.g. `bdd_and(f,g)` by prepending `bdd_` onto the `and(f,g)` function name.
    Functions names that consist of multiple words, e.g. *is true*, is written
    in *nocase*, e.g. `bdd_istrue(f)`.

- Type variables (*kinds*) are suffixed with `_type` (preferably the latter).
  Template type parameters are suffixed with `_t` such that it can be reexposed
  with the `_type` suffix without creating a name clash.

- Private class member variables are prefixed with a single `_`; public ones may
  also be prefixed as such.

### §4 No Almost Always Auto!

Some C++ developers prefer to use the `auto` keyword as much as possible to let
the compiler derive the type. Yet, we want to **not** do so such that (1) the
derived type is as intended and (2) to convey the programmer's intent to the
reader and (3) to provide all the information necessary to debug the code.

### §5 Documentation Comments Everywhere

As Adiar grows larger and more complex, one submodule is the foundation on which
others are built. Hence, **all** functions, data structures, classes, and
variables - even the ones in *adiar::internal* that are not part of the public
API - should be well documented for both the *end users* and especially also the
*developers*.

### §6 Test Everything Thoroughly

Adiar's Decision Diagrams are to be used in the context of verification of
critical software. At the same time, Adiar's algorithms are much more complex
than other BDD implementations and have multiple layers from which an error
could originate.

Hence, it is vital we can ensure correctness of Adiar by having as thorough unit
testing as possible. This also applies to everything within *adiar::internal*
that is not part of the public API.

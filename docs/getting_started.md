---
layout: default
title: Getting started
nav_order: 2
description: "The dependencies and installation of _Adiar_ and how to initialise it."
permalink: /getting_started
---

# Getting started
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

## Dependencies
One needs a C++ compiler of ones choice that supports the _17_ standard. All
development has been done with the gcc compiler, so we recommend one to use the
same. The project also has dependencies on the TPIE library, which itself has
dependencies on the Boost Library.

On Ubuntu 20+ you can obtain these dependencies with the following command.

```bash
apt install g++ libboost-all-dev
```

## Building with CMake
The project is built with _CMake_.

```bash
apt install cmake
```

To get started with using _Adiar_, you need to place the repository somewhere
locally. The simplest way to do so is to add it as a submodule inside of your
Git repository.

```bash
git submodule add https://github.com/SSoelvsten/adiar external/adiar
git submodule update --init --recursive
```

Then include the following line in your project's _CMakeLists.txt_.

```cmake
add_subdirectory (external/adiar adiar)
```

Finally, every single executable target is linked _Adiar_ in the
_CMakeLists.txt_ with the following lines.

```cmake
add_executable(<target> <source>)
target_link_libraries(<target> adiar)
set_target_properties(<target> PROPERTIES CXX_STANDARD 17)
```

You only need to include the third line if the `CXX_STANDARD` has not been set
project-wide to 17 or higher.

## Usage
After having linked the C++ source file with _Adiar_ as described above, then
one needs to include the `<adiar/adiar.h>` header, initialise the library before
use, and remember to deinitialise the library again before the program
terminates.

```c++
#include <adiar/adiar.h>

int main()
{
  adiar::adiar_init(128);

  {
    // do your stuff here...
  }
  
  adiar::adiar_deinit();
}
```

The `adiar_init` function initialises the BDD library given the following arguments

- `memory_limit_mb`

  The amount of internal memory in MiB that the _Adiar_ BDD library is allowed
  to use.

- `temp_dir` (optional)

  The directory in which to place all temporary files of _Adiar_. Default on
  Linux is the `/tmp` library.

If you create any [bdd](/bdd.md) objects then remember to have them garbage
collected (for example, by letting a local variable go out of scope as shown
above) before calling `adiar::adiar_deinit()`.

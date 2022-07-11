---
layout: default
title: Getting started
nav_order: 2
description: "The dependencies and installation of *Adiar* and how to initialise it."
permalink: /getting_started
---

# Getting started
{: .no_toc }

## Table of Contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Dependencies
One needs a C++ compiler of ones choice that supports the *17* standard. All
development has been done with the gcc compiler, so we recommend one to use the
same. The project also has dependencies on the TPIE library, which itself has
dependencies on the Boost Library. Finally, the entire project is built with
*CMake*.

To install all of the above then run the respective below.

| Operating System | Shell command                                |
|------------------|----------------------------------------------|
| Ubuntu 18+       | `apt install cmake g++     libboost-all-dev` |
| Fedora 35+       | `dnf install cmake gcc-c++ boost-devel`      |


## Building with CMake

To get started with *Adiar*, you need to place the repository somewhere on your
machine. The simplest way to do so is to add it as a submodule inside of your
Git repository.

```bash
git submodule add https://github.com/SSoelvsten/adiar external/adiar
git submodule update --init --recursive
```

Then include the following line in your project's *CMakeLists.txt*.

```cmake
add_subdirectory (external/adiar adiar)
```

Finally, every single executable target is linked to *Adiar* in the
*CMakeLists.txt* file with the following lines.

```cmake
add_executable(<target> <source>)
target_link_libraries(<target> adiar)
```

## Usage
After having linked the C++ source file with *Adiar* as described above, then
one needs to include the `<adiar/adiar.h>` header, initialise the library before
use, and remember to deinitialise the library again before the program
terminates.

```cpp
#include <adiar/adiar.h>

int main()
{
  adiar::adiar_init(128 * 1024 * 1024);

  {
    // do your stuff here...
  }

  adiar::adiar_deinit();
}
```

The `adiar_init` function initialises the BDD library given the following arguments

- `memory_limit_bytes`

  The amount of internal memory (in bytes) that *Adiar* is allowed to use.

- `temp_dir` (optional)

  The directory in which to place all temporary files. Default on Linux is the
  `/tmp` library.

If you create any [bdd](bdd.md) or [zdd](zdd.md) objects then remember to have
them garbage collected (for example, by letting a local variable go out of scope
as shown above) before calling `adiar::adiar_deinit()`.

By default *Adiar* decides whether to use internal or external memory for each
algorithm based on the size of the inputs. However, if you want to force using
always internal or external memory set the global variable `adiar::memory::mode`
to `adiar::memory::INTERNAL` or `adiar::memory::EXTERNAL` (default is
`adiar::memory::AUTO`). Setting `adiar::memory::mode` to `adiar::memory::INTERNAL`
is at your own risk and may crash if the input is too large.

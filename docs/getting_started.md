\page page__getting_started Getting started

[TOC]

Dependencies
=========================

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


Building with CMake
=========================

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

Usage
=========================

After having linked the C++ source file with *Adiar* as described above, then
one needs to include the `<adiar/adiar.h>` header, initialise the library before
use, and finally remember to deinitialise the library again before the program
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

In between, you can create and manipulate the \ref module__bdd and \ref
module__zdd decision diagrams.

\warning Before calling `adiar::adiar_deinit()` you have destruct all of Adiar's
objects that you own, e.g. by letting them go out of scope with the `{` and `}`
braces shown above.

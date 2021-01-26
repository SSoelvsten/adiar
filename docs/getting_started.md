# Getting Started

**Table of Contents**

- [Dependencies](#dependencies)
- [Building with CMake](#building-with-cmake)
- [Usage](#usage)

## Dependencies
One needs a C++ compiler of ones choice. All development has been done with the
gcc compiler, so we recommend one to use the same.

```bash
apt install g++
```

The project also has dependencies on the TPIE library, which itself has
dependencies on the Boost Library. You can on Ubuntu 18+ obtain these
dependencies with the following commands.

```bash
apt install libboost-all-dev
```

## Building with CMake
The project is built with _CMake_.

```bash
apt install cmake
```

To get started with using _Adiar_, you need to place the repository somewhere
within your project. This can neatly be done by adding it as a git submodule.

```bash
git submodule add https://github.com/SSoelvsten/adiar external/adiar
git submodule update --init --recursive
```

Then include the following line in your project's _CMakeLists.txt_ the following
line.

```cmake
add_subdirectory (external/adiar adiar)
```

Finally, every single executable target can then be linked _Adiar_ in a
_CMakeLists.txt_ as follows.

```cmake
add_executable(<target> <source>)
target_link_libraries(<target> adiar)
set_target_properties(<target> PROPERTIES CXX_STANDARD 17)
```

The last line is only necessary, if the `CXX_STANDARD` hasn't been set to 17 or
higher project-wide.

## Usage
After having linked up the C++ source file with _Adiar_ as described above, then
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

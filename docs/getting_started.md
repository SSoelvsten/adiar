# Getting Started

## Dependencies
One needs a C++ compiler of ones choice. All development has been done with the
gcc compiler, so we recommend one to use the same.

```bash
sudo apt install g++
```

The project also has dependencies on the TPIE library, which itself has
dependencies on the Boost Library and Aptitude. You can on Ubuntu 18+ obtain
these dependencies with the following commands.

```bash
apt install libboost-all-dev aptitude
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
  adiar::adiar_init();
  
  // do your stuff here...
  
  adiar::adiar_deinit();
}
```

If you create any [bdd](/bdd.md) object, then remember to have them garbage
collected (for example, let a variable go out of scope) before calling
`adiar::adiar_deinit()`.

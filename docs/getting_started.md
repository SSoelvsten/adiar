# Getting Started

## Dependencies
One needs a C++ compiler of ones choice. All development has been done with the
g++ compiler, so we recommend one to use the same.

```
sudo apt install g++
```

The project also has dependencies on the TPIE library, which itself has
dependencies on the Boost Library and Aptitude. On Ubuntu 18+ you can obtain
these dependencies with the following commands.

```
apt install libboost-all-dev aptitude
```

## Building with CMake
The project is built with _CMake_.

```
apt install cmake
```

To get started with using _Adiar_, you need to place the repository somewhere
within your project. This can neatly be done by adding it as a git submodule.

```
git submodule add https://github.com/SSoelvsten/adiar external/adiar
git submodule update --init --recursive
```

The in the projects _CMakeLists.txt_ one can include library as follows.

```cmake
add_subdirectory (external/adiar adiar)
```

A single executable target can then be linked _Adiar_ in a _CMakeLists.txt_ as
follows.

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


# Getting Started

The project has been created with the _CMake_ build system and is dependant on
another library underneath. Hence one first has to install dependencies, then
set up CMake and finally one can include _COOM_ in ones project.

## Dependencies

One needs a C++ compiler of ones choice; we have only tested _COOM_ with the
_g++_ compiler provided out-of-the-box by Ubuntu 18. The _TPIE_ library used
underneath also requires _Boost_ and _Aptitude_ installed, which can on Ubuntu
be done with the following command.

```
apt install libboost-all-dev aptitude
```

One may merely link to _COOM_ by placing the source files as a subdirectory,
such as adding the _COOM_ repository as a submodule.

```
git submodule add https://github.com/SSoelvsten/coom external/coom
git submodule update --init --recursive
```

## Building with CMake

At this point, one includes _COOM_ as a subdirectory within the project's
_CMakeLists.txt_ file.

```cmake
add_subdirectory (external/coom coom)
```

Then one links the library to any desired executable

```cmake
# -- N Queens --
add_executable(<target_name> <source_file>.cpp)
target_link_libraries(<target_name> coom)
set_target_properties(<target_name> PROPERTIES CXX_STANDARD 17)
```

where the last line is only necessary, if the `CXX_STANDARD` has not been set
project-wide to _17_ or higher.

## Usage
If all the above steps have been followed, then one can include all of _COOM_ in
a single statement. Before the library is used, it first has to be initialised
with _M_ megabytes of memory specified to be used by the library. Before the
program terminates, the library also has to be deinitialised.

```c++
#include <coom/coom.h>


main ()
{
  coom::coom_init(M);
  
  // your stuff goes then in here
  
  coom::coom_deinit();
}
```

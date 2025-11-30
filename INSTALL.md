# Building and Installation

## VCPKG

The `str-view` port is available in the vcpkg registry. For a review of how to use ports available in the vcpkg registry review the [vcpkg guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash). This library offers the following port name and usage file.

The `vcpkg.json` file.

```json
{
    "dependencies": [
        "str-view"
    ]
}
```

The `usage` file.

```txt
str_view provides CMake targets:

  find_package(str_view CONFIG REQUIRED)
  target_link_libraries(main PRIVATE str_view::str_view)
```

Then, the header in the `.c/.h` files.

```c
#include "str_view/str_view.h"
```

## Fetch Content

This approach will allow CMake to build `str_view` from source as part of your project. It does not have external dependencies, besides the standard library, so this may be viable for you. This is helpful if you want the ability to build the library in release or debug mode along with your project and possibly step through it with a debugger during a debug build. If you would rather link to the release build library file see the next section for the manual install.

To avoid including tests, samples, and other extraneous files when fetching content download a release.

```cmake
include(FetchContent)
FetchContent_Declare(
  ccc
  URL https://github.com/agl-alexglopez/str_view/releases/download/v[MAJOR.MINOR.PATCH]/str_view-v[MAJOR.MINOR.PATCH].zip
  #DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
)
FetchContent_MakeAvailable(str_view)
# Optionally ignore compiler warnings from the str_view library.
target_compile_options(str_view PRIVATE "-w")
```

Link against the library with its namespace.

```cmake
add_executable(main main.c)
target_link_libraries(main str_view::strrview)
```

Here is a concrete example with an arbitrary release that is likely out of date. Replace this version with the newest version on the releases page.

```cmake
include(FetchContent)
FetchContent_Declare(
  ccc
  URL https://github.com/agl-alexglopez/str_view/releases/download/v0.6.0/str_view-v0.6.0.zip
  #DOWNLOAD_EXTRACT_TIMESTAMP FALSE # CMake may raise a warning to set this. If so, uncomment and set.
)
FetchContent_MakeAvailable(str_view)
# Optionally ignore compiler warnings from the str_view library.
target_compile_options(str_view PRIVATE "-w")
add_executable(main main.c)
target_link_libraries(main str_view::str_view)
```

Now, `str_view` is part of your project build, allowing you to configure as you see fit. For a more traditional approach read the manual install section below.

## Manual Install Quick Start

1. Use the provided defaults 
2. Build the library
3. Install the library
4. Include the library.

To complete steps 1-3 with one command try the following if your system supports `make`.

```zsh
make str_view [OPTIONAL/INSTALL/PATH]
```

This will use CMake and your default compiler to build and install the library in release mode. By default, this library does not touch your system paths and it is installed in the `install/` directory of this folder. This is best for testing the library out while pointing `cmake` to the install location. Then, deleting the `install/` folder deletes any trace of this library from your system.

Then, in your `CMakeLists.txt`:

```cmake
find_package(str_view HINTS "~/path/to/str_view-v[VERSION]/install")
```

If you want to simply write the following command in your `CMakeLists.txt`,

```cmake
find_package(str_view)
```

specify that this library shall be installed to a location CMake recognizes by default. For example, my preferred location is as follows:

```zsh
make str_view ~/.local
```

Then the installation looks like this.

```txt
.local
├── include
│   └── str_view
│       └── str_view.h
└── lib
    ├── cmake
    │   └── str_view
    │       ├── str_viewConfig.cmake
    │       ├── str_viewConfigVersion.cmake
    │       ├── str_viewTargets.cmake
    │       └── str_viewTargets-release.cmake
    └── libstr_view_release.a
```

Now to delete the library if needed, simply find all folders and files with the `*str_view*` string somewhere within them and delete. You can also check the `build/install_manifest.txt` to confirm the locations of any files installed with this library.

## Include the Library

Once CMake can find the package, link against it and include the `str_view.h` header.

The `CMakeLists.txt` file.

```cmake
add_executable(my_exe my_exe.c)
target_link_libraries(my_exe str_view::str_view)
```

The C code.

```.c
#include "str_view/str_view.h"
```

## Alternative Builds

You may wish to use a different compiler and toolchain than what your system default specifies. Review the `CMakePrests.json` file for different compilers.

```zsh
make gcc-rel [OPTIONAL/INSTALL/PATH]
make install
```

Use Clang to compile the library.

```zsh
make clang-rel [OPTIONAL/INSTALL/PATH]
make install
```

## Without Make

If your system does not support Makefiles or the `make` command here are the cmake commands one can run that will allow another generator such as `Ninja` to complete building and installation.

```zsh
# Configure the project cmake files. 
# Replace this preset with your own if you'd like.
cmake --preset=default-rel -DCMAKE_INSTALL_PREFIX=[DESIRED/INSTALL/LOCATION]
cmake --build build
cmake --build build --target install
```

## User Presets

If you do not like the default presets, create a `CMakeUserPresets.json` in this folder and place your preferred configuration in that file. Here is my preferred configuration to get you started. I like to use a newer gcc version than the default presets specify.

```json
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 23,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "deb",
            "displayName": "GMake GCC Debug",
            "description": "Generated by GMake with GCC base debug preset.",
            "generator": "Unix Makefiles",
            "inherits": [
                "gcc-deb"
            ],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-12"
            }
        },
        {
            "name": "rel",
            "displayName": "GMake GCC Release",
            "description": "Generated by GMake with GCC base release preset.",
            "generator": "Unix Makefiles",
            "inherits": [
                "gcc-rel"
            ],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-12"
            }
        },
        {
            "name": "cdeb",
            "displayName": "Ninja clang Debug",
            "description": "Generated by Ninja with clang base debug preset.",
            "generator": "Ninja",
            "inherits": [
                "clang-deb"
            ],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang"
            }
        },
        {
            "name": "crel",
            "displayName": "Ninja clang Release",
            "description": "Generated by Ninja with clang base release preset.",
            "generator": "Ninja",
            "inherits": [
                "clang-rel"
            ],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "clang"
            }
        }
    ]
}
```

# str_view

![mini-grep](/images/mini-grep.png)

*Pictured Above: Mocking up a mini grep program is easy with the right tools. Use `mini_grep` and any other sample programs as they arrive in the `samples/` folder to test the utility and convenience of a `str_view`.*

The `str_view` type is a simple, copyable, flexible, read only view of `const char *` data in C. This implementation is experimental for now, lacking any official packaging or robust sample programs. However, this library is well tested and does what is advertised in the interface. The entire implementation can be viewed in `str_view/str_view.h/.c` and included in any project for some convenient string helpers.

A `str_view` is a 16-byte struct and, due to this size, is treated throughout the interface as a copyable type. This is neither a trivially cheap nor excessively expensive type to copy. The intention of this library is to abstract away many sharp edges of working with C-strings to provide usage that "just works," not optimize for performance at this time.

There are still improvements to be made to this library as time allows for packaging, sample programs, and further experimentation.

## Install Instructions

WIP

## Build Instructions

Here is my sample user presets I prefer for developing the library locally.

```cmake
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 21,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "gdeb",
            "displayName": "GMake GCC Debug",
            "description": "Generated by GMake with GCC base debug preset.",
            "generator": "Unix Makefiles",
            "inherits": [
                "gcc-deb"
            ],
            "cacheVariables": {
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_C_STANDARD": "11",
                "CMAKE_C_COMPILER": "gcc-12",
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_RUNTIME_OUTPUT_DIRECTORY": "${sourceDir}/build/deb",
                "CMAKE_C_FLAGS":
                    "-g3 -Wall -Wextra -Wfloat-equal -Wtype-limits -Wpointer-arith -Wshadow -Winit-self -fno-diagnostics-show-option -Wno-nonnull-compare -Wno-pointer-bool-conversion"
            }
        },
        {
            "name": "grel",
            "displayName": "GMake GCC Release",
            "description": "Generated by GMake with GCC base release preset.",
            "generator": "Unix Makefiles",
            "inherits": [
                "gcc-rel"
            ],
            "cacheVariables": {
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_C_STANDARD": "11",
                "CMAKE_C_COMPILER": "gcc-12",
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_RUNTIME_OUTPUT_DIRECTORY": "${sourceDir}/build/rel",
                "CMAKE_C_FLAGS":
                    "-Wall -Wextra -Wfloat-equal -Wtype-limits -Wpointer-arith -Wshadow -Winit-self -fno-diagnostics-show-option -Wno-nonnull-compare -Wno-pointer-bool-conversion"
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
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_C_STANDARD": "11",
                "CMAKE_C_COMPILER": "clang",
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_RUNTIME_OUTPUT_DIRECTORY": "${sourceDir}/build/deb",
                "CMAKE_C_FLAGS":
                    "-g3 -Wall -Wextra -Wfloat-equal -Wtype-limits -Wpointer-arith -Wshadow -Winit-self -fno-diagnostics-show-option -Wno-pointer-bool-conversion"
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
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_C_STANDARD": "11",
                "CMAKE_C_COMPILER": "clang",
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_RUNTIME_OUTPUT_DIRECTORY": "${sourceDir}/build/rel",
                "CMAKE_C_FLAGS":
                    "-Wall -Wextra -Wfloat-equal -Wtype-limits -Wpointer-arith -Wshadow -Winit-self -fno-diagnostics-show-option -Wno-pointer-bool-conversion"
            }
        }
    ]
}
```

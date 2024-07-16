# str_view

![mini-grep](/images/mini-grep.png)

*Pictured Above: Mocking up a mini grep program is easy with the right tools. Use `mini_grep` and any other sample programs as they arrive in the `samples/` folder to test the utility and convenience of a `str_view`.*

The `str_view` type is a simple, copyable, flexible, read only view of `const char *` data in C, modeled after `std::string_view` in C++. However, there are a few extra features including read-only tokenization and threeway comparison between plain `const char *` data and a `str_view`.

A `str_view` is a 16-byte struct and, due to this size, is treated throughout the interface as a copyable type. This is neither a trivially cheap nor excessively expensive type to copy. The intention of this library is to abstract away many sharp edges of working with C-strings to provide usage that "just works," not optimize for performance yet.

## Install Instructions

> **This library does not yet support the Windows platform. However, I plan on supporting Windows in the future.**

Download and extract the latest release `str_view-v[VERSION].zip` from the [Releases](https://github.com/agl-alexglopez/str_view/releases) page.

See [INSTALL.md](/INSTALL.md) file provided in this repo or in the downloaded release for full build and installation instructions.

## Interface

Read the [`str_view.h`](/str_view/str_view.h) interface for the full API and documentation.

## Status

This library is not yet version `1.0`. To reach `1.0` I would like to implement the following.

- More robust suite of tests to detect Undefined Behavor common with string handling.
- Better documentation highlighting when I cannot protect the user from Undefined Behavior through programmer error.
- SIMD intrinsics. At the very least, SIMD implemented for the short string brute force searches is critical.
- Windows support. MSVC does not implement some significant features that make this library more robust and safer for string handling and I need to learn more about the platform. See [this issue](https://github.com/agl-alexglopez/str_view/issues/9) for more.

That being said, I have already found this library very helpful whenever I need to write C code. Please consider giving it a try.

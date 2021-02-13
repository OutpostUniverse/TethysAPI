# TethysAPI

TethysAPI is a C++17 library providing access to Outpost 2's public API as well as its internal functions and data. It is purely header-only, and does not require linking against any libs to function.

Compared to Outpost2DLL, the public API exposed in TethysAPI has been changed to be more modern and tidier, and with better const-correctness in order to properly support MSVC in C++20 mode. TethysAPI essentially supercedes the Outpost2DLL, HFL/OP2Types, and OP2Internal libraries, although it should not be considered a drop-in replacement for them due to interface changes. TethysAPI can be used alongside these legacy libraries, which is helpful when transitioning an existing project to it.

# Usage

TethysAPI is intended to be added as a Git submodule to projects. It must reside in a subdirectory called `Tethys`.

The public mission API headers are located under the `API` directory. For your convenience, you may use `#include "Tethys/API/API.h"` to include all public API headers, but you are also free to include individual headers.

Headers containing internals, non-exported enums, or non-mission public APIs are located under the other directories:
* `Game` contains internal gameplay interfaces.
* `Network` contains internal networking interfaces.
* `Resource` contains resource management and graphics rendering interfaces.
* `UI` contains graphical user interface and related interfaces.

The public mission APIs are within the `TethysAPI` namespace, while everything else is within the `Tethys` namespace. You may wish to do `using namespace TethysAPI` and/or `using namespace Tethys`.

# Change log

## Version 0.8
* Initial release.

# Requirements

* Outpost 2 v1.2.7 or newer
* C++17

# mermaid-cpp

**Mermaid-cpp** is an implementation of [mermaid-js](https://github.com/mermaid-js/mermaid) in C++ (with possibility to compile back into WASM/Emscripten).

## Building

Use [CMake](https://cmake.org) for building and a decent C++23 compiler.

### Required dependencies

 - canvas\_ity (render canvas for diagrams);
 - fmtlib (format strings and prerequisite for ASCII render);
 - stb\_image (Image render).

The dependencies could be installed via [Conan](https://conan.io) or [vcpkg](https://vcpkg.io).

Conan:
```
conan install . -s build_type=<build_type> -of <build-dir>
```

vcpkg (assuming Windows 64 bit):
```
vcpkg install stb:x64-windows fmt:x64-windows canvas-ity:x64-windows
```

You could also use your distro's system libraries as well and set it via CMake's `-D<DepName_ROOT>=<path/to/depconfig>`,
but it is not tested or supported yet.


## Syntax

Can be viewed on [mermaid-js docs site](http://mermaid.js.org/intro/).


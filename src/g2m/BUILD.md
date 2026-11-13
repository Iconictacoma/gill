

## Build and compile

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
g2m main.gill -c main
g2m main.gill -c main -Ro
```
## Current implementation notes

The compiler currently emits C++20 and invokes the host C++ compiler. It is a prototype backend, not an LLVM backend.

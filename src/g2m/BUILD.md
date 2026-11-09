

## Build and compile

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
g2m main.gill -c main
g2m main.gill -c main -Ro
```
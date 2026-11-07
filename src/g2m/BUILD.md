## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Compile

```sh
g2m main.gill -c main
g2m main.gill -c main -Ro
```

Debug compilation is the default. `-Ro` enables host compiler release optimization.
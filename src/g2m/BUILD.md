## Build
```sh
cmake -S . -B build
cmake --build build
```

## Compile a Gill program
```sh
./build/g2m examples/hello.gill -c hello
./hello
```
Windows: use `build\\Debug\\g2m.exe` or `build\\g2m.exe`; the output is `hello.exe`.

`-Ro` enables optimized native compilation:
```sh
g2m main.gill -c main -Ro
```
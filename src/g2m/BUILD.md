**NOTE**: this is not the complete Gill compiler. There are some limitations that i'm too lazy to uncover.

Build:
```sh
cmake -S . -B build
cmake --build build
```

Running:
```sh
./build/g2m examples/hello.gill
```
On Windows, the executable is typically `build\\Debug\\g2m.exe` or `build\\g2m.exe`, depending on the generator.

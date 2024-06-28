# PN2HDA

## Dependencies

- [libxml2](https://gitlab.gnome.org/GNOME/libxml2)
- CMake (>= 3.21.2)
- C (std C99) -- GCC (or CLANG)

## Build

```sh
cmake -B build
cmake --build build --target pn2hda
```

## Usage

### Help

```sh
./build/pn2hda --help
```

### Examples

2 examples are provided in `./examples`.

```sh
./build/pn2hda --print_hda -o out.hda ./examples/simple-example.pnml
./build/pn2hda --logs NO ./examples/auto-concurrent-example.pnml
```

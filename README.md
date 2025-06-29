# Cyder

A "stupid-simple" command-line application for running audio plugins with hot reloading.

## Prerequisites
- CMake (>= 3.10)
- Git
- A C++ compiler (GCC, Clang, or MSVC)

## Cloning the repository
Clone the repository including all submodules:
```bash
git clone --recursive <repository-url>
```
If you've already cloned without submodules:
```bash
git submodule update --init --recursive
```

## Building
Run the following commands:
```bash
cmake . -B build
cmake --build build
```

## License
See [LICENSE.txt](LICENSE.txt) for license information.
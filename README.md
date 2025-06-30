# Cyder

An FX audio plugin wrapper that enables "hot-reloading" of the wrapped plugin.

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

## Installing the plugin

### macOS
```bash
sudo cp -R Cyder.vst3 /Library/Audio/Plug-Ins/VST3/
```

### Windows
Open a Command Prompt as administrator and run:
```cmd
xcopy /E /I Cyder.vst3 "C:\Program Files\Common Files\VST3\"
```

## License
See [LICENSE.txt](LICENSE.txt) for license information.
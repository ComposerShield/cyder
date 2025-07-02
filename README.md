# Cyder

An FX audio plugin wrapper that enables "hot-reloading" of the wrapped plugin.

## Prerequisites
- Mac or PC (PC still a work-in-progress)
- CMake (>= 3.10)
- Git
- A C++ compiler (Clang, Apple Clang, or MSVC supported)

## Cloning the repository
Clone the repository (no submodules in this project):
```bash
git clone git@github.com:ComposerShield/cyder.git
```

## Building
Run the following commands:
```bash
cmake . -B build
cmake --build build
```

## Installing the plugin

Copy the plugin into the system VST3 folder (or another path you configured in your DAW).
Here are some shortcut commands to make life easier:

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
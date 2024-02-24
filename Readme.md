# Turboballs

This is a camera controlled vollyball game

## Build for Multiple Platforms

You can build for multiple platforms using CMake, you will need the following installed to link for your platform of choice

- SDL2
- SDL2-mixer
- libvorbis
- please run [git submodule update --init --recursive](https://git-scm.com/book/en/v2/Git-Tools-Submodules) before building to pull other dependencies

### Windows

- You can install dependencies using [vcpkg](https://github.com/microsoft/vcpkg)
- It looks like vcpkg toolchain has updated: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vscode?pivots=shell-cmd 
- Open Project in Visual Studio (cmake support installed)
- Build
- Run Turboballs.exe
- if you run into any issues, ensure the assets directory is copied to the exe directory
  - with vscode cmake extension you can fix this by [setting build type on multi config](https://github.com/microsoft/vscode-cmake-tools/issues/1298)
  - note: game.dll hot reloading is supported with MSVC and Clang

### Linux / MacOS

- install dependencies using package manager i.e. apt, pacman, etc. for MacOS this would be [brew](https://brew.sh/)
- note: game.so hot reloading is supported with Clang

```zsh
mkdir build
cd build
cmake ..
make
./Turboballs
```

### Web

```zsh
mkdir build
cd build
emcmake cmake ..
emmake make
emrun Turboballs.html
```

## Format

I highly recommend setting your ide formatter to use clang format,

- install clang format (use package manager on mac or linux), this is also installed as part of llvm on windows visual studio installer
  - on windows you may need to have llvm tools on your path
    - example: `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin`
- run `py scripts/format.py` to format all cxx and hxx files in the project
- you can also use your ide to format on file save, this is highly recommended
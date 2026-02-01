# cpp-platformer
A hobby game project writen in C++ using the SDL2 libraries.

# Requirements
Project dependencies are downloaded and linked automatically via `vcpkg` and `CMake`.<br>
A C/C++ compiler with C++20 support (e.g. MSVC, GCC, or Clang)

## Windows
### vcpkg
1. Clone the `vcpkg` git repository.
```
git clone https://github.com/microsoft/vcpkg.git vcpkg
```
2. Run the `vcpkg` boostrap script.
```
cd vcpkg
.\bootstrap-vcpkg.bat
```
3. Set the `VCPKG_ROOT` environment variable and add it to `PATH`.
```
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"
```
> [!NOTE]
> Setting environment variables in this manner only affects the current terminal session. To make these changes permanent across all sessions, set them through the Windows System Environment Variables panel.

4. Open a new terminal and verify.
```
vcpkg --version
```

### CMake
Go to [cmake.org](https://cmake.org/) and download the `x86-x64` Windows installer.<br>
The `cmake` environment variable will be configured automatically.

## GNU/Linux
1. Install prerequisites.
```
sudo apt update
sudo apt install -y build-essential cmake ninja-build pkg-config curl zip unzip tar
```
2. Install `vcpkg` dependencies for building SDL from source.

On Linux SDL requires additional features for integration with the desktop stack - `sdl2[core,dbus,ibus,wayland,x11]`.<br>
In order for `vcpkg` to build them from source you need to install additional dependencies.
```
sudo apt install -y gperf meson python3-jinja2 libltdl-dev
```
3. Clone the `vcpkg` git repository.
```
git clone https://github.com/microsoft/vcpkg.git ~/.vcpkg
```
4. Run the `vcpkg` boostrap script.
```
cd ~/.vcpkg
./bootstrap-vcpkg.sh
```
5. Set the `VCPKG_ROOT` environment variable and add it to `PATH`.
```
echo 'export VCPKG_ROOT="$HOME/.vcpkg"' >> ~/.bashrc
echo 'export PATH="$PATH:$VCPKG_ROOT"' >> ~/.bashrc
```
6. Reload and verify.
```
source ~/.bashrc
vcpkg --version
```

## macOS
1. Install prerequisites.
```
xcode-select --install
brew install cmake ninja pkg-config
```
2. Clone the `vcpkg` git repository.
```
git clone https://github.com/microsoft/vcpkg.git ~/.vcpkg
```
3. Run the `vcpkg` boostrap script.
```
cd ~/.vcpkg
./bootstrap-vcpkg.sh
```
4. Set the `VCPKG_ROOT` environment variable and add it to `PATH`.
```
echo 'export VCPKG_ROOT="$HOME/.vcpkg"' >> ~/.zshrc
echo 'export PATH="$PATH:$VCPKG_ROOT"' >> ~/.zshrc
```
5. Reload and verify.
```
source ~/.zshrc
vcpkg --version
```

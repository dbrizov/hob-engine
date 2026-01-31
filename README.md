# cpp-platformer
A hobby game project writen in C++ using the SDL2 libraries.

# Requirements
Project dependencies are downloaded and linked automatically via `vcpkg` and `CMake`.<br>
Per platform build essentials (C/C++ compiler) - `MSVC`, `g++`, `clang`

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
sudo apt install -y build-essential cmake ninja-build curl zip unzip tar
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
echo 'export VCPKG_ROOT="$HOME/.vcpkg"' >> ~/.bashrc
echo 'export PATH="$PATH:$VCPKG_ROOT"' >> ~/.bashrc
```
5. Reload and verify.
```
source ~/.bashrc
vcpkg --version
```

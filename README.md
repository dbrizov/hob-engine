# cpp-platformer
A hobby game project writen in C++ using the SDL2 libraries.

# Requirements
Dependencies are downloaded and linked automatically via `vcpkg` and `CMake`.<br>
Currently only Windows is suppored, so you also need the `MSVC` compiler.<br>
I plan to make it multi-platform at some point.

## vcpkg
1. Clone the repository.

The first step is to clone the `vcpkg` repository from GitHub.
The repository contains scripts to acquire the `vcpkg` executable and a registry of curated open-source libraries maintained by the `vcpkg` community. To do this, run:
```
git clone https://github.com/microsoft/vcpkg.git
```

2. Run the bootstrap script.

Now that you have cloned the `vcpkg` repository, navigate to the `vcpkg` directory and execute the bootstrap script:
```
cd vcpkg; .\bootstrap-vcpkg.bat
```

3. Configure the `VCPKG_ROOT` environment variable.
```
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"
```
> [!NOTE]
> Setting environment variables in this manner only affects the current terminal session. To make these changes permanent across all sessions, set them through the Windows System Environment Variables panel.

## CMake
Go to [cmake.org](https://cmake.org/) and download the `x86-x64` Windows installer.<br>
The `cmake` environment variable will be configured automatically.

# arm64-osx overlay for directx-dxc.
#
# Microsoft does not publish prebuilt DirectX Shader Compiler binaries for
# macOS, so this overlay pulls the community arm64 build from
# MethanePowered/DirectXShaderCompilerBinary and installs it in the layout
# expected by directx-dxc-config.cmake.in (imported Microsoft::* targets plus
# the dxc command-line tool).
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)

if(NOT (VCPKG_TARGET_IS_OSX AND VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64"))
    message(FATAL_ERROR "${PORT} overlay only supports arm64-osx.")
endif()

# Pinned commit in MethanePowered/DirectXShaderCompilerBinary. The binaries
# correspond to upstream DXC v1.9.2602 (release date 2026-02-20).
set(DXC_REPO "MethanePowered/DirectXShaderCompilerBinary")
set(DXC_COMMIT "31d48398e0e76c0f18cdcedfe2d5c850d0e08419")
set(DXC_RAW "https://raw.githubusercontent.com/${DXC_REPO}/${DXC_COMMIT}")

# Prebuilt macOS binaries (libdxcompiler.dylib, libdxil.dylib, dxc tool).
vcpkg_download_distfile(ARCHIVE
    URLS "${DXC_RAW}/binaries/MacOS.zip"
    FILENAME "directx-dxc-macos-${DXC_COMMIT}.zip"
    SHA512 d8957a0fd14ad0ba20cd4d24367013228bbf07d39a5c3c269504ea60239c3d69a4b4699e5690b23015de347d9afec38f44af05cc4609e4b6974ec990b4c6bb7f
)

# Public headers (the zip ships binaries only).
vcpkg_download_distfile(HDR_DXCAPI
    URLS "${DXC_RAW}/include/dxc/dxcapi.h"
    FILENAME "directx-dxc-${DXC_COMMIT}-dxcapi.h"
    SHA512 0dbd3dee95f5a38f65e85f389965cf3e3027e4eab551b47d2d8020e11ed0053458eeda4a116aea6861c0e43e61c2bf7ef8ed2afb6b2e108411cba38140b0019d
)
vcpkg_download_distfile(HDR_DXCERRORS
    URLS "${DXC_RAW}/include/dxc/dxcerrors.h"
    FILENAME "directx-dxc-${DXC_COMMIT}-dxcerrors.h"
    SHA512 5849ce86f5d69ef3c3f67bb4918efae8ec9f5901ceb2878a39d285cce61fb367b26e2011ac4557e92e4adad4e8e856d35db8fa9109b42650cad50b347cc11135
)
vcpkg_download_distfile(HDR_DXCISENSE
    URLS "${DXC_RAW}/include/dxc/dxcisense.h"
    FILENAME "directx-dxc-${DXC_COMMIT}-dxcisense.h"
    SHA512 3aea0cfed1f0277894d2ea56103d2e8791028eefa0c77b4d56d25a450e2e0f1ee96cc603ab487bcad864bb399f13fe90fdd5b7f789208d1d55b404f0a3d3e06b
)
vcpkg_download_distfile(HDR_WINADAPTER
    URLS "${DXC_RAW}/include/dxc/WinAdapter.h"
    FILENAME "directx-dxc-${DXC_COMMIT}-WinAdapter.h"
    SHA512 1474bec2b2cee3676d4c266b8c2309dcd0682a6ed51919f2983e04c76d1b0f24d73d0fc793da5638ea899ea4616d827ade39d5c43279bcc86631164781735bd8
)

# License (reused from the upstream microsoft port; "license": null in vcpkg.json).
vcpkg_download_distfile(LICENSE_TXT
    URLS "https://raw.githubusercontent.com/microsoft/DirectXShaderCompiler/v1.9.2602/LICENSE.TXT"
    FILENAME "directx-dxc-LICENSE.v1.9.2602"
    SHA512 9feaa85ca6d42d5a2d6fe773706bbab8241e78390a9d61ea9061c8f0eeb5a3e380ff07c222e02fbf61af7f2b2f6dd31c5fc87247a94dae275dc0a20cdfcc8c9d
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    NO_REMOVE_ONE_LEVEL
)

# --- Headers --------------------------------------------------------------
file(INSTALL
    "${HDR_DXCAPI}"
    "${HDR_DXCERRORS}"
    "${HDR_DXCISENSE}"
    "${HDR_WINADAPTER}"
    DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}")
# vcpkg_download_distfile keeps the unique FILENAME; restore the real names.
file(RENAME "${CURRENT_PACKAGES_DIR}/include/${PORT}/directx-dxc-${DXC_COMMIT}-dxcapi.h"     "${CURRENT_PACKAGES_DIR}/include/${PORT}/dxcapi.h")
file(RENAME "${CURRENT_PACKAGES_DIR}/include/${PORT}/directx-dxc-${DXC_COMMIT}-dxcerrors.h"  "${CURRENT_PACKAGES_DIR}/include/${PORT}/dxcerrors.h")
file(RENAME "${CURRENT_PACKAGES_DIR}/include/${PORT}/directx-dxc-${DXC_COMMIT}-dxcisense.h"  "${CURRENT_PACKAGES_DIR}/include/${PORT}/dxcisense.h")
file(RENAME "${CURRENT_PACKAGES_DIR}/include/${PORT}/directx-dxc-${DXC_COMMIT}-WinAdapter.h" "${CURRENT_PACKAGES_DIR}/include/${PORT}/WinAdapter.h")

# --- Runtime libraries ----------------------------------------------------
file(INSTALL
    "${SOURCE_PATH}/MacOS/lib/libdxcompiler.dylib"
    "${SOURCE_PATH}/MacOS/lib/libdxil.dylib"
    DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
if(NOT DEFINED VCPKG_BUILD_TYPE)
    file(INSTALL
        "${SOURCE_PATH}/MacOS/lib/libdxcompiler.dylib"
        "${SOURCE_PATH}/MacOS/lib/libdxil.dylib"
        DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")
endif()

# --- dxc command-line tool ------------------------------------------------
# dxc-3.7 is the real binary (bin/dxc is a relative symlink to it) and its
# LC_RPATH is @executable_path/../lib, so install it under tools/<port>/bin
# with the dylibs in tools/<port>/lib next to it.
file(INSTALL
    "${SOURCE_PATH}/MacOS/bin/dxc-3.7"
    DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}/bin"
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
file(RENAME
    "${CURRENT_PACKAGES_DIR}/tools/${PORT}/bin/dxc-3.7"
    "${CURRENT_PACKAGES_DIR}/tools/${PORT}/bin/dxc")
file(INSTALL
    "${SOURCE_PATH}/MacOS/lib/libdxcompiler.dylib"
    "${SOURCE_PATH}/MacOS/lib/libdxil.dylib"
    DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}/lib")

# --- CMake config ---------------------------------------------------------
set(dll_name_dxc  "libdxcompiler.dylib")
set(dll_name_dxil "libdxil.dylib")
set(dll_dir       "lib")
if(NOT DEFINED VCPKG_BUILD_TYPE)
    set(dll_debug_dir "debug/lib")
else()
    set(dll_debug_dir "lib")
endif()
set(lib_name      "@rpath/libdxcompiler.dylib")
set(tool_path     "tools/${PORT}/bin/dxc")

configure_file("${CMAKE_CURRENT_LIST_DIR}/directx-dxc-config.cmake.in"
    "${CURRENT_PACKAGES_DIR}/share/${PORT}/${PORT}-config.cmake"
    @ONLY)

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${LICENSE_TXT}")

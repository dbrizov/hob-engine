#include "PathUtils.h"

namespace hob {
    std::filesystem::path PathUtils::get_root_path() {
#ifndef NDEBUG
        // (IN DEBUG MODE)
        // Return the root directory of the project
        std::filesystem::path source_file_path = __FILE__;
        std::filesystem::path project_root_path = source_file_path
            .parent_path() // root/src/engine/core
            .parent_path() // root/src/engine
            .parent_path() // root/src
            .parent_path(); // root

        return project_root_path;
#else
        // (IN RELEASE MODE)
        // Return the current directory of the executable
        std::filesystem::path current_path = std::filesystem::current_path();
        return current_path;
#endif
    }

    std::filesystem::path PathUtils::get_assets_root_path() {
        std::filesystem::path root_path = get_root_path();
        std::filesystem::path assets_root_path = root_path / "assets";
        return assets_root_path;
    }

    std::filesystem::path PathUtils::get_input_config_path() {
        std::filesystem::path root_path = get_root_path();
        std::filesystem::path input_config_path = root_path / "config" / "input_config.json";
        return input_config_path;
    }
}

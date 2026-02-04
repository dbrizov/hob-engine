#ifndef CPP_PLATFORMER_IOUTILS_H
#define CPP_PLATFORMER_IOUTILS_H
#include <filesystem>


class PathUtils {
public:
    static std::filesystem::path get_root_path();
    static std::filesystem::path get_assets_root_path();
    static std::filesystem::path get_input_config_path();
};


#endif //CPP_PLATFORMER_IOUTILS_H

#ifndef HOB_ENGINE_IOUTILS_H
#define HOB_ENGINE_IOUTILS_H
#include <filesystem>


namespace hob {
    class PathUtils {
    public:
        static std::filesystem::path get_root_path();
        static std::filesystem::path get_assets_root_path();
        static std::filesystem::path get_input_config_path();
    };
}


#endif //HOB_ENGINE_IOUTILS_H

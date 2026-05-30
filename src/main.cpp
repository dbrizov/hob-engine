#include "engine/core/app.h"
#include "engine/core/path_utils.h"

int main(int argc, char* argv[]) {
    hob::EngineConfig config(hob::PathUtils::get_engine_config_path());
    hob::App app(config);

    if (!app.is_initialized()) {
        return 1;
    }

    app.run();

    return 0;
}

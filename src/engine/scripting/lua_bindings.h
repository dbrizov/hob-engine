#pragma once

namespace sol {
    class state;
}

namespace hob {
    class App;

    void register_bindings(sol::state& lua, App& app);
}

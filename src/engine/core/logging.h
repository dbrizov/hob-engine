#ifndef HOB_ENGINE_LOGGING_H
#define HOB_ENGINE_LOGGING_H
#include <format>
#include <iostream>

namespace hob::debug {
    template<typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
    }

    template<typename... Args>
    void log_error(std::format_string<Args...> fmt, Args&&... args) {
        std::cerr << std::format(fmt, std::forward<Args>(args)...) << std::endl;
    }
}

#endif //HOB_ENGINE_LOGGING_H

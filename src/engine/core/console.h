#ifndef HOB_ENGINE_CONSOLE_H
#define HOB_ENGINE_CONSOLE_H
#include <format>
#include <imgui.h>
#include <string>
#include <string_view>
#include <vector>

namespace hob {
    class App;

    class Console {
        static constexpr ImColor LOG_ENTRY_COLOR_WHITE = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
        static constexpr ImColor LOG_ENTRY_COLOR_RED = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
        static constexpr ImColor LOG_ENTRY_COLOR_ORANGE = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
        static constexpr size_t INPUT_BUFFER_SIZE = 256;

        const App& m_app;
        bool m_open = false;
        char m_input_buffer[INPUT_BUFFER_SIZE] = {};
        std::vector<std::string> m_log;
        bool m_scroll_to_bottom = true;
        std::vector<std::string> m_history;
        int m_history_index = -1; // -1: new line, [0..history-1] browsing history.
        std::vector<std::string> m_commands;

    public:
        explicit Console(const App& app);

        bool is_open() const;
        void toggle_open();

        void clear_input_buffer();
        void clear_log();

        template<typename... Args>
        void log(std::format_string<Args...> fmt, Args&&... args) {
            m_log.emplace_back(std::format(fmt, std::forward<Args>(args)...));
            m_scroll_to_bottom = true;
        }

        template<typename... Args>
        void log_error(std::format_string<Args...> fmt, Args&&... args) {
            log("[error] {}", std::format(fmt, std::forward<Args>(args)...));
        }

        void render();

    private:
        void execute_command(std::string_view command_line_sv);

        static int text_edit_callback_stub(ImGuiInputTextCallbackData* data);
        int text_edit_callback(ImGuiInputTextCallbackData* data);

        static void trim_right_spaces(char* s);
        static unsigned char to_upper(unsigned char c);
        static unsigned char to_lower(unsigned char c);
        static bool equals_ci(std::string_view a, std::string_view b);
        static bool starts_with_ci(std::string_view s, std::string_view prefix);
    };
}

#endif //HOB_ENGINE_CONSOLE_H

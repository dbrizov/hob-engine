#ifndef HOB_ENGINE_APP_CONSOLE_H
#define HOB_ENGINE_APP_CONSOLE_H
#include <format>
#include <imgui.h>
#include <string>
#include <vector>

namespace hob {
    class App;

    class Console {
        const App& m_app;

        bool m_open = false;

        char m_input_buffer[256] = {};

        std::vector<std::string> m_log;
        bool m_scroll_to_bottom = false;

        std::vector<std::string> m_history;
        int m_history_index = -1; // -1: new line, [0..history-1] browsing history.

        std::vector<std::string> m_commands;

    public:
        Console(const App& app);

        bool is_open() const;
        void toggle_open();

        void clear_log();

        template<typename... Args>
        void add_log(std::format_string<Args...> fmt, Args&&... args) {
            m_log.emplace_back(std::format(fmt, std::forward<Args>(args)...));
            m_scroll_to_bottom = true;
        }

        void clear_input_buffer();

        bool render();

    private:
        void execute_command(const char* command_line_cstr);

        static int text_edit_callback_stub(ImGuiInputTextCallbackData* data);
        int text_edit_callback(ImGuiInputTextCallbackData* data);

        static void trim_right_spaces(char* s);
        static unsigned char to_upper(unsigned char c);
        static unsigned char to_lower(unsigned char c);
        static bool equals_ci(std::string_view a, std::string_view b);
        static bool starts_with_ci(std::string_view s, std::string_view prefix);
    };
}

#endif // HOB_ENGINE_APP_CONSOLE_H

#ifndef HOB_ENGINE_APP_CONSOLE_H
#define HOB_ENGINE_APP_CONSOLE_H
#include <imgui.h>
#include <string>
#include <vector>

namespace hob {
    class AppConsole {
        static constexpr size_t INPUT_BUFF_SIZE = 256;

        char m_input_buff[INPUT_BUFF_SIZE] = {};
        std::vector<std::string> m_items;

        bool m_scroll_to_bottom = false;

        std::vector<std::string> m_history;
        int m_history_pos = -1; // -1: new line, [0..history-1] browsing history.

        std::vector<std::string> m_commands;

    public:
        AppConsole();

        void add_log(const char* fmt, ...) IM_FMTARGS(2);
        void clear_log();

        void draw(const char* title, bool* p_open);

    private:
        void exec_command(const char* command_line_cstr);

        static int text_edit_callback_stub(ImGuiInputTextCallbackData* data);
        int text_edit_callback(ImGuiInputTextCallbackData* data);

        static void trim_right_spaces(char* s);
        static unsigned char lower_uc(unsigned char c);
        static bool iequals(std::string_view a, std::string_view b);
        static bool istarts_with(std::string_view s, std::string_view prefix);
    };
}

#endif // HOB_ENGINE_APP_CONSOLE_H

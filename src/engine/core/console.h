#ifndef HOB_ENGINE_CONSOLE_H
#define HOB_ENGINE_CONSOLE_H
#include <cstdint>
#include <format>
#include <functional>
#include <imgui.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hob {
    class App;

    class ConsoleBackend {
    public:
        struct Command;
        struct CVar;

    private:
        std::unordered_map<std::string, Command> m_commands;
        std::unordered_map<std::string, CVar> m_cvars;

    public:
        using Args = std::span<const std::string>;
        using CmdFunc = std::function<void(Args)>;

        enum class CVarType {
            Bool,
            Int,
            Float,
            String
        };

        enum CVarFlags : uint32_t {
            None = 0,
            Archive = 1 << 0,
            ReadOnly = 1 << 1,
            Cheat = 1 << 2,
        };

        struct Command {
            std::string name;
            std::string help;
            CmdFunc func;

            std::string to_string(uint32_t indent = 0) const {
                return std::format("{:<{}} ({})", name, indent, help);
            }
        };

        struct CVar {
            std::string name;
            std::string help;
            CVarType type = CVarType::String;
            uint32_t flags = None;

            std::string value;
            std::string default_value;

            std::function<void(const CVar&)> on_changed;

            std::string to_string(uint32_t indent = 0) const {
                return std::format("{:<{}} = '{}' (default '{}')", name, indent, value, default_value);
            }
        };

        // Logging sink (frontend sets this)
        std::function<void(std::string_view)> print;
        std::function<void(std::string_view)> print_error;

        ConsoleBackend();

        bool register_command(std::string name, std::string help, CmdFunc func);

        bool register_cvar(std::string name,
                           std::string help,
                           std::string default_value,
                           CVarType type,
                           uint32_t flags = None,
                           std::function<void(const CVar&)> on_changed = {});

        void execute_line(std::string_view line);

        // Completion for a prefix (word currently being completed)
        std::vector<std::string_view> complete(std::string_view prefix);

        const Command* find_command(std::string_view name) const;
        const CVar* find_cvar(std::string_view name) const;

    private:
        static std::string key_of(std::string_view s);
        static std::vector<std::string> tokenize(std::string_view line);

        void execute_cvar(CVar& cvar, Args args);

        // Built-in commands
        void cmd_help() const;
        void cmd_cmdlist(uint32_t indent = 0) const;
        void cmd_cvarlist(uint32_t indent = 0) const;
    };

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

        ConsoleBackend m_backend;

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
    };
}

#endif //HOB_ENGINE_CONSOLE_H

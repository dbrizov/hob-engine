#ifndef HOB_ENGINE_CONSOLE_H
#define HOB_ENGINE_CONSOLE_H
#include <algorithm>
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
        using Args = std::span<const std::string>;
        using CmdFunc = std::function<void(Args)>;

        enum class CVarType {
            Bool,
            Int,
            Float,
            String
        };

        struct Command {
            std::string name;
            std::string help;
            CmdFunc func;
        };

        struct CVar {
            std::string name;
            std::string help;
            CVarType type = CVarType::String;

            std::string value; // quake-style: store as string
            std::string default_value;

            std::function<void(const CVar&)> on_changed;
        };

        // Logging sink (frontend sets this)
        std::function<void(std::string_view)> print;
        std::function<void(std::string_view)> print_error;

        ConsoleBackend() {
            // Built-in commands (Quake-like baseline)
            register_command("help", "List commands and cvars", [this](Args args) { cmd_help(args); });
            register_command("cmdlist", "List all commands", [this](Args) { cmd_cmdlist(); });
            register_command("cvarlist", "List all cvars", [this](Args) { cmd_cvarlist(); });
        }

        bool register_command(std::string name, std::string help, CmdFunc fn) {
            // store original casing, but key comparisons are case-insensitive
            auto key = key_of(name);
            return m_commands.emplace(std::move(key), Command{std::move(name), std::move(help), std::move(fn)}).second;
        }

        bool register_cvar(std::string name, std::string help, std::string default_value, CVarType type,
                           std::function<void(const CVar&)> on_changed = {}) {
            auto key = key_of(name);
            CVar cvar;
            cvar.name = std::move(name);
            cvar.help = std::move(help);
            cvar.type = type;
            cvar.value = default_value;
            cvar.default_value = std::move(default_value);
            cvar.on_changed = std::move(on_changed);
            return m_cvars.emplace(std::move(key), std::move(cvar)).second;
        }

        // Execute a full line, already trimmed by frontend
        void exec_line(std::string_view line) {
            auto tokens = tokenize(line);
            if (tokens.empty())
                return;

            auto key = key_of(tokens[0]);

            // command?
            if (auto it = m_commands.find(key); it != m_commands.end()) {
                it->second.func(tokens);
                return;
            }

            // cvar? (Quake: typing a cvar prints it, typing "cvar value" sets it)
            if (auto it = m_cvars.find(key); it != m_cvars.end()) {
                exec_cvar(it->second, tokens);
                return;
            }

            if (print_error)
                print_error("Unknown command/cvar: '" + std::string(tokens[0]) + "'");
        }

        // Completion for a prefix (word currently being completed)
        std::vector<std::string_view> complete(std::string_view prefix) const {
            std::vector<std::string_view> out;
            out.reserve(m_commands.size() + m_cvars.size());

            for (auto& [_, cmd] : m_commands)
                if (starts_with_ci(cmd.name, prefix))
                    out.push_back(cmd.name);

            for (auto& [_, cv] : m_cvars)
                if (starts_with_ci(cv.name, prefix))
                    out.push_back(cv.name);

            std::sort(out.begin(), out.end(), [](std::string_view a, std::string_view b) {
                // case-insensitive sort would be nicer, but this is fine for now
                return a < b;
            });
            return out;
        }

        // Optional: access cvar for engine systems
        const CVar* find_cvar(std::string_view name) const {
            auto it = m_cvars.find(key_of(name));
            return (it != m_cvars.end()) ? &it->second : nullptr;
        }

        CVar* find_cvar(std::string_view name) {
            auto it = m_cvars.find(key_of(name));
            return (it != m_cvars.end()) ? &it->second : nullptr;
        }

    private:
        // Case-insensitive key for unordered_map: we store lowercase keys.
        static std::string key_of(std::string_view s) {
            std::string k(s);
            std::transform(k.begin(), k.end(), k.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            return k;
        }

        // Your helpers (kept here so backend doesn't depend on frontend)
        static unsigned char to_lower_uc(unsigned char c) { return (unsigned char)std::tolower(c); }

        static bool equals_ci(std::string_view a, std::string_view b) {
            if (a.size() != b.size())
                return false;
            for (size_t i = 0; i < a.size(); ++i)
                if (to_lower_uc((unsigned char)a[i]) != to_lower_uc((unsigned char)b[i]))
                    return false;
            return true;
        }

        static bool starts_with_ci(std::string_view s, std::string_view prefix) {
            if (prefix.size() > s.size())
                return false;
            for (size_t i = 0; i < prefix.size(); ++i)
                if (to_lower_uc((unsigned char)s[i]) != to_lower_uc((unsigned char)prefix[i]))
                    return false;
            return true;
        }

        // Tokenizer with quotes, returns owning strings for args span
        static std::vector<std::string> tokenize(std::string_view line) {
            std::vector<std::string> out;
            std::string cur;
            bool in_quotes = false;

            auto push = [&] {
                if (!cur.empty()) {
                    out.push_back(cur);
                    cur.clear();
                }
            };

            for (size_t i = 0; i < line.size(); ++i) {
                char c = line[i];

                if (c == '"') {
                    in_quotes = !in_quotes;
                    continue;
                }

                if (!in_quotes && (c == ' ' || c == '\t')) {
                    push();
                    continue;
                }

                cur.push_back(c);
            }
            push();
            return out;
        }

        void exec_cvar(CVar& cv, Args args) {
            if (args.size() == 1) {
                if (print) {
                    print(cv.name + " = \"" + cv.value + "\" (default \"" + cv.default_value + "\")");
                }
                return;
            }

            // Join args[1..] with spaces (so quotes are optional if you want)
            std::string new_value = args[1];
            for (size_t i = 2; i < args.size(); ++i) {
                new_value.push_back(' ');
                new_value += args[i];
            }

            cv.value = std::move(new_value);
            if (cv.on_changed)
                cv.on_changed(cv);

            if (print)
                print(cv.name + " set to \"" + cv.value + "\"");
        }

        // Built-ins
        void cmd_help(Args) {
            if (print)
                print("Commands: (use cmdlist), CVars: (use cvarlist)");
        }

        void cmd_cmdlist() {
            if (!print)
                return;
            print("Commands:");
            // show sorted by display name
            std::vector<std::string_view> names;
            names.reserve(m_commands.size());
            for (auto& [_, cmd] : m_commands)
                names.push_back(cmd.name);
            std::sort(names.begin(), names.end());
            for (auto n : names)
                print(std::string("- ") + std::string(n));
        }

        void cmd_cvarlist() {
            if (!print)
                return;
            print("CVars:");
            std::vector<std::string_view> names;
            names.reserve(m_cvars.size());
            for (auto& [_, cv] : m_cvars)
                names.push_back(cv.name);
            std::sort(names.begin(), names.end());
            for (auto n : names)
                print(std::string("- ") + std::string(n));
        }

        void cmd_set(Args args) {
            if (args.size() < 3) {
                if (print_error)
                    print_error("Usage: set <cvar> <value...>");
                return;
            }
            auto* cv = find_cvar(args[1]);
            if (!cv) {
                if (print_error)
                    print_error("Unknown cvar: '" + args[1] + "'");
                return;
            }

            // create a fake args list: ["cvar", "value..."]
            std::vector<std::string> set_args;
            set_args.reserve(args.size() - 1);
            set_args.push_back(args[1]);
            for (size_t i = 2; i < args.size(); ++i)
                set_args.push_back(args[i]);
            exec_cvar(*cv, set_args);
        }

        void cmd_toggle(Args args) {
            if (args.size() != 2) {
                if (print_error)
                    print_error("Usage: toggle <cvar>");
                return;
            }
            auto* cv = find_cvar(args[1]);
            if (!cv) {
                if (print_error)
                    print_error("Unknown cvar: '" + args[1] + "'");
                return;
            }
            if (cv->type != CVarType::Bool) {
                if (print_error)
                    print_error("toggle works only on bool cvars.");
                return;
            }

            if (equals_ci(cv->value, "0") || equals_ci(cv->value, "false") || cv->value.empty())
                cv->value = "1";
            else
                cv->value = "0";

            if (cv->on_changed)
                cv->on_changed(*cv);
            if (print)
                print(cv->name + " set to \"" + cv->value + "\"");
        }

        void cmd_reset(Args args) {
            if (args.size() != 2) {
                if (print_error)
                    print_error("Usage: reset <cvar>");
                return;
            }
            auto* cv = find_cvar(args[1]);
            if (!cv) {
                if (print_error)
                    print_error("Unknown cvar: '" + args[1] + "'");
                return;
            }
            cv->value = cv->default_value;
            if (cv->on_changed)
                cv->on_changed(*cv);
            if (print)
                print(cv->name + " reset to default \"" + cv->value + "\"");
        }

    private:
        std::unordered_map<std::string, Command> m_commands; // lowercase key -> command
        std::unordered_map<std::string, CVar> m_cvars; // lowercase key -> cvar
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

        static void trim_right_spaces(char* s);
        static unsigned char to_upper(unsigned char c);
        static unsigned char to_lower(unsigned char c);
        static bool equals_ci(std::string_view a, std::string_view b);
        static bool starts_with_ci(std::string_view s, std::string_view prefix);
    };
}

#endif //HOB_ENGINE_CONSOLE_H
